#include "log.h"

int apilogger_handler(CelHttpContext *http_ctx)
{
//puts("apilogger_handler");
	char uri[256];
	int size = 0;

	size += snprintf(uri, 256 - size, "%s", cel_vstring_str_a(&(http_ctx->req.url.path)));
	if (http_ctx->req.url.fragment.size > 0)
	{
		size += snprintf(uri + size, 256 - size, "#%s", 
			cel_vstring_str_a(&(http_ctx->req.url.fragment)));
	}
	if (http_ctx->req.url.query.size > 0)
	{
		size += snprintf(uri + size, 256 - size, "?%s", 
			cel_vstring_str_a(&(http_ctx->req.url.query)));
	}

	if ((cel_httpresponse_get_statuscode(&(http_ctx->rsp)) / 400) >= 1)
	{
		Err((_T("\"%ldms\" \"%s\" - %s \"%s %s %s\" %d %d \"%s\" \"%s\"(%s)"),
			cel_httpcontext_request_time(http_ctx),
			cel_httpclient_get_remoteaddr_str(&(http_ctx->http_client)),
			http_ctx->user,
			cel_httprequest_get_method_str(&(http_ctx->req)),
			uri,
			cel_httprequest_get_version_str(&(http_ctx->req)),
			cel_httpresponse_get_statuscode(&(http_ctx->rsp)),
			cel_httpresponse_get_send_body_size(&(http_ctx->rsp)),
			cel_httprequest_get_header(&(http_ctx->req), CEL_HTTPHDR_USER_AGENT),
			cel_httprequest_get_header(&(http_ctx->req), CEL_HTTPHDR_X_FORWARDED_FOR),
			cel_httpcontxt_get_errstr(http_ctx)));
	}
	else
	{
		Info((_T("\"%ldms\" \"%s\" - %s \"%s %s %s\" %d %d \"%s\" \"%s\""),
			cel_httpcontext_request_time(http_ctx),
			cel_httpclient_get_remoteaddr_str(&(http_ctx->http_client)),
			http_ctx->user,
			cel_httprequest_get_method_str(&(http_ctx->req)),
			uri,
			cel_httprequest_get_version_str(&(http_ctx->req)),
			cel_httpresponse_get_statuscode(&(http_ctx->rsp)),
			cel_httpresponse_get_send_body_size(&(http_ctx->rsp)),
			cel_httprequest_get_header(&(http_ctx->req), CEL_HTTPHDR_USER_AGENT),
			cel_httprequest_get_header(&(http_ctx->req), CEL_HTTPHDR_X_FORWARDED_FOR)));
	}

	return CEL_RET_OK;
}