#include <hypercalls/hp_open.h>
#include <mm/kmalloc.h>
#include <mm/translate.h>
#include <syscalls/sys_execve.h>
#include <utils/string.h>

#define MSR_STAR 0xc0000081 /* legacy mode SYSCALL target */
#define MSR_LSTAR 0xc0000082 /* long mode SYSCALL target */
#define MSR_CSTAR 0xc0000083 /* compat mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084

int register_syscall() {
  asm(
    "xor rax, rax;"
    "mov rdx, 0x00200008;"
    "mov ecx, %[msr_star];"
    "wrmsr;"

    "mov eax, %[fmask];"
    "xor rdx, rdx;"
    "mov ecx, %[msr_fmask];"
    "wrmsr;"

    "lea rax, [rip + syscall_entry];"
    "mov rdx, %[base] >> 32;"
    "mov ecx, %[msr_syscall];"
    "wrmsr;"
    :: [msr_star]"i"(MSR_STAR),
       [fmask]"i"(0x3f7fd5), [msr_fmask]"i"(MSR_SYSCALL_MASK),
       [base]"i"(KERNEL_BASE_OFFSET), [msr_syscall]"i"(MSR_LSTAR)
    : "rax", "rdx", "rcx");
  return 0;
}

void switch_user(uint64_t argc, char *argv[]) {
  int total_len = (argv[argc - 1] + strlen(argv[argc - 1]) + 1) - (char*) argv;
  /* temporary area for putting user-accessible data */
  char *s = kmalloc(total_len, MALLOC_PAGE_ALIGN);
  uint64_t sp = physical(s);
  add_trans_user((void*) sp, (void*) sp, PROT_RW); /* sp is page aligned */

  /* copy strings and argv onto user-accessible area */
  for(int i = 0; i < argc; i++)
    argv[i] = (char*) (argv[i] - (char*) argv + sp);
  memcpy(s, argv, total_len);
  sys_execve(argv[0], (char**) sp, (char**) (sp + argc * sizeof(char*)));
}

int kernel_main(void* addr, uint64_t len, uint64_t argc, char *argv[]) {
  init_pagetable();
  /* new paging enabled! */
  init_allocator((void*) ((uint64_t) addr | KERNEL_BASE_OFFSET), len);
  if(register_syscall() != 0) return 1;
  switch_user(argc, argv);
  return 0;
}
