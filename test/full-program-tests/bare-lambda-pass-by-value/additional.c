#ifdef _MSC_VER
#  define EXPORT_SYMBOL __declspec(dllexport)
#else
#  define EXPORT_SYMBOL
#endif

#include <stdint.h>

#define SMALL_SIZE 2
#define LARGE_SIZE 64

typedef struct
{
  int32_t ar[SMALL_SIZE];
}S1Small;

typedef struct
{
  int32_t ar[LARGE_SIZE];
}S1Large;

EXPORT_SYMBOL void call_via_C_clobber_small(void (*lambda)(S1Small, S1Small), S1Small s1, S1Small s2)
{
  lambda(s1, s2);
}

EXPORT_SYMBOL void call_via_C_clobber_large(void (*lambda)(S1Large, S1Large), S1Large s1, S1Large s2)
{
  lambda(s1, s2);
}

EXPORT_SYMBOL S1Small call_via_C_add_small(S1Small (*lambda)(S1Small, S1Small), S1Small s1, S1Small s2)
{
  return lambda(s1, s2);
}

EXPORT_SYMBOL S1Large call_via_C_add_large(S1Large (*lambda)(S1Large, S1Large), S1Large s1, S1Large s2)
{
  return lambda(s1, s2);
}
