#ifndef STRING_H
#define STRING_H

#include <stdint.h>

uint64_t strlen(const char *s);
uint64_t strnlen(const char *s, uint64_t maxlen);
void *memset(void *b, int c, uint64_t len);
void *memcpy(void *dst, const void *src, uint64_t n);
int memcmp(const void *s1, const void *s2, uint64_t n);

#endif
