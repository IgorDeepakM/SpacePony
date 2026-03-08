#ifndef INT_UTILS_H
#define INT_UTILS_H

#define ALIGN_UP(addr, bytes) (((addr) + ((bytes) - 1)) & ((~bytes) + 1))
#define ALIGN_DOWN(addr, bytes) ((addr) & ((~bytes) + 1))
#define IS_ALIGNED(addr, bytes) (!((addr) & (bytes - 1)))

#endif
