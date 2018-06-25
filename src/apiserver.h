#ifndef __CELDEMO_APISERVER_H__
#define __CELDEMO_APISERVER_H__

#include "celdemo.h"
#include "cel/net/wmiplistener.h"

extern CelSslContext *wmip_sslctx;
extern CelWmipListener wmip_listener;

int apiserver_listen(CelWmipListener *listener, 
                     const TCHAR *address, CelSslContext *wmip_sslctx);
void apiserver_close(CelWmipListener *listener);

#endif
