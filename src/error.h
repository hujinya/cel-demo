#ifndef __CELDEMO_ERROR_H__
#define __CELDEMO_ERROR_H__

#include "cel/types.h"
#include "cel/error.h"

typedef enum _ErrorNo
{
    ERR_NO = 0,
    ERR_UNDEFINED = 1,

    ERR_XX = 100,

    ERR_XXX= 200,

    ERR_COUNT
}ErrorNo;

typedef struct _ErrorMessage
{
    int err_no;
    const char *msg;
}ErrorMessage;

extern ErrorMessage err_msg[ERR_COUNT / 100 + 1][100];

#define deamo_seterrno(error_no) cel_seterrno((error_no)|CEL_ERR_USER)
#define deamo_geterrno() (cel_geterrno() & ~CEL_ERR_USER)

#define DEAMO_SETERRSTR(args) cel_seterrstr args
#define DEAMO_SETERR(args) cel_seterr args

#define deamo_geterrmsg(error_no) \
    (err_msg[(error_no) / 100][(error_no) % 100].msg)
#define deamo_checkerrmsg(error_no) \
    (err_msg[(error_no) / 100][(error_no) % 100].err_no == error_no)
#define deamo_geterrstr(error_no) cel_geterrstr((error_no)|CEL_ERR_USER)

#endif
