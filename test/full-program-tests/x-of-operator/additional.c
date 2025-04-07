#ifdef _MSC_VER
#  define EXPORT_SYMBOL __declspec(dllexport)
#else
#  define EXPORT_SYMBOL
#endif

#include <stddef.h>
#include <stdint.h>

int32_t FFI_Func1(int32_t x, int32_t y)
{
  return x + y;
}
