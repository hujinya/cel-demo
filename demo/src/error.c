#include "error.h"

ErrorMessage err_msg[ERR_COUNT/100 + 1][100] = {
    {
        /* �ɹ� */
        { ERR_NO, "\xE6\x88\x90\xE5\x8A\x9F" },
        /* ϵͳ�ڲ����� */
        { ERR_UNDEFINED, "\xE7\xB3\xBB\xE7\xBB\x9F\xE5\x86\x85\xE9\x83\xA8\xE9\x94\x99\xE8\xAF\xAF"},
        { -1, NULL }
    },
    {
        /* �ɹ� */
        { ERR_XX, "\xE6\x88\x90\xE5\x8A\x9F" },
        { -1, NULL }
    },
    {
        /* �ɹ� */
        { ERR_XXX, "\xE6\x88\x90\xE5\x8A\x9F" },
        { -1, NULL }
    }
};
