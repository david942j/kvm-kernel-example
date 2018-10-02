#include <mm/translate.h>
#include <syscalls/syscall_handler.h>
#include <utils/errno.h>

static const void* syscall_table[MAX_SYS_NR + 1] = {
#define ENTRY(f) [SYS_##f]=sys_##f

  ENTRY(read),
  ENTRY(write),
  ENTRY(open),
  ENTRY(close),
  ENTRY(exit),

#undef ENTRY
};

uint64_t syscall_handler(
  uint64_t arg0, uint64_t arg1, uint64_t arg2,
  uint64_t arg3, uint64_t arg4, uint64_t arg5) {

  uint32_t nr;
  asm("mov %[nr], eax;"
    : [nr] "=r"(nr)
    );
  if(nr > MAX_SYS_NR || syscall_table[nr] == 0) return -ENOSYS;
  void *fptr = (void*) ((uint64_t) syscall_table[nr] | KERNEL_BASE_OFFSET);
  return ((uint64_t(*)(
        uint64_t, uint64_t, uint64_t,
        uint64_t, uint64_t, uint64_t)) fptr)(
    arg0, arg1, arg2,
    arg3, arg4, arg5
    );
}
