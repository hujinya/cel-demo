#ifndef __CELDEMO_H__
#define __CELDEMO_H__

#include "cel/types.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/version.h"
#include "cel/eventloopgroup.h"
#include "cel/net/socket.h"

#define MAX_FD          1024 * 1024
#define CELDEMO_SERVICE _T("cel-demo")

extern CelVersion celdemo_ver;
extern CelEventLoopGroup evt_loop_grp;

#endif
