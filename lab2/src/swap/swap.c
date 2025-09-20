#include "swap.h"

void Swap(char *left, char *right)
{
 char tmp = *right;
    *right = *left;
    *left = tmp;
}
