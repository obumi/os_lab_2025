#include <string.h>
#include "revert_string.h"

void RevertString(char *str)
{
    if (!str) return;
    char *l = str;
    char *r = str + strlen(str);
    if (r == str) return;
    r--;
    while (l < r) {
        char tmp = *l;
        *l = *r;
        *r = tmp;
        l++; r;
    }
}
