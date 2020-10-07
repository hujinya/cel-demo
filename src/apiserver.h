#ifndef __CELDEMO_APISERVER_H__
#define __CELDEMO_APISERVER_H__

#include "celdemo.h"
#include "cel/net/httpserve.h"

extern CelSslContext *api_sslctx;
extern CelHttpServe api_listener;

int apiserver_listen(CelHttpServe *listener, 
                     const TCHAR *address, CelSslContext *api_sslctx);
void apiserver_close(CelHttpServe *listener);

#endif
