#ifndef __CELDEMO_LOG_H__
#define __CELDEMO_LOG_H__

#include "cel/log.h"

#define Debug(args) CEL_LOG_DEBUG(args)
#define Info(args) CEL_LOG_INFO(args)
#define Notice(args) CEL_LOG_NOTICE(args)
#define Warning(args) CEL_LOG_WARNING(args)
#define Err(args) CEL_LOG_ERR(args)
#define Crit(args) CEL_LOG_CRIT(args)
#define Alert(args) CEL_LOG_ALERT(args)
#define Emerg(args) CEL_LOG_EMERG(args)

#endif
