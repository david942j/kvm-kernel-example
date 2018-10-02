#include <hypercalls/hypercall.h>

int hypercall(uint16_t port, uint32_t data) {
  int ret = 0;
  asm(
    "mov dx, %[port];"
    "mov eax, %[data];"
    "out dx, eax;"
    "in eax, dx;"
    "mov %[ret], eax;"
    : [ret] "=r"(ret)
    : [port] "r"(port), [data] "r"(data)
    : "rax", "rdx"
    );
  return ret;
}
