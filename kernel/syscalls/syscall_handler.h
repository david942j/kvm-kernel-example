#ifndef SYSCALL_HANDLER_H
#define SYSCALL_HANDLER_H

#include <stdint.h>

#include <syscalls/sys_close.h>
#include <syscalls/sys_exit.h>
#include <syscalls/sys_open.h>
#include <syscalls/sys_read.h>
#include <syscalls/sys_write.h>

#define SYS_read 0
#define SYS_write 1
#define SYS_open 2
#define SYS_close 3
#define SYS_exit 60

#define MAX_SYS_NR 60

uint64_t syscall_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2,
  uint64_t arg3, uint64_t arg4, uint64_t arg5);

#endif
