#ifndef DEFINITION_H
#define DEFINITION_H

#include <stdint.h>

typedef struct VM {
  void *mem;
  uint64_t mem_size;
  /* Only supports one vCPU */
  int vcpufd;
  struct kvm_run *run;
} VM;

/* Common macros */
#define error(fmt, ...) do { \
  fprintf(stderr, fmt, ##__VA_ARGS__); \
  exit(EXIT_FAILURE); \
} while(0)

#define pexit(x) do { \
  perror(x); \
  exit(EXIT_FAILURE); \
} while(0)

/* CR0 bits */
#define CR0_PE 1u
#define CR0_MP (1u << 1)
#define CR0_EM (1u << 2)
#define CR0_TS (1u << 3)
#define CR0_ET (1u << 4)
#define CR0_NE (1u << 5)
#define CR0_WP (1u << 16)
#define CR0_AM (1u << 18)
#define CR0_NW (1u << 29)
#define CR0_CD (1u << 30)
#define CR0_PG (1u << 31)

/* CR4 bits */
#define CR4_VME 1u
#define CR4_PVI (1u << 1)
#define CR4_TSD (1u << 2)
#define CR4_DE (1u << 3)
#define CR4_PSE (1u << 4)
#define CR4_PAE (1u << 5)
#define CR4_MCE (1u << 6)
#define CR4_PGE (1u << 7)
#define CR4_PCE (1u << 8)
#define CR4_OSFXSR (1u << 9)
#define CR4_OSXMMEXCPT (1u << 10)
#define CR4_UMIP (1u << 11)
#define CR4_VMXE (1u << 13)
#define CR4_SMXE (1u << 14)
#define CR4_FSGSBASE (1u << 16)
#define CR4_PCIDE (1u << 17)
#define CR4_OSXSAVE (1u << 18)
#define CR4_SMEP (1u << 20)
#define CR4_SMAP (1u << 21)
#define CR4_PKE (1u << 22)

#define EFER_SCE 1
#define EFER_LME (1 << 8)
#define EFER_LMA (1 << 10)
#define EFER_NXE (1 << 11)
#define EFER_SVME (1 << 12)
#define EFER_LMSLE (1 << 13)
#define EFER_FFXSR (1 << 14)
#define EFER_TCE (1 << 15)

/* 64-bit page * entry bits */
#define PDE64_PRESENT 1
#define PDE64_RW (1 << 1)
#define PDE64_USER (1 << 2)
#define PDE64_ACCESSED (1 << 5)
#define PDE64_DIRTY (1 << 6)
#define PDE64_PS (1 << 7)
#define PDE64_G (1 << 8)

#endif
