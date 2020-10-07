#include "apiserver.h"
#include "error.h"
#include "log.h"
#include "cel/convert.h"
#include "cel/file.h"
#include "cel/sys/perf.h"

static int demo_conf_get(CelHttpContext *http_ctx);
static int demo_error_list(CelHttpContext *http_ctx);
static int demo_html(CelHttpContext *http_ctx);
static int demo_post_file(CelHttpContext *http_ctx);
static int demo_service_get_version(CelHttpContext *http_ctx);
static int demo_sso_login(CelHttpContext *http_ctx);
static int demp_orgs_users_get(CelHttpContext *http_ctx);

CelSslContext *api_sslctx = NULL;
CelHttpServeContext s_api_ctx;
CelHttpServe api_listener;

int apiserver_listen(CelHttpServe *listener, 
                     const TCHAR *address, CelSslContext *api_sslctx)
{
    CelSockAddr addr;
    CelHttpRoute *route;

    route = &s_api_ctx.route;
    cel_httproute_init(route, "/demo/");
    cel_httproute_add(route, CEL_HTTPM_GET, "conf/get", 
        (CelHttpRouteHandleFunc)demo_conf_get);
    cel_httproute_add(route, CEL_HTTPM_GET, "error/list", 
        (CelHttpRouteHandleFunc)demo_error_list);
    cel_httproute_add(route, CEL_HTTPM_GET, "html", 
        (CelHttpRouteHandleFunc)demo_html);
    cel_httproute_add(route, CEL_HTTPM_GET, "service/get_version", 
        (CelHttpRouteHandleFunc)demo_service_get_version);
    cel_httproute_add(route, CEL_HTTPM_POST, "sso/login", 
        (CelHttpRouteHandleFunc)demo_sso_login);
    cel_httproute_add(route, CEL_HTTPM_POST, "post/file", 
        (CelHttpRouteHandleFunc)demo_post_file);
    cel_httproute_add(route, CEL_HTTPM_GET, "orgs/<org_name>/users/<user_name>",
        (CelHttpRouteHandleFunc)demp_orgs_users_get);

    snprintf(s_api_ctx.server, 
        CEL_HNLEN, "cel-demo %s", cel_version_release(&ver));
    s_api_ctx.new_func = NULL;
    s_api_ctx.free_func = NULL;

    if (cel_sockaddr_init_str(&addr, address) != -1)
    {
        if (cel_httpserve_init(listener, &addr, api_sslctx, &s_api_ctx) != -1)
        {
            if (cel_httpserve_run_group(listener, &evt_loop_grp) != -1)
            {
                Info((_T("Api listener %s start"), 
                    cel_httpserve_get_localaddrs(listener)));
                return 0;
            }
            cel_httpserve_destroy(listener);
        }
    }
    Err((_T("Api listener %s start failed(%s)"),
        address, cel_geterrstr()));

    return -1;
}

int demo_conf_get(CelHttpContext *http_ctx)
{
    cel_httpresponse_send_file(&(http_ctx->rsp), "/etc/sunrun/cel-demo.conf", 0, 0, 
        (CelTime *)cel_httprequest_get_header(
        &(http_ctx->req), CEL_HTTPHDR_IF_MODIFIED_SINCE), NULL);

    cel_httpcontext_response_write(http_ctx, 200, 0, NULL);
	return CEL_RET_OK;
}

int demo_error_list(CelHttpContext *http_ctx)
{
    int error;
    const char *err_str;

    cel_httpresponse_resize_send_buffer(&(http_ctx->rsp), 256 * 1024);
    cel_httpresponse_set_header(&(http_ctx->rsp), CEL_HTTPHDR_CONTENT_TYPE,
        "application/json", sizeof("application/json") - 1);
    cel_httpresponse_printf(&(http_ctx->rsp), 
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
        cel_httpresponse_printf(&(http_ctx->rsp), 
            ",{\"error\":%d,\"message\":\"%s\"}", error, err_str);
    }
    cel_httpresponse_printf(&(http_ctx->rsp), "]}");
    cel_httpresponse_end(&(http_ctx->rsp));
    //puts(api_client->http_ctx->rsp.s.buffer);
    cel_httpcontext_response_write(http_ctx, 200, 0, NULL);
	return CEL_RET_OK;
}

static int demo_html(CelHttpContext *http_ctx)
{
    char path[CEL_PATHLEN];
    int size1;
    size_t size2;
    CelTime dt;
    //char str[64];

	puts((char *)http_ctx->req.hs.s.buffer);
    size1 = snprintf(path, CEL_PATHLEN, "/mnt/hgfs/e/work/solution/cel-demo/bin/html/");
    size2 = CEL_PATHLEN - size1;
    /*if (cel_httpwebrequest_get_request_file_path(api_client, path + size1, &size2) == NULL)
        return cel_httpwebrequest_async_send_response_result(api_client, 
        CEL_HTTPCONTEXT_NOT_FOUND_EXCEPTION, 0, cel_geterrstr_a());*/
    //printf("%s %s\r\n", path, cel_httpwebrequest_get_remoteaddr_str(api_client));
    cel_httpresponse_set_header(&(http_ctx->rsp), CEL_HTTPHDR_PRAGMA, "Pragma", strlen("Pragma"));
    cel_httpresponse_set_header(&(http_ctx->rsp), 
        CEL_HTTPHDR_CACHE_CONTROL, "max-age=43200", strlen("max-age=43200"));
    /*cel_httpresponse_set_header(&(http_ctx->rsp), 
    CEL_HTTPHDR_CACHE_CONTROL, "public", strlen("public"));*/
    cel_time_init_now(&dt);
    cel_time_add_days(&dt, 30);
    cel_httpresponse_set_header(&(http_ctx->rsp), 
        CEL_HTTPHDR_EXPIRES, &dt, sizeof(CelTime));

    /*printf("dat1 = %ld\r\n", dt);
    cel_time_strfgmtime_a(&dt, str, 64, "%a, %d %b %Y %X GMT");
    printf("dat2 = %s\r\n", str);
    cel_time_init_strtime_a(&dt, str);
    printf("dat3 = %ld\r\n", dt);*/
    cel_httpcontext_response_sendfile(http_ctx, path, 0, 0, 
        (CelTime *)cel_httprequest_get_header(&(http_ctx->req), 
        CEL_HTTPHDR_IF_MODIFIED_SINCE), NULL);

    return CEL_RET_OK;
}

int demo_post_file(CelHttpContext *http_ctx)
{
    CelHttpMultipart *multipart;
    CelHttpMultipartEntity *entity;

    puts("xxx");
    if (cel_httprequest_is_multipart(&(http_ctx->req)))
    {
        //puts("sdklfdkfkl");
        multipart = cel_httprequest_get_multipart(&(http_ctx->req));
        entity = cel_httpmultipart_get_first_entity(multipart);
        if (entity != NULL)
        {
            //puts("qqqq");
            if (cel_httpmulitpart_entity_move_file(entity, cel_fullpath_a("./x.data")) != 0)
                puts("cel_httpmulitpart_entity_move_file error");
            else
                puts(cel_fullpath_a("./x.data"));
        }
    }
    cel_httpcontext_response_write(http_ctx, 200, 0, NULL);
    //char buf[128];

    //puts((char *)api_client->req.s.buffer);
    //cel_httprequest_save_body_data(&(api_client->req), 0,
    //    cel_httprequest_get_body_size(&(api_client->req)) - 1, 
    //    "e:\\x.jpg");
    //puts("demo_post_test");
    ////puts((char *)api_client->req.body_data.buffer);
    //printf("socpe= %s\r\n", 
    //    cel_httprequest_get_params_(&(api_client->req), "scope", buf, 128));
    //cel_httpcontext_response_write(http_ctx, 200, 0, NULL);
	return CEL_RET_OK;
}

//CelHttpServeContext api_ctx;
//
//int post_file()
//{
//    CelHttpRequest *req;
//    CelHttpWebClient *api_client;
//    CelHttpConnection connection = CEL_HTTPCON_KEEPALIVE;
//
//    memset(&api_ctx, 0 ,sizeof(CelHttpServeContext));
//    api_client = cel_httpwebrequest_new(NULL, &api_ctx);
//
//    cel_httpwebrequest_set_nonblock(api_client, 1);
//    cel_eventloopgroup_add_channel(&evt_loop_grp, 
//        -1, cel_httpwebrequest_get_channel(api_client), NULL);
//
//    req = cel_httpwebrequest_get_request(api_client);
//
//    //cel_httpwebrequest_set_method(api_client,CEL_HTTPM_POST);
//
//    cel_httprequest_post_file(req, CEL_HTTPM_POST, 
//        "https://192.168.2.43:9045/sunruncel/post/test", 
//        "e:\\20170125094814.jpg", 0, 0);
//
//    //puts((char *)api_client->req.s.buffer);
//
//    return cel_httpwebrequest_async_execute_request(api_client, NULL);
//}
//
//void request_calllback(CelHttpWebClient *api_client, CelAsyncResult *result)
//{
//    CelHttpRequest *req = cel_httpwebrequest_get_request(api_client);
//
//    puts("xxxxx");
//    puts((char *)req->body_cache.buf.buffer);
//    //cel_httprequest_clear(req);
//    //cel_httpresponse_clear(req);
//    //cel_httprequest_set_method(req, CEL_HTTPM_POST);
//    //
//    //cel_httprequest_set_url_str(req, "https://192.168.2.43:9045/sunruncel/post/test");
//    //cel_httprequest_set_header(req, CEL_HTTPHDR_CONTENT_TYPE,
//    //    "application/x-www-form-urlencoded; charset=UTF-8", 
//    //    strlen("application/x-www-form-urlencoded; charset=UTF-8"));
//    ///*cel_httprequest_set_header(req, CEL_HTTPHDR_COOKIE, 
//    //    "JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB", 
//    //    strlen("JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB"));
//    //cel_httprequest_set_header(req,
//    //    CEL_HTTPHDR_CONNECTION, &connection, sizeof(CelHttpConnection));*/
//    //cel_httprequest_printf(req, "scope=sunruniam-api%%3A*%%3A*");
//    //cel_httprequest_end(req);
//
//    //puts((char *)api_client->req.s.buffer);
//
//    //cel_httpwebrequest_async_execute_request(api_client, NULL);
//}
//
//int send_request_test()
//{
//    CelHttpRequest *req;
//    CelHttpWebClient *api_client;
//    CelHttpConnection connection = CEL_HTTPCON_KEEPALIVE;
//
//    memset(&api_ctx, 0 ,sizeof(CelHttpServeContext));
//    api_client = cel_httpwebrequest_new(NULL, &api_ctx);
//
//    req = cel_httpwebrequest_get_request(api_client);
//
//    cel_httpwebrequest_set_nonblock(api_client, 1);
//    cel_eventloopgroup_add_channel(&evt_loop_grp, 
//        -1, cel_httpwebrequest_get_channel(api_client), NULL);
//
//    //cel_httprequest_set_method(req, CEL_HTTPM_POST);
//    
//    //cel_httprequest_set_url(req, "https://192.168.2.43:9045/sunruncel/post?x=t");
//    //cel_httprequest_set_url(req, "https://10.11.15.253/sunruniam-admin/index.php/Home/Login/login_Out");
//
//    //cel_httprequest_set_header(req, CEL_HTTPHDR_CONTENT_TYPE,
//    //    "application/x-www-form-urlencoded; charset=UTF-8", 
//    //    strlen("application/x-www-form-urlencoded; charset=UTF-8"));
//    /*cel_httprequest_set_header(req, CEL_HTTPHDR_COOKIE, 
//        "JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB", 
//        strlen("JSESSIONID=64D21B4D69DFB3041B6375C1932BD6CB"));*/
//    //cel_httprequest_set_header(req,
//    //    CEL_HTTPHDR_CONNECTION, &connection, sizeof(CelHttpConnection));
//    //cel_httprequest_printf(req, "scope=sunruniam-api%%3A*%%3A*");
//    cel_httprequest_set_url_str(req, 
//        "http://10.21.21.252:8080/cas/serviceValidate?ticket=ST-18-czB-6J6LRnbKfx-YdgsPOdpGC34-localhost&service=http%3A%2F%2Fwww.baidu.com");
//    cel_httprequest_end(req);
//
//    puts((char *)req->s.buffer);
//
//    return cel_httpwebrequest_async_execute_request(api_client, request_calllback);
//}

int demo_service_get_version(CelHttpContext *http_ctx)
{
    char arg1[128];
    size_t arg1_size = 128;
    CelHttpCookie set_cookie;
    CelHttpMethod method;

    //_tprintf("xx %s\r\n", cel_httpwebrequest_get_remoteaddr_str(api_client));
    //post_file();
    //_tprintf("yy %s\r\n",  cel_httpwebrequest_get_remoteaddr_str(api_client));
    //send_request_test();

    method = cel_httprequest_get_method(&(http_ctx->req));
    printf("method = %d\r\n", method);

    if (cel_httprequest_get_params(&(http_ctx->req), "service", arg1, &arg1_size) != NULL)
        puts(arg1);
    cel_httpcookie_init(&set_cookie);
    cel_httpcookie_set_value(&set_cookie, "xrt1", "123456", strlen("123456"));
    cel_httpcookie_set_value(&set_cookie, "xrt", "123456", strlen("123456"));
    cel_httpcookie_set_value(&set_cookie, "TGC", "123456", strlen("123456"));
    cel_httpcookie_set_attribute(&set_cookie, NULL, 0, NULL, NULL, FALSE, TRUE);

    //cel_httpresponse_set_header(&(http_ctx->rsp), CEL_HTTPHDR_CONTENT_TYPE, "text/html", strlen("text/html"));
    cel_httpresponse_set_header(&(http_ctx->rsp), CEL_HTTPHDR_SET_COOKIE, &set_cookie, sizeof(CelHttpCookie));
    cel_httpcookie_destroy(&set_cookie);

    //return cel_httpwebrequest_async_response_send_redirect(api_client, 
    // "/sunruncel-demo/html/login.html?lt=LT-djkdfjkfdjfdjk&service=dkldfklfdlk", NULL);

    cel_httpresponse_printf(&(http_ctx->rsp), "{"CEL_CRLF_A
        "    \"version\":\"%s\""CEL_CRLF_A
        "}"CEL_CRLF_A, cel_version_release(&ver));
    Info((_T("Api api_client %s request [%s, %d]."), 
        cel_httpclient_get_remoteaddr_str(&(http_ctx->http_client)), 
        cel_httprequest_get_url_path(&(http_ctx->req)), 200));

    cel_httpcontext_response_write(http_ctx, 200, 0, NULL);
	return CEL_RET_OK;
}

int demo_sso_login(CelHttpContext *http_ctx)
{
    char value[256];
    size_t size = 256;

    puts((char *)http_ctx->req.hs.s.buffer);
    if (http_ctx->req.body_cache.buf.buffer != NULL)
        puts((char *)http_ctx->req.body_cache.buf.buffer);
    cel_httpcontext_get_param(http_ctx, "access_token", value, &size);
    puts(value);
	printf("demo_sso_login %s\r\n", cel_httpclient_get_remoteaddr_str(&(http_ctx->http_client)));
    cel_httpcontext_response_write(http_ctx, 200, 0, NULL);
	return CEL_RET_OK;
}

int demp_orgs_users_get(CelHttpContext *http_ctx)
{
    char value[128];
    char *buf;
    size_t size;

    cel_httpcontext_get_param_string(http_ctx, "org_name", value, 128);
    //puts(value);
    cel_httpcontext_get_param_string(http_ctx, "user_name", value, 128);
    cel_httpresponse_resize_send_buffer(&(http_ctx->rsp), 256 * 1024);
    buf = cel_httpresponse_get_send_buffer(&(http_ctx->rsp));
    size = cel_httpresponse_get_send_buffer_size(&(http_ctx->rsp));
    memset(buf, 'x', size);
    cel_httpresponse_seek_send_buffer(&(http_ctx->rsp), size);
    //printf("size = %d %d\r\n", size, 256 * 1024);
    //puts(value);
    cel_httpcontext_response_write(http_ctx, 200, 0, NULL);
	return CEL_RET_OK;
}
