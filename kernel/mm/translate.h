#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <stdint.h>

#include <elf/elf.h>

#define PROT_R 1
#define PROT_W 2
#define PROT_X 4
#define PROT_RW (PROT_R | PROT_W)
#define PROT_RWX (PROT_RW | PROT_X)

/* 64-bit page * entry bits */
#define PDE64_PRESENT 1
#define PDE64_RW (1 << 1)
#define PDE64_USER (1 << 2)
#define PDE64_ACCESSED (1 << 5)
#define PDE64_DIRTY (1 << 6)
#define PDE64_PS (1 << 7)
#define PDE64_G (1 << 8)

#define KERNEL_PAGING_SIZE (0x200000)
#define MIN_MMAP_ADDR KERNEL_PAGING_SIZE
#define KERNEL_BASE_OFFSET (0x8000000000llu)

#define MIN_USER_MEM MIN_MMAP_ADDR

#define USER_MEM_RANGE_OK(v) ((uint64_t)(v) >= MIN_USER_MEM && \
  ((uint64_t)(v) >> 48) == 0 && \
  ((uint64_t)(v) >> 39) != 1)

void init_pagetable();

uint64_t translate(void* vaddr, int usermode, int writable);
uint64_t physical(void *vaddr);
void add_trans_user(void* vaddr, void* paddr, int prot);
int modify_permission(void *vaddr, int prot);
int pf_to_prot(Elf64_Word pf);

#endif
