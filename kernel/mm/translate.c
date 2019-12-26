#include <mm/kmalloc.h>
#include <mm/translate.h>
#include <utils/panic.h>

/* Maps
 *  0x8000000000 ~ 0x8040000000 -> 0 ~ 0x40000000
 */
void init_pagetable() {
  uint64_t* pml4;
  asm("mov %[pml4], cr3" : [pml4]"=r"(pml4));
  uint64_t* pdp = (uint64_t*) ((uint64_t) pml4 + 0x3000);
  pml4[1] = PDE64_PRESENT | PDE64_RW | (uint64_t) pdp; // 0x8000000000
  uint64_t* pd = (uint64_t*) ((uint64_t) pdp + 0x1000);
  pdp[0] = PDE64_PRESENT | PDE64_RW | (uint64_t) pd;
  for(uint64_t i = 0; i < 0x200; i++)
    pd[i] = PDE64_PRESENT | PDE64_RW | PDE64_PS | (i * KERNEL_PAGING_SIZE);
}

static inline uint64_t* get_pml4_addr() {
  uint64_t pml4;
  asm("mov %[pml4], cr3" : [pml4]"=r"(pml4));
  return (uint64_t*) (pml4 | KERNEL_BASE_OFFSET);
}

#define _OFFSET(v, bits) (((uint64_t)(v) >> (bits)) & 0x1ff)

#define PML4OFF(v) _OFFSET(v, 39)
#define PDPOFF(v) _OFFSET(v, 30)
#define PDOFF(v) _OFFSET(v, 21)
#define PTOFF(v) _OFFSET(v, 12)

/* if vaddr is already mapping to some address, overwrite it. */
void add_trans_user(void* vaddr_, void* paddr_, int prot) {
  uint64_t vaddr = (uint64_t) vaddr_;
  /* validation of vaddr should be done in sys_mmap, so we can simply panic here */
  if(!USER_MEM_RANGE_OK(vaddr)) panic("translate.c#add_trans_user: not allowed");
  uint64_t paddr = (uint64_t) paddr_ & ~KERNEL_BASE_OFFSET;
  uint64_t* pml4 = get_pml4_addr(), *pdp, *pd, *pt;
#define PAGING(p, c) do { \
    if(!(*p & PDE64_PRESENT)) { \
      c = (uint64_t*) kmalloc(0x1000, MALLOC_PAGE_ALIGN); \
      *p = PDE64_PRESENT | PDE64_RW | PDE64_USER | physical(c); \
    } else { \
      if(!(*p & PDE64_USER)) panic("translate.c#add_trans_user: invalid address"); \
      c = (uint64_t*) ((*p & -0x1000) | KERNEL_BASE_OFFSET); \
    } \
  } while(0);
  PAGING(&pml4[PML4OFF(vaddr)], pdp);
  PAGING(&pdp[PDPOFF(vaddr)], pd);
  PAGING(&pd[PDOFF(vaddr)], pt);
#undef PAGING
  pt[PTOFF(vaddr)] = PDE64_PRESENT | paddr;
  if(prot & PROT_R) pt[PTOFF(vaddr)] |= PDE64_USER;
  if(prot & PROT_W) pt[PTOFF(vaddr)] |= PDE64_RW;
}

int modify_permission(void *vaddr, int prot) {
  uint64_t *pml4 = get_pml4_addr(), *pdp, *pd, *pt;
#define PAGING(p, c) do { \
    if(!(*p & PDE64_PRESENT)) return -1; \
    c = (uint64_t*) ((*p & -0x1000) | KERNEL_BASE_OFFSET);\
  } while(0);
  PAGING(&pml4[PML4OFF(vaddr)], pdp);
  PAGING(&pdp[PDPOFF(vaddr)], pd);
  PAGING(&pd[PDOFF(vaddr)], pt);
#undef PAGING
  uint64_t* e = &pt[PTOFF(vaddr)];
  if(!(*e & PDE64_PRESENT)) return -1;
  *e &= ~(PDE64_USER | PDE64_RW);
  if(prot & PROT_R) *e |= PDE64_USER;
  if(prot & PROT_W) *e |= PDE64_RW;
  return 0;
}

/* translate the virtual address to physical address.
 * returns -1 if page not presented or permission not matched
 */
uint64_t translate(void *vaddr, int usermode, int writable) {
  uint64_t *pml4 = get_pml4_addr(), *pdp, *pd, *pt, *ret;
#define PAGING(p, c) do { \
    if(!(*p & PDE64_PRESENT)) return -1; \
    if(usermode && !(*p & PDE64_USER)) return -1; \
    if(writable && !(*p & PDE64_RW)) return -1; \
    c = (uint64_t*) ((*p & -0x1000) | KERNEL_BASE_OFFSET);\
  } while(0);
  PAGING(&pml4[PML4OFF(vaddr)], pdp);
  PAGING(&pdp[PDPOFF(vaddr)], pd);
  PAGING(&pd[PDOFF(vaddr)], pt);
  /* special handles 2MB paging */
  if(pd[PDOFF(vaddr)] & PDE64_PS)
    return (pd[PDOFF(vaddr)] & -0x200000) + ((uint64_t) vaddr & 0x1fffff);
  PAGING(&pt[PTOFF(vaddr)], ret);
#undef PAGING
  return physical(ret) + ((uint64_t) vaddr & 0xfff);
}

/* vaddr should always an address of kernel-space */
uint64_t physical(void *vaddr_) {
  uint64_t vaddr = (uint64_t) vaddr_;
  if(vaddr & KERNEL_BASE_OFFSET) return vaddr ^ KERNEL_BASE_OFFSET;
  panic("translate.c#physical: don't pass non-kernel based address");
}

int pf_to_prot(Elf64_Word pf) {
  int ret = 0;
  if(pf & PF_R) ret |= PROT_R;
  if(pf & PF_W) ret |= PROT_W;
  if(pf & PF_X) ret |= PROT_X;
  return ret;
}
