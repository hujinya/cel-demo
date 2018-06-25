#ifndef __CELDEMO_CONNECTION_H__
#define __CELDEMO_CONNECTION_H__

#include "celdemo.h"
#include "cel/refcounted.h"

typedef enum _ConnectionType
{
    API_SERVER,
    API_CLIENT
}ConnectionType;

typedef enum _ConnectionState
{
    CONNECTION_INITING = 0,
    CONNECTION_CONNECTING = 1,
    CONNECTION_CONNECTED = 2,
    CONNECTION_SHUTDOWN = 3,
    CONNECTION_CLOSED = 4
}ConnectionState;

typedef struct _Connection
{
    ConnectionType type;
    ConnectionState state;
    CelRefCounted ref_counted;
}Connection;

void *connection_new(int fd, ConnectionType type, CelFreeFunc free_func);
void connection_free(void *data);

void *connection_ref(void *data);
void connection_deref(void *data);

void connection_set_state(void *data, ConnectionState state);
ConnectionState connection_get_state(void *data);

#endif
