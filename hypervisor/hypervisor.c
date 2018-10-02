#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "debug.h"
#include "definition.h"
#include "hypercall.h"

#define PS_LIMIT (0x200000)
#define KERNEL_STACK_SIZE (0x4000)
#define MAX_KERNEL_SIZE (PS_LIMIT - 0x5000 - KERNEL_STACK_SIZE)
#define MEM_SIZE (PS_LIMIT * 0x2)

void read_file(const char *filename, uint8_t** content_ptr, size_t* size_ptr) {
  FILE *f = fopen(filename, "rb");
  if(f == NULL) error("Open file '%s' failed.\n", filename);
  if(fseek(f, 0, SEEK_END) < 0) pexit("fseek(SEEK_END)");

  size_t size = ftell(f);
  if(size == 0) error("Empty file '%s'.\n", filename);
  if(fseek(f, 0, SEEK_SET) < 0) pexit("fseek(SEEK_SET)");

  uint8_t *content = (uint8_t*) malloc(size);
  if(content == NULL) error("read_file: Cannot allocate memory\n");
  if(fread(content, 1, size, f) != size) error("read_file: Unexpected EOF\n");

  fclose(f);
  *content_ptr = content;
  *size_ptr = size;
}

/* set rip = entry point
 * set rsp = MAX_KERNEL_SIZE + KERNEL_STACK_SIZE (the max address can be used)
 *
 * set rdi = PS_LIMIT (start of free (unpaging) physical pages)
 * set rsi = MEM_SIZE - rdi (total length of free pages)
 * Kernel could use rdi and rsi to initalize its memory allocator.
 */
void setup_regs(VM *vm, size_t entry) {
  struct kvm_regs regs;
  if(ioctl(vm->vcpufd, KVM_GET_REGS, &regs) < 0) pexit("ioctl(KVM_GET_REGS)");
  regs.rip = entry;
  regs.rsp = MAX_KERNEL_SIZE + KERNEL_STACK_SIZE; /* temporary stack */
  regs.rdi = PS_LIMIT; /* start of free pages */
  regs.rsi = MEM_SIZE - regs.rdi; /* total length of free pages */
  regs.rflags = 0x2;
  if(ioctl(vm->vcpufd, KVM_SET_REGS, &regs) < 0) pexit("ioctl(KVM_SET_REGS");
}

/* Maps:
 * 0 ~ 0x200000 -> 0 ~ 0x200000 with kernel privilege
 */
void setup_paging(VM *vm) {
  struct kvm_sregs sregs;
  if(ioctl(vm->vcpufd, KVM_GET_SREGS, &sregs) < 0) pexit("ioctl(KVM_GET_SREGS)");
  uint64_t pml4_addr = MAX_KERNEL_SIZE;
  uint64_t *pml4 = (void*) (vm->mem + pml4_addr);

  uint64_t pdp_addr = pml4_addr + 0x1000;
  uint64_t *pdp = (void*) (vm->mem + pdp_addr);

  uint64_t pd_addr = pdp_addr + 0x1000;
  uint64_t *pd = (void*) (vm->mem + pd_addr);

  pml4[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pdp_addr;
  pdp[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pd_addr;
  pd[0] = PDE64_PRESENT | PDE64_RW | PDE64_PS; /* kernel only, no PED64_USER */

  sregs.cr3 = pml4_addr;
  sregs.cr4 = CR4_PAE;
  sregs.cr4 |= CR4_OSFXSR | CR4_OSXMMEXCPT; /* enable SSE instruction */
  sregs.cr0 = CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM | CR0_PG;
  sregs.efer = EFER_LME | EFER_LMA;
  sregs.efer |= EFER_SCE; /* enable syscall instruction */

  if(ioctl(vm->vcpufd, KVM_SET_SREGS, &sregs) < 0) pexit("ioctl(KVM_SET_SREGS)");
}

void setup_seg_regs(VM *vm) {
  struct kvm_sregs sregs;
  if(ioctl(vm->vcpufd, KVM_GET_SREGS, &sregs) < 0) pexit("ioctl(KVM_GET_SREGS)");
  struct kvm_segment seg = {
    .base = 0,
    .limit = 0xffffffff,
    .selector = 1 << 3,
    .present = 1,
    .type = 0xb, /* Code segment */
    .dpl = 0, /* Kernel: level 0 */
    .db = 0,
    .s = 1,
    .l = 1, /* long mode */
    .g = 1
  };
  sregs.cs = seg;
  seg.type = 0x3; /* Data segment */
  seg.selector = 2 << 3;
  sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss = seg;
  if(ioctl(vm->vcpufd, KVM_SET_SREGS, &sregs) < 0) pexit("ioctl(KVM_SET_SREGS)");
}

/*
 * Switching to long mode usually done by kernel.
 * We put the task in hypervisor because we want our KVM be able to execute
 * normal x86-64 assembled code as well. Which let us easier to debug and test.
 *
 */
void setup_long_mode(VM *vm) {
  setup_paging(vm);
  setup_seg_regs(vm);
}

VM* kvm_init(uint8_t code[], size_t len) {
  int kvmfd = open("/dev/kvm", O_RDWR | O_CLOEXEC);
  if(kvmfd < 0) pexit("open(/dev/kvm)");

  int api_ver = ioctl(kvmfd, KVM_GET_API_VERSION, 0);
	if(api_ver < 0) pexit("KVM_GET_API_VERSION");
  if(api_ver != KVM_API_VERSION) {
    error("Got KVM api version %d, expected %d\n",
      api_ver, KVM_API_VERSION);
  }
  int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0);
  if(vmfd < 0) pexit("ioctl(KVM_CREATE_VM)");
  void *mem = mmap(0,
    MEM_SIZE,
    PROT_READ | PROT_WRITE,
    MAP_SHARED | MAP_ANONYMOUS,
    -1, 0);
  if(mem == NULL) pexit("mmap(MEM_SIZE)");
  size_t entry = 0;
  memcpy((void*) mem + entry, code, len);
  struct kvm_userspace_memory_region region = {
    .slot = 0,
    .flags = 0,
    .guest_phys_addr = 0,
    .memory_size = MEM_SIZE,
    .userspace_addr = (size_t) mem
  };
  if(ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region) < 0) {
    pexit("ioctl(KVM_SET_USER_MEMORY_REGION)");
  }
  int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, 0);
  if(vcpufd < 0) pexit("ioctl(KVM_CREATE_VCPU)");
  size_t vcpu_mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
  struct kvm_run *run = (struct kvm_run*) mmap(0,
    vcpu_mmap_size,
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    vcpufd, 0);

  VM *vm = (VM*) malloc(sizeof(VM));
  *vm = (struct VM){
    .mem = mem,
    .mem_size = MEM_SIZE,
    .vcpufd = vcpufd,
    .run = run
  };

  setup_regs(vm, entry);
  setup_long_mode(vm);

  return vm;
}

void execute(VM* vm) {
  while(1) {
    ioctl(vm->vcpufd, KVM_RUN, NULL);
    dump_regs(vm->vcpufd);
    switch (vm->run->exit_reason) {
    case KVM_EXIT_HLT:
      fprintf(stderr, "KVM_EXIT_HLT\n");
      return;
    case KVM_EXIT_IO:
      if(vm->run->io.port & HP_NR_MARK) {
        if(hp_handler(vm->run->io.port, vm) < 0) error("Hypercall failed\n");
      }
      else error("Unhandled I/O port: 0x%x\n", vm->run->io.port);
      break;
    case KVM_EXIT_FAIL_ENTRY:
      error("KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx\n",
        vm->run->fail_entry.hardware_entry_failure_reason);
    case KVM_EXIT_INTERNAL_ERROR:
      error("KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x\n",
        vm->run->internal.suberror);
    case KVM_EXIT_SHUTDOWN:
      error("KVM_EXIT_SHUTDOWN\n");
    default:
      error("Unhandled reason: %d\n", vm->run->exit_reason);
    }
  }
}

/* copy argv onto kernel's stack */
void copy_argv(VM* vm, int argc, char *argv[]) {
  struct kvm_regs regs;
  if(ioctl(vm->vcpufd, KVM_GET_REGS, &regs) < 0) pexit("ioctl(KVM_GET_REGS)");
  char *sp = (char*)vm->mem + regs.rsp;
  char **copy = (char**) malloc(argc * sizeof(char*));
#define STACK_ALLOC(sp, len) ({ sp -= len; sp; })
  for(int i = argc - 1; i >= 0; i--) {
    int len = strlen(argv[i]) + 1;
    copy[i] = STACK_ALLOC(sp, len);
    memcpy(copy[i], argv[i], len);
  }
  sp = (char*) ((uint64_t) sp & -0x10);
  /* push argv */
  *(uint64_t*) STACK_ALLOC(sp, sizeof(char*)) = 0;
  for(int i = argc - 1; i >= 0; i--)
    *(uint64_t*) STACK_ALLOC(sp, sizeof(char*)) = copy[i] - (char*)vm->mem;
  /* push argc */
  *(uint64_t*) STACK_ALLOC(sp, sizeof(uint64_t)) = argc;
  free(copy);
#undef STACK_ALLOC
  regs.rsp = sp - (char*) vm->mem;
  if(ioctl(vm->vcpufd, KVM_SET_REGS, &regs) < 0) pexit("ioctl(KVM_SET_REGS)");
}

int main(int argc, char *argv[]) {
  if(argc < 3) {
    printf("Usage: %s kernel.bin user.elf [user_args...]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  uint8_t *code;
  size_t len;
  read_file(argv[1], &code, &len);
  if(len > MAX_KERNEL_SIZE)
    error("Kernel size exceeded, %p > MAX_KERNEL_SIZE(%p).\n",
      (void*) len,
      (void*) MAX_KERNEL_SIZE);
  VM* vm = kvm_init(code, len);
  copy_argv(vm, argc - 2, &argv[2]);
  execute(vm);
}
