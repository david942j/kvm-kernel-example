#include <hypercalls/hp_close.h>

int hp_close(int fildes) {
  return hypercall(NR_HP_close, fildes);
}
