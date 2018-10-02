#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG

#include <assert.h>
#include <linux/kvm.h>
#include <stdio.h>
#include <sys/ioctl.h>

#define debug(...) fprintf(stderr, __VA_ARGS__)
#define dump_seg(n, s) \
  debug("%3s base=0x%016llx limit=%08x selector=%#04x " \
    "type=0x%02x dpl=%d db=%d l=%d g=%d avl=%d\n", \
    (n), (s)->base, (s)->limit, (s)->selector, \
    (s)->type, (s)->dpl, (s)->db, (s)->l, (s)->g, (s)->avl)

void dump_regs(int vcpufd) {
  struct kvm_regs regs;
  int ret = ioctl(vcpufd, KVM_GET_REGS, &regs);
  assert(ret == 0);

  debug("\nDump regs\n");
  debug("rax\t0x%016llx rbx\t0x%016llx rcx\t0x%016llx rdx\t0x%016llx\n",
    regs.rax, regs.rbx, regs.rcx, regs.rdx);
  debug("rsp\t0x%016llx rbp\t0x%016llx rsi\t0x%016llx rdi\t0x%016llx\n",
    regs.rsp, regs.rbp, regs.rsi, regs.rdi);
  debug("rip\t0x%016llx r8\t0x%016llx r9\t0x%016llx r10\t0x%016llx\n",
    regs.rip, regs.r8, regs.r9, regs.r10);
  debug("r11\t0x%016llx r12\t0x%016llx r13\t0x%016llx r14\t0x%016llx\n",
    regs.r11, regs.r12, regs.r13, regs.r14);
  debug("r15\t0x%016llx rflags\t0x%08llx\n", regs.r15, regs.rflags);

  struct kvm_sregs sregs;
  ret = ioctl(vcpufd, KVM_GET_SREGS, &sregs);
  assert(ret == 0);

  dump_seg("cs", &sregs.cs);
  dump_seg("ds", &sregs.ds);
  dump_seg("es", &sregs.es);
  dump_seg("ss", &sregs.ss);
  dump_seg("fs", &sregs.fs);
  dump_seg("gs", &sregs.gs);

  debug("cr0\t0x%016llx\n", sregs.cr0);
  debug("cr3\t0x%016llx\n", sregs.cr3);
}

#else

#define dump_regs(...)

#endif /* DEBUG */

#endif /* DEBUG_H */
