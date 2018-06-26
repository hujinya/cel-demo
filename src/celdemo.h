#ifndef __CELDEMO_H__
#define __CELDEMO_H__

#include "cel/types.h"
#include "cel/log.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/version.h"
#include "cel/eventloopgroup.h"
#include "cel/net/socket.h"

#define Debug(args) CEL_LOG_DEBUG(args)
#define Info(args) CEL_LOG_INFO(args)
#define Notice(args) CEL_LOG_NOTICE(args)
#define Warning(args) CEL_LOG_WARNING(args)
#define Err(args) CEL_LOG_ERR(args)
#define Crit(args) CEL_LOG_CRIT(args)
#define Alert(args) CEL_LOG_ALERT(args)
#define Emerg(args) CEL_LOG_EMERG(args)

#define MAX_FD          1024 * 1024
#define CELDEMO_SERVICE _T("cel-demo")

extern CelVersion celdemo_ver;
extern CelEventLoopGroup evt_loop_grp;

#endif
