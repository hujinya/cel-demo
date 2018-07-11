#ifndef __CELDEMO_APISERVER_H__
#define __CELDEMO_APISERVER_H__

#include "celdemo.h"
#include "cel/net/httpweblistener.h"

extern CelSslContext *api_sslctx;
extern CelHttpWebListener api_listener;

int apiserver_listen(CelHttpWebListener *listener, 
                     const TCHAR *address, CelSslContext *api_sslctx);
void apiserver_close(CelHttpWebListener *listener);

#endif
