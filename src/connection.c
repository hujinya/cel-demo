#include "connection.h"
#include "error.h"
#include "apiserver.h"

typedef union 
{
    CelWmipListener wmip_srv;
    CelWmipClient wmip_clt;
}ConnectionData;

Connection *fds[MAX_FD] = { NULL };

void *connection_new(int fd, ConnectionType type, CelFreeFunc free_func)
{
    if (fd < 0 
        || fd > MAX_FD - 1
        || (fds[fd] == NULL
        && (fds[fd] = 
        (Connection *)cel_malloc(sizeof(Connection) + sizeof(ConnectionData))) == NULL))
    {
        DEAMO_SETERR((0, _T("Connection new failed.")));
        return NULL;
    }
    fds[fd]->type = type;
    fds[fd]->state = CONNECTION_INITING;
    cel_refcounted_init(&(fds[fd]->ref_counted), free_func);

    return (fds[fd] + 1);
}

void connection_free(void *data)
{
    Connection *con;

    con = (Connection *)((char *)data - sizeof(Connection));
    con->state = CONNECTION_SHUTDOWN;
    cel_refcounted_destroy(&(con->ref_counted), data);
    //Debug((_T("##free %d ref cnt %ld"), con->type, con->ref_counted.cnt));
}

void *connection_ref(void *data)
{
    Connection *con;

    con = (Connection *)((char *)data - sizeof(Connection));
    //Debug((_T("ref %d ref cnt %ld"), con->type, con->ref_counted.cnt));
    return cel_refcounted_ref_ptr(&(con->ref_counted), data);
}

void connection_deref(void *data)
{
    Connection *con;

    con = (Connection *)((char *)data - sizeof(Connection));
    cel_refcounted_deref(&(con->ref_counted), data);
    //Debug((_T("##unref %d ref cnt %ld"), con->type, con->ref_counted.cnt));
}

void connection_set_state(void *data, ConnectionState state)
{
    Connection *con;

    con = (Connection *)((char *)data - sizeof(Connection));
    con->state = state;
}

ConnectionState connection_get_state(void *data)
{
    Connection *con;

    con = (Connection *)((char *)data - sizeof(Connection));
    return con->state;
}
