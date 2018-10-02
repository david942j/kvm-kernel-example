#include <hypercalls/hp_lseek.h>
#include <hypercalls/hp_read.h>
#include <mm/mmap.h>
#include <mm/translate.h>
#include <syscalls/sys_mmap.h>
#include <utils/errno.h>
#include <utils/misc.h>

void *sys_mmap(
  void *addr, uint64_t len, int prot,
  int flags, int fd, uint64_t offset) {

  if(!alignok(addr)) return (void*) -EINVAL;
  if(len == 0) return (void*) -EINVAL;

  if(!(flags & MAP_FIXED)) addr = 0; // no MAP_FIXED, address decided by kernel
  else if(addr != 0 && !USER_MEM_RANGE_OK(addr)) return (void*) -EINVAL;

  uint64_t aligned_len = alignup(len);
  addr = mmap(addr, aligned_len, prot | PROT_RW); // temporary mark it read/writable
  if(addr == 0) return (void*) -ENOMEM;

  if(fd >= 0) {
    int ret = hp_lseek(fd, offset, SEEK_SET);
    if(ret < 0) return (void*) (int64_t) ret;
    hp_read(fd, translate(addr, 1, 1), len);
  }

  /* this should never fail */
  mprotect(addr, aligned_len, prot); // correct protection

  return addr;
}
