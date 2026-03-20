#ifndef INT_UTILS_H
#define INT_UTILS_H

PONY_EXTERN_C_BEGIN

#define ALIGN_UP(addr, bytes) (((addr) + ((bytes) - 1)) & ((~bytes) + 1))
#define ALIGN_DOWN(addr, bytes) ((addr) & ((~bytes) + 1))
#define IS_ALIGNED(addr, bytes) (!((addr) & (bytes - 1)))

bool is_power_of_2(size_t x);

PONY_EXTERN_C_END

#endif
