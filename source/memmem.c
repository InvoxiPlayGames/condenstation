#include <string.h>
#include "memmem.h"

void *own_memmem(const void *haystack, size_t haystack_len, const void *needle, const size_t needle_len)
{
    // basic sanity checks on input
    if (haystack == NULL || haystack_len == 0)
        return NULL;
    if (needle == NULL || needle_len == 0)
        return NULL;
    // memcompare evrything to see if we've got a match. slow but functional
    for (const char *h = haystack; haystack_len >= needle_len; ++h, --haystack_len)
        if (!memcmp(h, needle, needle_len))
            return (void *)h;
    // noep
    return NULL;
}
