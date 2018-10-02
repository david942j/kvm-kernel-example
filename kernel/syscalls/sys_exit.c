#include <hypercalls/hypercall.h>
#include <syscalls/sys_exit.h>

void sys_exit(int status) {
  hypercall(NR_HP_exit, status);
}
