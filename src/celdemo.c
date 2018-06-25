#include "celdemo.h"
#include "version.h"
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

CelVersion celdemo_ver;
CelEventLoopGroup evt_loop_grp;

static BOOL is_debug = FALSE;
static BOOL is_exit = FALSE;
#ifdef _CEL_UNIX
static CelSignals s_signals[] = 
{
    { SIGPIPE, "sigpipe", SIG_IGN }, 
    { 0, NULL, NULL }
};
static CelResourceLimits s_rlimits = { 0, MAX_FD, MAX_FD };
#endif
TCHAR log_path[CEL_PATHLEN];

int celdemo_main(int argc, TCHAR *argv[])
{
    if (cel_service_is_running(CELDEMO_SERVICE))
    {
        Warning((CELDEMO_SERVICE _T(" is running...")));
        exit(1);
    }
    cel_ssllibrary_init();
    cel_multithread_support();
    //cel_allocator_hook_register(NULL, NULL, NULL);
    cel_cryptomutex_register(NULL, NULL);
    
#ifdef _CEL_UNIX
    cel_signals_init(s_signals);
    cel_resourcelimits_set(&s_rlimits);
#endif
#ifdef _CEL_WIN
    cel_wsastartup();
#endif
    if (conf_read(&conf, cel_fullpath(_T("../etc/cel-demo.conf"))) != 0
        || conf_write(&conf, cel_fullpath(_T("../etc/cel-demo.conf"))) != 0)
    {
        Err((_T("%s"), cel_geterrstr(cel_geterrno())));
        exit(1);
    }
    cel_logger_level_set(conf.log.level);
    //cel_logger_hostname_set();
    if (!is_debug)
    {
        cel_fullpath_r(conf.log.path, log_path, CEL_PATHLEN);
        cel_logger_hook_register("logger-file", 
            cel_logmsg_fwrite, cel_logmsg_fflush, log_path);
        Info((CELDEMO_SERVICE _T(" version %s"), 
            cel_version_release(&celdemo_ver)));
    }
    if (cel_eventloopgroup_init(&evt_loop_grp, -1, TRUE) == -1)
        return 1;
    Info((_T("Event loop init success, threads:%d"), 
        cel_eventloopgroup_get_threads_num(&evt_loop_grp)));
    if (conf.api_server.ssl.on == 1)
    {
        if (((wmip_sslctx = cel_sslcontext_new(
            cel_sslcontext_method(conf.api_server.ssl.protocols))) == NULL
            || cel_sslcontext_set_own_cert(wmip_sslctx, 
            cel_fullpath(conf.api_server.ssl.cert), 
            cel_fullpath(conf.api_server.ssl.key), 
            conf.api_server.ssl.key_pswd) == -1
            || cel_sslcontext_set_ciphersuites(wmip_sslctx, 
            conf.api_server.ssl.ciphers) == -1))
        {
            Err(("Vp ssl context init failed.(%s)", 
                cel_geterrstr(cel_geterrno())));
            exit(1);
        }
        Info(("Vp ssl context init successed, protocol \"%s, %s\".", 
            conf.api_server.ssl.protocols, conf.api_server.ssl.ciphers));
    }
    if (apiserver_listen(&wmip_listener, 
        conf.api_server.address, wmip_sslctx) == -1)
        exit(1);
    cel_eventloopgroup_schedule_timer(
        &evt_loop_grp, 1000, 1, (CelTimerCallbackFunc)cel_log_flush, NULL);
    cel_eventloopgroup_exit(&evt_loop_grp);
#ifdef _CEL_WIN
    cel_wsacleanup();
#endif
    //cel_allocator_hook_unregister();

    return 0;
}

void celdemo_stop(void)
{
    Info(("Receive sinigal[exit]."));
    is_exit = TRUE;
    cel_eventloopgroup_exit(&evt_loop_grp);
}

static int start_service(int argc, TCHAR *argv[])
{
    CelServiceEntry *sc_entry;

    _tprintf(CELDEMO_SERVICE _T(" version %s")CEL_CRLF,
        cel_version_release(&celdemo_ver));
    if (argc == 1
        || (_tcscmp(argv[1], _T("-s")) == 0  
        || _tcscmp(argv[1], _T("--signal")) == 0))
    {
        if (argc == 3)
        {
            if (_tcscmp(argv[2], _T("stop")) == 0)
                return (cel_service_stop(CELDEMO_SERVICE) ? 0 : 1);
            else if (_tcscmp(argv[2], _T("reload")) == 0)
                return (cel_service_reload(CELDEMO_SERVICE) ? 0 : 1);
        }
        sc_entry = cel_service_entry_create(
            CELDEMO_SERVICE, celdemo_main, celdemo_stop);
        return cel_service_entry_dispatch(sc_entry);
    }
    is_debug = TRUE;
    return celdemo_main(argc, argv);
}

static int puts_version(int argc, TCHAR *argv[])
{
    _tprintf(CELDEMO_SERVICE _T(" version %s")CEL_CRLF,
        cel_version_release(&celdemo_ver));
    _tprintf(_T("cel version %s")CEL_CRLF, cel_realse());
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

    cel_version_init(&celdemo_ver, NULL);
    return cel_commandline_dispatch(cmd_table, argc, argv);
}
