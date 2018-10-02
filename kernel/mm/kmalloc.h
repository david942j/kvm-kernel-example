#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdint.h>

void init_allocator(void *addr, uint64_t len);

#define MALLOC_NO_ALIGN 0x0
#define MALLOC_PAGE_ALIGN 0x1000
void *kmalloc(uint64_t len, int align);
void kfree(void *addr);

#endif
