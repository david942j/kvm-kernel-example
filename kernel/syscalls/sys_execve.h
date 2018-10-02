#ifndef SYS_EXECVE_H
#define SYS_EXECVE_H

#include <stdint.h>

typedef struct process {
  uint64_t load_addr;
  uint64_t entry;
  uint64_t stack_base;
  uint64_t stack_size;
  uint64_t rsp;
} process;

int sys_execve(const char *path, char *const argv[], char *const envp[]);

#endif
