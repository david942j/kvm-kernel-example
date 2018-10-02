#include <hypercalls/hp_read.h>
#include <mm/kmalloc.h>
#include <mm/translate.h>
#include <mm/uaccess.h>
#include <syscalls/sys_read.h>
#include <utils/errno.h>
#include <utils/string.h>

int64_t sys_read(int fildes, void *buf, uint64_t nbyte) {
  if(fildes < 0) return -EBADF;
  if(!access_ok(VERIFY_WRITE, buf, nbyte)) return -EFAULT;
  void *dst = kmalloc(nbyte, MALLOC_NO_ALIGN);
  int64_t ret = hp_read(fildes, physical(dst), nbyte);
  if(ret >= 0) memcpy(buf, dst, ret);
  kfree(dst);
  return ret;
}
