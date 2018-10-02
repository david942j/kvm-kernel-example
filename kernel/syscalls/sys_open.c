#include <hypercalls/hp_open.h>
#include <mm/kmalloc.h>
#include <mm/translate.h>
#include <mm/uaccess.h>
#include <syscalls/sys_open.h>
#include <utils/errno.h>

int sys_open(const char *path) {
  if(!access_string_ok(path)) return -EFAULT;
  void *dst = copy_str_from_user(path);
  if(dst == 0) return -ENOMEM;
  int fd = hp_open(physical(dst));
  kfree(dst);
  return fd;
}
