#include "warnfix.h"
#include "hwconfig.h"
#include <string.h>
#include <basetype.h>

extern void FCopy(void *dest, const void *src, size_t count);
extern void FFillBytes(void *dest, int value, size_t count);

void * o_memchr(const void *s, int c, size_t n)
{
        return memchr(s, c, n);
}

int o_memcmp(const void *s, const void *s2, size_t n)
{
        return memcmp(s, s2, n);
}

void * o_memmove(void *s1, const void *s2, size_t n)
{
    if ( ( (u_int32) s1 >= (u_int32) s2) && ( ((u_int32)s2 + (u_int32)n) > (u_int32) s1)){
        memmove(s1, s2, n);
    }
    else{
        FCopy(s1, s2, n);
    }
    return s1;
}

void * o_memset(void *s, int c, size_t n)
{

    FFillBytes(s, c, n);
    return(s);
}

/* Simple wrapper round FCopy to keep some modules which still call  */
/* the old API happy (saves a bunch of branching in the source tree) */
void FCopyBytes(void *dest, const void *src, size_t count)
{
  FCopy(dest, src, count);
}

