#ifndef MMAP_H
#define MMAP_H

#include <stdint.h>

void *mmap(void *addr, uint64_t len, int prot);
int mprotect(void *addr, uint64_t len, int prot);

#endif
