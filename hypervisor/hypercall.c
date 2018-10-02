#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hypercall.h"

static int hp_handle_open(VM*);
static int hp_handle_read(VM*);
static int hp_handle_write(VM*);
static int hp_handle_close(VM*);
static int hp_handle_lseek(VM*);
static int hp_handle_exit(VM*);
static int hp_handle_panic(VM*);

int hp_handler(uint16_t nr, VM* vm) {
  switch(nr) {
#define handle(f) case NR_HP_##f: return hp_handle_##f(vm)

  handle(open);
  handle(read);
  handle(write);
  handle(close);
  handle(lseek);
  handle(exit);
  handle(panic);

#undef handle
  default:
    return -ENOSYS;
  }
}

typedef struct fd_handle {
  int real_fd;
  int opening;
} fd_handle;

#define MAX_FD 255

static fd_handle fd_map[MAX_FD + 1];

static inline void MAY_INIT_FD_MAP() {
  static int fd_map_init = 0;
  if(!fd_map_init) {
    fd_map_init = 1;
    fd_map[0].real_fd = 0; fd_map[0].opening = 1;
    fd_map[1].real_fd = 1; fd_map[1].opening = 1;
    fd_map[2].real_fd = 2; fd_map[2].opening = 1;
    for(int i = 3; i <= MAX_FD; i++)
      fd_map[i].opening = 0;
  }
}

#define UNUSED_VAR 0xdeadffffu

#define FETCH_U32 (*(uint32_t*)((uint8_t*)vm->run + vm->run->io.data_offset))

#define PROCESS if(vm->run->io.direction == KVM_EXIT_IO_OUT)
#define THEN_RETURN(var) else { \
    if(var == UNUSED_VAR) return -1; \
    FETCH_U32 = var; \
    var = UNUSED_VAR; \
  }

#define CHECK_OOB(var) assert(0 <= (var) && (var) < vm->mem_size)

#define MEM_AT(offset) ( \
    CHECK_OOB(offset), \
    (uint8_t*) vm->mem + (uint64_t) offset \
  )

static int hp_handle_open(VM *vm) {
  static int ret = UNUSED_VAR;
  PROCESS {
    uint32_t offset = FETCH_U32;
    const char *filename = (char*) MEM_AT(offset);
    uint32_t end = offset + strlen(filename);
    CHECK_OOB(end);

    MAY_INIT_FD_MAP();
    int min_fd;
    for(min_fd = 0; min_fd <= MAX_FD; min_fd++)
      if(fd_map[min_fd].opening == 0) break;
    if(min_fd > MAX_FD) ret = -ENFILE;
    else {
      int fd = open(filename, O_RDONLY, 0);
      if(fd < 0) ret = -errno;
      else {
        fd_map[min_fd].real_fd = fd;
        fd_map[min_fd].opening = 1;
        ret = min_fd;
      }
    }
  } THEN_RETURN(ret);
  return 0;
}

#define BADFD(fd) (fd < 0 || fd > MAX_FD || fd_map[fd].opening == 0)
#define PROCESS_ON_FD(work) do { \
    MAY_INIT_FD_MAP(); \
    if(BADFD(fd)) ret = -EBADF; \
    else { \
      ret = work; \
      if(ret < 0) ret = -errno; \
    } \
  } while(0)

static int handle_rw(VM* vm, typeof(read) fptr) {
  static int ret = UNUSED_VAR;
  PROCESS {
    uint32_t offset = FETCH_U32;
    const uint64_t *kbuf = (uint64_t*) MEM_AT(offset);
    int fd = (int) kbuf[0];
    uint64_t paddr = kbuf[1];
    uint64_t nbytes = kbuf[2];

    PROCESS_ON_FD(fptr(fd_map[fd].real_fd, MEM_AT(paddr), nbytes));

  } THEN_RETURN(ret);
  return 0;
}

static int hp_handle_read(VM* vm) {
  return handle_rw(vm, read);
}

static int hp_handle_write(VM* vm) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
  /* Justification: here just cast write into typeof(read) */
  return handle_rw(vm, write);
#pragma GCC diagnostic pop
}

static inline int do_close(struct fd_handle *h) {
  h->opening = 0;
  return close(h->real_fd);
}

static int hp_handle_close(VM *vm) {
  static int ret = UNUSED_VAR;
  PROCESS {
    int fd = FETCH_U32;

    PROCESS_ON_FD(do_close(&fd_map[fd]));

  } THEN_RETURN(ret);
  return 0;
}

static int hp_handle_lseek(VM *vm) {
  static int ret = UNUSED_VAR;
  PROCESS {
    uint32_t offset = FETCH_U32;
    const uint32_t *kbuf = (uint32_t*) MEM_AT(offset);
    int fd = kbuf[0];
    uint32_t off = kbuf[1];
    int whence = kbuf[2];

    PROCESS_ON_FD(lseek(fd_map[fd].real_fd, off, whence));

  } THEN_RETURN(ret);
  return 0;
}
static int hp_handle_exit(VM *vm) {
  int status = FETCH_U32;
  fprintf(stderr, "+++ exited with %d +++\n", status);
  exit(0);
}

static int hp_handle_panic(VM *vm) {
  uint32_t offset = FETCH_U32;
  fprintf(stderr, "[\e[31mPANIC\e[0m] %s\n", MEM_AT(offset));
  exit(1);
  return -1;
}
