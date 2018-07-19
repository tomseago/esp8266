#include <string.h>

#include "butil.h"

int
buf_toi(uint8_t* data, size_t len)
{
    if (!data || !len) return 0;

    char* szTemp = strndup((char *)data, len);
    if (!szTemp) return 0;

    int i = atoi(szTemp);
    free(szTemp);
    return i;
}