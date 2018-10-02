#ifndef HP_WRITE_H
#define HP_WRITE_H

#include <stdint.h>

#include <hypercalls/hypercall.h>

int hp_write(int fildes, uint64_t phy_addr, uint64_t nbyte);

#endif
