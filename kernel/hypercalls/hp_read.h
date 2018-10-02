#ifndef HP_READ_H
#define HP_READ_H

#include <stdint.h>

#include <hypercalls/hypercall.h>

int hp_read(int fildes, uint64_t phy_addr, uint64_t nbyte);

#endif
