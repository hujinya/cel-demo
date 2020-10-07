#ifndef __CELDEMO_H__
#define __CELDEMO_H__

#include "cel/types.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/version.h"
#include "cel/eventloopgroup.h"
#include "cel/net/socket.h"

#define MAX_FD          1024 * 1024
#define SERVICE_NAME _T("cel-demo")
#define SERVICE_CONF _T("../data/etc/cel-demo.conf")

extern CelVersion ver;
extern CelEventLoopGroup evt_loop_grp;

#endif
