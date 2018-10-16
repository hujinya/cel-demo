#ifndef __CELDEMO_CONF_H__
#define __CELDEMO_CONF_H__

#include "celdemo.h"

typedef struct _Conf
{
    struct {
        TCHAR address[CEL_ADDRLEN];
        struct {
            int on;
            TCHAR protocols[32];
            TCHAR cert[CEL_PATHLEN];
            TCHAR key[CEL_PATHLEN];
            TCHAR key_pswd[CEL_PWLEN];
            TCHAR ciphers[64];
        }ssl;
    }api_server;
    struct {
        int level;
        TCHAR path[CEL_PATHLEN];
    }log;
}Conf;
extern Conf conf;

int conf_read(Conf *conf, const TCHAR *file);
int conf_write(Conf *conf, const TCHAR *file);

#endif
