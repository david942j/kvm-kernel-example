#ifndef MISC_H
#define MISC_H

#define aligndown(v) ((uint64_t) (v) & -0x1000)
#define alignup(v) (((uint64_t) (v) & 0xfff) ? aligndown(v) + 0x1000 : (uint64_t) (v))
#define alignok(v) ((uint64_t) (v) == aligndown(v))

#endif

