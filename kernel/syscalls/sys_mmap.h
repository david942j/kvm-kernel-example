#ifndef SYS_MMAP_H
#define SYS_MMAP_H

#include <stdint.h>

#define MAP_FIXED 0x10

void *sys_mmap(
  void *addr, uint64_t len, int prot,
  int flags, int fd, uint64_t offset);

#endif
