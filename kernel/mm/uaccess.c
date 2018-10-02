#include <mm/kmalloc.h>
#include <mm/translate.h>
#include <mm/uaccess.h>
#include <utils/misc.h>
#include <utils/string.h>

int access_ok(int type, const void* addr_, uint64_t size) {
  uint64_t addr = (uint64_t) addr_;
  if(!USER_MEM_RANGE_OK(addr)) return 0;
  if(!USER_MEM_RANGE_OK(addr + size - 1)) return 0;
  for(uint64_t v = aligndown(addr); v < alignup(addr + size); v += 0x1000)
    if(translate((void*) v, 1, type) == (uint64_t) -1) return 0;
  return 1;
}

/* check if addr ~ addr+strlen(addr) are all accessible */
int access_string_ok(const void *addr_) {
  if(!access_ok(VERIFY_READ, addr_, 1)) return 0;
  uint64_t addr = (uint64_t) addr_;
  uint64_t remain_size = 0x1000 - (addr & 0xfff);
  /* we have checked the whole page of addr is accessible */
  uint64_t l = strnlen(addr_, remain_size);
  /* length not enough.. recursive it */
  if(l == remain_size) return access_string_ok((void*) (addr + l));
  return 1;
}

void *copy_str_from_user(const char *s) {
  int len = strlen(s);
  void *dst = kmalloc(len + 1, MALLOC_NO_ALIGN);
  if(dst == 0) return 0;
  memcpy(dst, s, len + 1);
  return dst;
}
