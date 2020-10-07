#include "celdemo.h"
#include "version.h"
#include "error.h"
#include "log.h"
#include "conf.h"
#include "apiserver.h"
#include "cel/multithread.h"
#include "cel/thread.h"
#include "cel/file.h"
#include "cel/commandline.h"
#include "cel/sys/envinfo.h"
#include "cel/sys/service.h"
#include "cel/crypto/crypto.h"
#include "cel/net/ssl.h"

CelVersion ver;
CelEventLoopGroup evt_loop_grp;
BOOL is_foreground = FALSE;
BOOL is_debug = FALSE;

static BOOL is_exit = FALSE;
#ifdef _CEL_UNIX
static CelResourceLimits s_rlimits = { 0, MAX_FD, MAX_FD };
#endif
TCHAR log_path[CEL_PATHLEN];

int service_main(int argc, TCHAR *argv[])
{
    if (cel_service_is_running(SERVICE_NAME))
    {
        Warning((SERVICE_NAME _T(" is running...")));
        exit(1);
    }
    
    cel_multithread_support();
    cel_allocator_hook_register(NULL, NULL, NULL);
    cel_cryptomutex_register(NULL, NULL);
    cel_ssllibrary_init();
    cel_wsastartup();
#ifdef _CEL_UNIX
    cel_resourcelimits_set(&s_rlimits);
#endif
    if (conf_read(&conf, cel_fullpath(SERVICE_CONF)) != 0
        || conf_write(&conf, cel_fullpath(SERVICE_CONF)) != 0)
    {
        Err((_T("%s"), cel_geterrstr()));
        exit(1);
    }
    if (is_debug)
    {
        cel_logger_level_set(
            &g_logger, CEL_LOGFACILITY_LOCAL0, CEL_LOGLEVEL_ERR);
        cel_log_level_set(CEL_LOGLEVEL_DEBUG);
        //cel_log_hostname_set(conf.server.host);
        cel_log_hook_register("console", cel_logmsg_puts, NULL, NULL);
    }
    else
    {
        cel_fullpath_r(conf.log.path, log_path, CEL_PATHLEN);
        cel_log_hook_register("logger-file", 
            cel_logmsg_fwrite, cel_logmsg_fflush, log_path);
        Info((SERVICE_NAME _T(" version %s"), 
            cel_version_release(&ver)));
    }
    if (cel_eventloopgroup_init(&evt_loop_grp, MAX_FD, -1, TRUE) == -1)
        return 1;
    Info((_T("Event loop init success, threads:%d"), 
        cel_eventloopgroup_get_threads_num(&evt_loop_grp)));
    if (conf.api_server.ssl.on == 1)
    {
        if (((api_sslctx = cel_sslcontext_new(
            cel_sslcontext_method(conf.api_server.ssl.protocols))) == NULL
            || cel_sslcontext_set_own_cert(api_sslctx, 
            cel_fullpath(conf.api_server.ssl.cert), 
            cel_fullpath(conf.api_server.ssl.key), 
            conf.api_server.ssl.key_pswd) == -1
            || cel_sslcontext_set_ciphersuites(api_sslctx, 
            conf.api_server.ssl.ciphers) == -1))
        {
            Err(("Vp ssl context init failed.(%s)", 
                cel_geterrstr()));
            exit(1);
        }
        Info(("Vp ssl context init successed, protocol \"%s, %s\".", 
            conf.api_server.ssl.protocols, conf.api_server.ssl.ciphers));
    }
    if (apiserver_listen(&api_listener, 
        conf.api_server.address, api_sslctx) == -1)
        exit(1);
    cel_eventloopgroup_schedule_timer(
        &evt_loop_grp, 1000, 1, (CelTimerCallbackFunc)cel_log_flush, NULL);
    
    cel_eventloopgroup_destroy(&evt_loop_grp);
#ifdef _CEL_WIN
    cel_wsacleanup();
#endif
    //cel_allocator_hook_unregister();

    return 0;
}

void service_stop(void)
{
    Info(("Receive sinigal[exit]."));
    is_exit = TRUE;
    cel_eventloopgroup_exit(&evt_loop_grp);
}

static int start_service(int argc, TCHAR *argv[])
{
    CelServiceEntry *sc_entry;

    _tprintf(SERVICE_NAME _T(" version %s")CEL_CRLF,
        cel_version_release(&ver));
    if (argc == 1
        || (_tcscmp(argv[1], _T("-s")) == 0  
        || _tcscmp(argv[1], _T("--signal")) == 0))
    {
        if (argc == 3)
        {
            if (_tcscmp(argv[2], _T("stop")) == 0)
                return (cel_service_stop(SERVICE_NAME) ? 0 : 1);
            else if (_tcscmp(argv[2], _T("reload")) == 0)
                return (cel_service_reload(SERVICE_NAME) ? 0 : 1);
        }
        sc_entry = cel_service_entry_create(
            SERVICE_NAME, service_main, service_stop);
        return cel_service_entry_dispatch(
            sc_entry, CEL_SERVICE_START_DAEMON);
    }
    if ((_tcscmp(argv[1], _T("-f")) == 0  
        || _tcscmp(argv[1], _T("--foreground")) == 0))
    {
        is_foreground = TRUE;
        sc_entry = cel_service_entry_create(
            SERVICE_NAME, service_main, service_stop);
        return cel_service_entry_dispatch(
            sc_entry, CEL_SERVICE_START_FOREGROUND);
    }
    else
    {
        is_debug = TRUE;
        sc_entry = cel_service_entry_create(
            SERVICE_NAME, service_main, service_stop);
        return cel_service_entry_dispatch(
            sc_entry, CEL_SERVICE_START_DEBUG);
    }
}

static int puts_version(int argc, TCHAR *argv[])
{
    _tprintf(SERVICE_NAME _T(" version %s")CEL_CRLF,
        cel_version_release(&ver));
    _tprintf(_T("cel version %s")CEL_CRLF, cel_lib_realse());
    return 0;
}

int _tmain(int argc, TCHAR *argv[])
{
    CelCommandTableEntry cmd_table[] = 
    {
        { _T("-d"), _T("--debug"), _T("Print debug information on the screen"),
        start_service, { NULL } },

        { _T("-f"), _T("--foreground"), _T("Running it in the foregroud"),
        start_service, { NULL } },

        { _T("-h"), _T("--help"), _T("Display this help and exit"), NULL, { NULL } },

        { _T("-s"), _T("--signal"), 
        _T("Send signal to a master process: stop, reload"),
        start_service, { NULL } },

        { _T("-v"), _T("--version"),_T("Output version information and exit"),
        puts_version,  { NULL } },
        {NULL, NULL, NULL, NULL, { NULL }}
    };

    cel_version_init(&ver, NULL);
    return cel_commandline_dispatch(cmd_table, argc, argv);
}
