#include "apiserver.h"
#include "connection.h"
#include "error.h"
#include "log.h"
#include "cel/convert.h"
#include "cel/file.h"
#include "cel/sys/perf.h"

static int demo_conf_get(CelHttpWebClient *api_client);
static int demo_error_list(CelHttpWebClient *api_client);
static int demo_html(CelHttpWebClient *api_client);
static int demo_post_test(CelHttpWebClient *api_client);
static int demo_service_get_version(CelHttpWebClient *api_client);
static int demo_sso_login(CelHttpWebClient *api_client);
static int demp_orgs_users_get(CelHttpWebClient *api_client);

CelSslContext *api_sslctx = NULL;
CelHttpWebContext s_api_ctx;
CelHttpWebListener api_listener;

CelHttpWebClient *demo_new(size_t size, int fd)
{
    return connection_new(fd, API_CLIENT, NULL);
}

void demo_free(CelHttpWebClient *api_client)
{
    connection_free(api_client);
}

int apiserver_listen(CelHttpWebListener *listener, 
                     const TCHAR *address, CelSslContext *api_sslctx)
{
    CelSockAddr addr;
    CelHttpRoute *route;

    snprintf(s_api_ctx.server, 
        CEL_HNLEN, "cel-demo %s", cel_version_release(&celdemo_ver));
    s_api_ctx.prefix.key_len = sizeof("/demo/") - 1;
    s_api_ctx.prefix.key_word = "/demo/";
    s_api_ctx.prefix.value = NULL;

    route = &s_api_ctx.route;
    cel_httproute_init(route);
    cel_httproute_add(route, CEL_HTTPM_GET, "conf/get", 
        (CelHttpHandleFunc)demo_conf_get);
    cel_httproute_add(route, CEL_HTTPM_GET, "error/list", 
        (CelHttpHandleFunc)demo_error_list);
    cel_httproute_add(route, CEL_HTTPM_GET, "html", 
        (CelHttpHandleFunc)demo_html);
    cel_httproute_add(route, CEL_HTTPM_GET, "service/get_version", 
        (CelHttpHandleFunc)demo_service_get_version);
    cel_httproute_add(route, CEL_HTTPM_POST, "sso/login", 
        (CelHttpHandleFunc)demo_sso_login);
    cel_httproute_add(route, CEL_HTTPM_GET, "orgs/{org_name}/users/{user_name}",
        (CelHttpHandleFunc)demp_orgs_users_get);

    s_api_ctx.new_func = (CelHttpWebClientNewFunc)demo_new;
    s_api_ctx.free_func = (CelHttpWebClientFreeFunc)demo_free;
    if (cel_sockaddr_init_str(&addr, address) != -1)
    {
        if (cel_httpweblistener_init(listener, &addr, api_sslctx, &s_api_ctx) != -1)
        {
            if (cel_httpweblistener_run_group(listener, &evt_loop_grp) != -1)
            {
                Info((_T("Api listener %s start."), 
                    cel_httpweblistener_get_localaddrs(listener)));
                return 0;
            }
            cel_httpweblistener_destroy(listener);
        }
    }
    Err((_T("Api listener %s start failed(%s)"),
        address, cel_geterrstr(cel_geterrno())));

    return -1;
}

int demo_conf_get(CelHttpWebClient *api_client)
{
    CelHttpRequest *req = cel_httpwebclient_get_request(api_client);
    CelHttpResponse *rsp = cel_httpwebclient_get_response(api_client);

    cel_httpresponse_send_file(rsp, "/etc/sunrun/cel-demo.conf", 0, 0, 
        cel_httprequest_get_header(req, CEL_HTTPHDR_IF_MODIFIED_SINCE), NULL);

    return cel_httpwebclient_async_send_response(api_client, NULL);
}

int demo_error_list(CelHttpWebClient *api_client)
{
    CelHttpResponse *rsp = cel_httpwebclient_get_response(api_client);
    int error;
    const char *err_str;

    cel_httpresponse_resize_send_buffer(rsp, 256 * 1024);
    cel_httpresponse_set_header(rsp, CEL_HTTPHDR_CONTENT_TYPE,
        "application/json", sizeof("application/json") - 1);
    cel_httpresponse_printf(rsp, 
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
        cel_httpresponse_printf(rsp, 
            ",{\"error\":%d,\"message\":\"%s\"}", error, err_str);
    }
    cel_httpresponse_printf(rsp, "]}");
    cel_httpresponse_end(rsp);
    //puts(api_client->rsp.s.buffer);
    return cel_httpwebclient_async_send_response(api_client, NULL);
}

static int demo_html(CelHttpWebClient *api_client)
{
    CelHttpRequest *req = cel_httpwebclient_get_request(api_client);
    CelHttpResponse *rsp = cel_httpwebclient_get_response(api_client);
    char path[CEL_PATHLEN];
    int size1;
    size_t size2;
    CelDateTime dt;
    //char str[64];

    puts((char *)api_client->req.s.buffer);
    size1 = snprintf(path, CEL_PATHLEN, "/mnt/hgfs/e/work/solution/cel-demo/bin/html/");
    size2 = CEL_PATHLEN - size1;
    /*if (cel_httpwebclient_get_request_file_path(api_client, path + size1, &size2) == NULL)
        return cel_httpwebclient_async_send_response_result(api_client, 
        CEL_HTTPWEB_NOT_FOUND_EXCEPTION, 0, cel_geterrstr_a(cel_geterrno()));*/
    printf("%s %s\r\n", path, cel_httpwebclient_get_remoteaddrs(api_client));
    cel_httpresponse_set_header(rsp, CEL_HTTPHDR_PRAGMA, "Pragma", strlen("Pragma"));
    cel_httpresponse_set_header(rsp, 
        CEL_HTTPHDR_CACHE_CONTROL, "max-age=43200", strlen("max-age=43200"));
    /*cel_httpresponse_set_header(rsp, 
    CEL_HTTPHDR_CACHE_CONTROL, "public", strlen("public"));*/
    cel_datetime_init_now(&dt);
    cel_datetime_add_days(&dt, 30);
    cel_httpresponse_set_header(rsp, 
        CEL_HTTPHDR_EXPIRES, &dt, sizeof(CelDateTime));

    /*printf("dat1 = %ld\r\n", dt);
    cel_datetime_strfgmtime_a(&dt, str, 64, "%a, %d %b %Y %X GMT");
    printf("dat2 = %s\r\n", str);
    cel_datetime_init_strtime_a(&dt, str);
    printf("dat3 = %ld\r\n", dt);*/
    cel_httpresponse_send_file(rsp, path, 0, 0, 
        cel_httprequest_get_header(&(api_client->req), 
        CEL_HTTPHDR_IF_MODIFIED_SINCE), NULL);

    return cel_httpwebclient_async_send_response(api_client, NULL);
}

int demo_post_test(CelHttpWebClient *api_client)
{
    CelHttpRequest *req = cel_httpwebclient_get_request(api_client);
    CelHttpMultipart *multipart;
    CelHttpMultipartEntity *entity;

    if (cel_httprequest_is_multipart(req))
    {
        //puts("sdklfdkfkl");
        multipart = cel_httprequest_get_multipart(req);
        entity = cel_httpmultipart_get_first_entity(multipart);
        if (entity != NULL)
        {
            //puts("qqqq");
            if (cel_httpmulitpart_entity_move_file(entity, "e:\\x.data") != 0)
                puts("cel_httpmulitpart_entity_move_file error");
        }
    }
    return cel_httpwebclient_async_send_response_result(api_client, CEL_HTTPWEB_OK, 0, NULL);
    //char buf[128];

    //puts((char *)api_client->req.s.buffer);
    //cel_httprequest_save_body_data(&(api_client->req), 0,
    //    cel_httprequest_get_body_size(&(api_client->req)) - 1, 
    //    "e:\\x.jpg");
    //puts("demo_post_test");
    ////puts((char *)api_client->req.body_data.buffer);
    //printf("socpe= %s\r\n", 
    //    cel_httprequest_get_params_(&(api_client->req), "scope", buf, 128));
    return cel_httpwebclient_async_send_response_result(api_client, CEL_HTTPWEB_OK, 0, NULL);
}

CelHttpWebContext api_ctx;

int post_file()
{
    CelHttpRequest *req;
    CelHttpWebClient *api_client;
    CelHttpConnection connection = CEL_HTTPCON_KEEPALIVE;

    memset(&api_ctx, 0 ,sizeof(CelHttpWebContext));
    api_client = cel_httpwebclient_new(NULL, &api_ctx);

    cel_httpwebclient_set_nonblock(api_client, 1);
    cel_eventloopgroup_add_channel(&evt_loop_grp, 
        -1, cel_httpwebclient_get_channel(api_client), NULL);

    req = cel_httpwebclient_get_request(api_client);

    //cel_httpwebclient_set_method(api_client,CEL_HTTPM_POST);

    cel_httprequest_post_file(req, CEL_HTTPM_POST, 
        "https://192.168.2.43:9045/sunruncel/post/test", "e:\\20170125094814.jpg", 0, 0);

    //puts((char *)api_client->req.s.buffer);

    return cel_httpwebclient_async_execute_request(api_client, NULL);
}

void request_calllback(CelHttpWebClient *api_client, CelAsyncResult *result)
{
    CelHttpRequest *req = cel_httpwebclient_get_request(api_client);

    puts("xxxxx");
    puts((char *)req->body_cache.buf.buffer);
    //cel_httprequest_clear(req);
    //cel_httpresponse_clear(req);
    //cel_httprequest_set_method(req, CEL_HTTPM_POST);
    //
    //cel_httprequest_set_url_str(req, "https://192.168.2.43:9045/sunruncel/post/test");
    //cel_httprequest_set_header(req, CEL_HTTPHDR_CONTENT_TYPE,
    //    "application/x-www-form-urlencoded; charset=UTF-8", 
    //    strlen("application/x-www-form-urlencoded; charset=UTF-8"));
    ///*cel_httprequest_set_header(req, CEL_HTTPHDR_COOKIE, 
    //    "JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB", 
    //    strlen("JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB"));
    //cel_httprequest_set_header(req,
    //    CEL_HTTPHDR_CONNECTION, &connection, sizeof(CelHttpConnection));*/
    //cel_httprequest_printf(req, "scope=sunruniam-api%%3A*%%3A*");
    //cel_httprequest_end(req);

    //puts((char *)api_client->req.s.buffer);

    //cel_httpwebclient_async_execute_request(api_client, NULL);
}

int send_request_test()
{
    CelHttpRequest *req;
    CelHttpWebClient *api_client;
    CelHttpConnection connection = CEL_HTTPCON_KEEPALIVE;

    memset(&api_ctx, 0 ,sizeof(CelHttpWebContext));
    api_client = cel_httpwebclient_new(NULL, &api_ctx);

    req = cel_httpwebclient_get_request(api_client);

    cel_httpwebclient_set_nonblock(api_client, 1);
    cel_eventloopgroup_add_channel(&evt_loop_grp, 
        -1, cel_httpwebclient_get_channel(api_client), NULL);

    //cel_httprequest_set_method(req, CEL_HTTPM_POST);
    
    //cel_httprequest_set_url(req, "https://192.168.2.43:9045/sunruncel/post?x=t");
    //cel_httprequest_set_url(req, "https://10.11.15.253/sunruniam-admin/index.php/Home/Login/login_Out");

    //cel_httprequest_set_header(req, CEL_HTTPHDR_CONTENT_TYPE,
    //    "application/x-www-form-urlencoded; charset=UTF-8", 
    //    strlen("application/x-www-form-urlencoded; charset=UTF-8"));
    /*cel_httprequest_set_header(req, CEL_HTTPHDR_COOKIE, 
        "JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB", 
        strlen("JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB"));*/
    //cel_httprequest_set_header(req,
    //    CEL_HTTPHDR_CONNECTION, &connection, sizeof(CelHttpConnection));
    //cel_httprequest_printf(req, "scope=sunruniam-api%%3A*%%3A*");
    cel_httprequest_set_url_str(req, 
        "http://10.21.21.252:8080/cas/serviceValidate?ticket=ST-18-czB-6J6LRnbKfx-YdgsPOdpGC34-localhost&service=http%3A%2F%2Fwww.baidu.com");
    cel_httprequest_end(req);

    puts((char *)req->s.buffer);

    return cel_httpwebclient_async_execute_request(api_client, request_calllback);
}

int demo_service_get_version(CelHttpWebClient *api_client)
{
    char arg1[128];
    size_t arg1_size = 128;
    CelHttpResponse *rsp = cel_httpwebclient_get_response(api_client);
    CelHttpCookie set_cookie;
    CelHttpMethod method;

    //_tprintf("xx %s\r\n", cel_httpwebclient_get_remoteaddrs(api_client));
    //post_file();
    //_tprintf("yy %s\r\n",  cel_httpwebclient_get_remoteaddrs(api_client));
    send_request_test();

    method = cel_httprequest_get_method(&(api_client->req));
    printf("method = %d\r\n", method);

    if (cel_httprequest_get_params(&(api_client->req), "service", arg1, &arg1_size) != NULL)
        puts(arg1);
    cel_httpcookie_init(&set_cookie);
    cel_httpcookie_set_value(&set_cookie, "xrt1", "123456", strlen("123456"));
    cel_httpcookie_set_value(&set_cookie, "xrt", "123456", strlen("123456"));
    cel_httpcookie_set_value(&set_cookie, "TGC", "123456", strlen("123456"));
    cel_httpcookie_set_attribute(&set_cookie, NULL, 0, NULL, NULL, FALSE, TRUE);

    //cel_httpresponse_set_header(rsp, CEL_HTTPHDR_CONTENT_TYPE, "text/html", strlen("text/html"));
    cel_httpresponse_set_header(rsp, CEL_HTTPHDR_SET_COOKIE, &set_cookie, sizeof(CelHttpCookie));
    cel_httpcookie_destroy(&set_cookie);

    //return cel_httpwebclient_async_response_send_redirect(api_client, 
    // "/sunruncel-demo/html/login.html?lt=LT-djkdfjkfdjfdjk&service=dkldfklfdlk", NULL);

    cel_httpresponse_printf(&(api_client->rsp), "{"CEL_CRLF_A
        "    \"version\":\"%s\""CEL_CRLF_A
        "}"CEL_CRLF_A, cel_version_release(&celdemo_ver));
    Info((_T("Api api_client %s request [%s, %d]."), 
        cel_httpwebclient_get_remoteaddrs(api_client), 
        cel_httpwebclient_get_route_path(api_client), CEL_HTTPWEB_OK));

    return cel_httpwebclient_async_send_response_result(api_client, CEL_HTTPWEB_OK, 0, NULL);
}

int demo_sso_login(CelHttpWebClient *api_client)
{
    char value[256];
    size_t size = 256;

    puts((char *)api_client->req.s.buffer);
    if (api_client->req.body_cache.buf.buffer != NULL)
        puts((char *)api_client->req.body_cache.buf.buffer);
    cel_httprequest_get_params(&(api_client->req), "access_token", value, &size);
    puts(value);
    printf("demo_sso_login %s\r\n", cel_httpwebclient_get_remoteaddrs(api_client));
    return cel_httpwebclient_async_send_response_result(api_client, CEL_HTTPWEB_OK, 0, NULL);
}

int demp_orgs_users_get(CelHttpWebClient *api_client)
{
    CelHttpRouteData *rt_dt = cel_httpwebclient_get_routedata(api_client);
    char value[128];
    size_t size = 128;

    puts(cel_httproutedata_get(rt_dt, "org_name", value, &size));
    size = 128;
    puts(cel_httproutedata_get(rt_dt, "user_name", value, &size));
    return cel_httpwebclient_async_send_response_result(api_client, CEL_HTTPWEB_OK, 0, NULL);
}
