#ifndef HP_LSEEK_H
#define HP_LSEEK_H

#include <hypercalls/hypercall.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int hp_lseek(int fildes, uint32_t offset, int whence);

#endif
