#ifndef HYPERCALL_H
#define HYPERCALL_H

#include <stdint.h>

#include <hypercalls/hypercall_table.h>
#include <utils/panic.h>

int hypercall(unsigned short port, uint32_t data);

#endif
