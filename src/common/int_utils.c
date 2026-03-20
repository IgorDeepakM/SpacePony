#include <platform.h>
#include "int_utils.h"


bool is_power_of_2(size_t x)
{
  return x > 0 && !(x & (x - 1));
}
