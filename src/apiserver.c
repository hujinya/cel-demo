#include "apiserver.h"
#include "connection.h"
#include "error.h"
#include "cel/convert.h"
#include "cel/file.h"
#include "cel/sys/perf.h"

CelSslContext *wmip_sslctx = NULL;
CelWmipListener wmip_listener;

static int apiclient_conf_getstatus(CelWmipClient *wmipclt, CelWmipMessage *message);
static int apiclient_error_list(CelWmipClient *wmipclt, CelWmipMessage *message);
static int apiclient_html(CelWmipClient *wmipclt, CelWmipMessage *message);
static int apiclient_post_test(CelWmipClient *wmipclt, CelWmipMessage *message);
static int apiclient_service_get_version(CelWmipClient *wmipclt, CelWmipMessage *message);
static int apiclient_sso_login(CelWmipClient *wmipclt, CelWmipMessage *message);

static CelKeywordA s_apiroutes[] = 
{
    { sizeof("conf/get") - 1, "conf/get", apiclient_conf_getstatus },
    { sizeof("error_message/list") - 1, "error_message/list", apiclient_error_list },
    { sizeof("html/") - 1, "html/", apiclient_html },
    { sizeof("post/test") - 1, "post/test", apiclient_post_test },
    { sizeof("service/get_version") - 1, "service/get_version", apiclient_service_get_version },
    { sizeof("sso/login") - 1, "sso/login", apiclient_sso_login }
};

#define APIROUTE_LIST_SIZE sizeof(s_apiroutes)/sizeof(CelKeyword)

CelWmipContext s_wmip_ctx;

CelWmipClient *apiclient_new(size_t size, int fd)
{
    return connection_new(fd, API_CLIENT, NULL);
}

void apiclient_free(CelWmipClient *client)
{
    connection_free(client);
}

int apiserver_listen(CelWmipListener *listener, 
                     const TCHAR *address, CelSslContext *wmip_sslctx)
{
    CelSockAddr addr;

    snprintf(s_wmip_ctx.server, 
        CEL_HNLEN, "sunruncel-demo %s", cel_version_release(&celdemo_ver));
    s_wmip_ctx.version.key_len = sizeof("/sunruncel/") - 1;
    s_wmip_ctx.version.key_word = "/sunruncel/";
    s_wmip_ctx.version.value = NULL;
    s_wmip_ctx.new_func = (CelWmipClientNewFunc)apiclient_new;
    s_wmip_ctx.free_func = (CelWmipClientFreeFunc)apiclient_free;
    s_wmip_ctx.ops = s_apiroutes;
    s_wmip_ctx.ops_size = APIROUTE_LIST_SIZE;
    if (cel_sockaddr_init_str(&addr, address) != -1)
    {
        if (cel_wmiplistener_init(listener, &addr, wmip_sslctx, &s_wmip_ctx) != -1)
        {
            if (cel_wmiplistener_run_group(listener, &evt_loop_grp) != -1)
            {
                Info((_T("Wmip listener %s start."), 
                    cel_wmiplistener_get_localaddrs(listener)));
                return 0;
            }
            cel_wmiplistener_destroy(listener);
        }
    }
    Err((_T("Wmip listener %s start failed(%s)"), address, cel_geterrstr(cel_geterrno())));

    return -1;
}

int apiclient_conf_getstatus(CelWmipClient *wmipclt, CelWmipMessage *message)
{
    return cel_wmipclient_async_response_send_file(wmipclt,
        "/etc/sunrun/cel-demo.conf", 0, 0,
        cel_httprequest_get_header(&(wmipclt->wmip_msg->req), 
        CEL_HTTPHDR_IF_MODIFIED_SINCE), NULL, NULL);
}

int apiclient_error_list(CelWmipClient *wmipclt, CelWmipMessage *message)
{
    int error;
    const char *err_str;

    cel_wmipclient_resize_response_send_buffer(wmipclt, 256 * 1024);
    cel_wmipclient_set_response_header(wmipclt, 
        CEL_HTTPHDR_CONTENT_TYPE,
        "application/json", sizeof("application/json") - 1);
    cel_wmipclient_response_printf(wmipclt, 
        "{\"error_list\":["
        "{\"error\":%d,\"message\":\"%s\"}", 0, deamo_geterrmsg(0));
    for (error = 1; error < ERR_COUNT; error++)
    {
        if ((err_str = deamo_geterrmsg(error)) == NULL)
        {
            error = (error / 100 + 1) * 100;
            continue;
        }
        if (!deamo_checkerrmsg(error))
            break;
        cel_wmipclient_response_printf(wmipclt, 
            ",{\"error\":%d,\"message\":\"%s\"}", error, err_str);
    }
    cel_wmipclient_response_printf(wmipclt, "]}");
    cel_wmipclient_response_end(wmipclt);
    //puts(wmipclt->wmip_msg->rsp.s.buffer);
    return cel_wmipclient_result(wmipclt, message, CEL_WMIPSC_OK, 0, NULL);
}

static int apiclient_html(CelWmipClient *wmipclt, CelWmipMessage *message)
{
    char path[CEL_PATHLEN];
    int size1;
    size_t size2;
    CelDateTime dt;
    //char str[64];

    puts((char *)message->req.s.buffer);
    size1 = snprintf(path, CEL_PATHLEN, "/mnt/hgfs/e/work/solution/cel-demo/bin/html/");
    size2 = CEL_PATHLEN - size1;
    if (cel_wmipclient_get_request_file_path(wmipclt, path + size1, &size2) == NULL)
        return cel_wmipclient_result(wmipclt, message, 
        CEL_WMIPSC_NOT_FOUND_EXCEPTION, 0, cel_geterrstr_a(cel_geterrno()));
    printf("%s %s\r\n", path, cel_wmipclient_get_remoteaddrs(wmipclt));
    cel_httpresponse_set_header(&(wmipclt->wmip_msg->rsp), 
        CEL_HTTPHDR_PRAGMA, "Pragma", strlen("Pragma"));
    cel_httpresponse_set_header(&(wmipclt->wmip_msg->rsp), 
        CEL_HTTPHDR_CACHE_CONTROL, "max-age=43200", strlen("max-age=43200"));
    /*cel_httpresponse_set_header(&(wmipclt->wmip_msg->rsp), 
    CEL_HTTPHDR_CACHE_CONTROL, "public", strlen("public"));*/
    cel_datetime_init_now(&dt);
    cel_datetime_add_days(&dt, 30);
    cel_httpresponse_set_header(&(wmipclt->wmip_msg->rsp), 
        CEL_HTTPHDR_EXPIRES, &dt, sizeof(CelDateTime));

    /*printf("dat1 = %ld\r\n", dt);
    cel_datetime_strfgmtime_a(&dt, str, 64, "%a, %d %b %Y %X GMT");
    printf("dat2 = %s\r\n", str);
    cel_datetime_init_strtime_a(&dt, str);
    printf("dat3 = %ld\r\n", dt);*/

    return cel_wmipclient_async_response_send_file(wmipclt, path, 0, 0, 
        cel_httprequest_get_header(&(wmipclt->wmip_msg->req),
        CEL_HTTPHDR_IF_MODIFIED_SINCE), NULL, NULL);
}

int apiclient_post_test(CelWmipClient *wmipclt, CelWmipMessage *message)
{
    CelHttpMultipart *multipart;
    CelHttpMultipartEntity *entity;

    if (cel_httprequest_is_multipart(&(wmipclt->wmip_msg->req)))
    {
        //puts("sdklfdkfkl");
        multipart = cel_httprequest_get_multipart(&(wmipclt->wmip_msg->req));
        entity = cel_httpmultipart_get_first_entity(multipart);
        if (entity != NULL)
        {
            //puts("qqqq");
            if (cel_httpmulitpart_entity_move_file(entity, "e:\\x.data") != 0)
                puts("cel_httpmulitpart_entity_move_file error");
        }
    }
    return cel_wmipclient_result(wmipclt, message, CEL_WMIPSC_OK, 0, NULL);
    //char buf[128];

    //puts((char *)wmipclt->wmip_msg->req.s.buffer);
    //cel_httprequest_save_body_data(&(wmipclt->wmip_msg->req), 0,
    //    cel_httprequest_get_body_size(&(wmipclt->wmip_msg->req)) - 1, 
    //    "e:\\x.jpg");
    //puts("apiclient_post_test");
    ////puts((char *)message->req.body_data.buffer);
    //printf("socpe= %s\r\n", 
    //    cel_httprequest_get_params_(&(wmipclt->wmip_msg->req), "scope", buf, 128));
    return cel_wmipclient_result(wmipclt, message, CEL_WMIPSC_OK, 0, NULL);
}

CelWmipContext wmip_ctx;

int post_file()
{
    CelWmipClient *client;
    CelHttpConnection connection = CEL_HTTPCON_KEEPALIVE;

    memset(&wmip_ctx, 0 ,sizeof(CelWmipContext));
    client = cel_wmipclient_new(NULL, &wmip_ctx);

    cel_wmipclient_set_nonblock(client, 1);
    cel_eventloopgroup_add_channel(&evt_loop_grp, -1, cel_wmipclient_get_channel(client), NULL);

    //cel_wmipclient_set_method(client,CEL_HTTPM_POST);

    cel_httprequest_post_file(&(client->wmip_msg->req), CEL_HTTPM_POST, 
        "https://192.168.2.43:9045/sunruncel/post/test", "e:\\20170125094814.jpg", 0, 0);

    //puts((char *)client->wmip_msg->req.s.buffer);

    return cel_wmipclient_async_send_request(client, NULL);
}

void request_calllback(CelWmipClient *client, CelAsyncResult *result)
{
    puts("xxxxx");
    puts((char *)client->wmip_msg->rsp.body_cache.buf.buffer);
    //cel_wmipclient_request_clear(client);
    //cel_wmipclient_response_clear(client);
    //cel_wmipclient_set_method(client, CEL_HTTPM_POST);
    //
    //cel_wmipclient_set_url(client, "https://192.168.2.43:9045/sunruncel/post/test");
    //cel_wmipclient_set_request_header(client, CEL_HTTPHDR_CONTENT_TYPE,
    //    "application/x-www-form-urlencoded; charset=UTF-8", 
    //    strlen("application/x-www-form-urlencoded; charset=UTF-8"));
    ///*cel_wmipclient_set_request_header(client, CEL_HTTPHDR_COOKIE, 
    //    "JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB", 
    //    strlen("JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB"));
    //cel_wmipclient_set_request_header(client,
    //    CEL_HTTPHDR_CONNECTION, &connection, sizeof(CelHttpConnection));*/
    //cel_wmipclient_request_printf(client, "scope=sunruniam-api%%3A*%%3A*");
    //cel_wmipclient_request_end(client);

    //puts((char *)client->wmip_msg->req.s.buffer);

    //cel_wmipclient_async_send_request(client, NULL);
}

int send_request_test()
{
    CelWmipClient *client;
    CelHttpConnection connection = CEL_HTTPCON_KEEPALIVE;

    memset(&wmip_ctx, 0 ,sizeof(CelWmipContext));
    client = cel_wmipclient_new(NULL, &wmip_ctx);

    cel_wmipclient_set_nonblock(client, 1);
    cel_eventloopgroup_add_channel(&evt_loop_grp, -1, cel_wmipclient_get_channel(client), NULL);

    //cel_wmipclient_set_method(client,CEL_HTTPM_POST);
    
    //cel_wmipclient_set_url(client, "https://192.168.2.43:9045/sunruncel/post?x=t");
    //cel_wmipclient_set_url(client, "https://10.11.15.253/sunruniam-admin/index.php/Home/Login/login_Out");

    //cel_wmipclient_set_request_header(client, CEL_HTTPHDR_CONTENT_TYPE,
    //    "application/x-www-form-urlencoded; charset=UTF-8", 
    //    strlen("application/x-www-form-urlencoded; charset=UTF-8"));
    /*cel_wmipclient_set_request_header(client, CEL_HTTPHDR_COOKIE, 
        "JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB", 
        strlen("JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB"));*/
    //cel_wmipclient_set_request_header(client,
    //    CEL_HTTPHDR_CONNECTION, &connection, sizeof(CelHttpConnection));
    //cel_wmipclient_request_printf(client, "scope=sunruniam-api%%3A*%%3A*");
    cel_wmipclient_set_url(client, 
        "http://10.21.21.252:8080/cas/serviceValidate?ticket=ST-18-czB-6J6LRnbKfx-YdgsPOdpGC34-localhost&service=http%3A%2F%2Fwww.baidu.com");
    cel_wmipclient_request_end(client);

    puts((char *)client->wmip_msg->req.s.buffer);

    return cel_wmipclient_async_send_request(client, request_calllback);
}

int apiclient_service_get_version(CelWmipClient *wmipclt, CelWmipMessage *message)
{
    char arg1[128];
    size_t arg1_size = 128;
    CelHttpResponse *rsp = cel_wmipclient_get_httpresponse(wmipclt);
    CelHttpCookie set_cookie;
    CelHttpMethod method;

    //_tprintf("xx %s\r\n", cel_wmipclient_get_remoteaddrs(wmipclt));
    //post_file();
    //_tprintf("yy %s\r\n",  cel_wmipclient_get_remoteaddrs(wmipclt));
    send_request_test();

    method = cel_httprequest_get_method(&(message->req));
    printf("method = %d\r\n", method);

    if (cel_httprequest_get_params(&(message->req), "service", arg1, &arg1_size) != NULL)
        puts(arg1);
    cel_httpcookie_init(&set_cookie);
    cel_httpcookie_set_value(&set_cookie, "xrt1", "123456", strlen("123456"));
    cel_httpcookie_set_value(&set_cookie, "xrt", "123456", strlen("123456"));
    cel_httpcookie_set_value(&set_cookie, "TGC", "123456", strlen("123456"));
    cel_httpcookie_set_attribute(&set_cookie, NULL, 0, NULL, NULL, FALSE, TRUE);

    //cel_httpresponse_set_header(rsp, CEL_HTTPHDR_CONTENT_TYPE, "text/html", strlen("text/html"));
    cel_httpresponse_set_header(rsp, CEL_HTTPHDR_SET_COOKIE, &set_cookie, sizeof(CelHttpCookie));
    cel_httpcookie_destroy(&set_cookie);

    //return cel_wmipclient_async_response_send_redirect(wmipclt, 
    // "/sunruncel-demo/html/login.html?lt=LT-djkdfjkfdjfdjk&service=dkldfklfdlk", NULL);

    cel_wmipclient_response_printf(wmipclt, "{"CEL_CRLF_A
        "    \"version\":\"%s\""CEL_CRLF_A
        "}"CEL_CRLF_A, cel_version_release(&celdemo_ver));
    Info((_T("Wmip client %s request [%s, %d]."), 
        cel_wmipclient_get_remoteaddrs(wmipclt), 
        wmipclt->wmip_ctx->ops[wmipclt->op_index].key_word, CEL_WMIPSC_OK));

    return cel_wmipclient_result(wmipclt, message, CEL_WMIPSC_OK, 0, NULL);
}

int apiclient_sso_login(CelWmipClient *wmipclt, CelWmipMessage *message)
{
    char value[256];
    size_t size = 256;

    puts((char *)message->req.s.buffer);
    if (message->req.body_cache.buf.buffer != NULL)
        puts((char *)message->req.body_cache.buf.buffer);
    cel_httprequest_get_params(&(message->req), "access_token", value, &size);
    puts(value);
    printf("apiclient_sso_login %s\r\n", cel_wmipclient_get_remoteaddrs(wmipclt));
    return cel_wmipclient_result(wmipclt, message, CEL_WMIPSC_OK, 0, NULL);
}

