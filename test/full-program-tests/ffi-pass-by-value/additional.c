#ifdef _MSC_VER
#  define EXPORT_SYMBOL __declspec(dllexport)
#else
#  define EXPORT_SYMBOL
#endif

#include <stddef.h>
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

EXPORT_SYMBOL void FFI_clobber_small(S1Small s1, S1Small s2)
{
  for(size_t i = 0; i < SMALL_SIZE; i++)
  {
    s1.ar[i] = 0x1235;
    s2.ar[i] = 0x1235;
  }
}

EXPORT_SYMBOL void FFI_clobber_large(S1Large s1, S1Large s2)
{
  for(size_t i = 0; i < LARGE_SIZE; i++)
  {
    s1.ar[i] = 0x1235;
    s2.ar[i] = 0x1235;
  }
}

EXPORT_SYMBOL S1Small FFI_add_small(S1Small s1, S1Small s2)
{
  S1Small ret;

  for(size_t i = 0; i < SMALL_SIZE; i++)
  {
    ret.ar[i] = s1.ar[i] + s2.ar[i];
  }
  
  return ret;
}

EXPORT_SYMBOL S1Large FFI_add_large(S1Large s1, S1Large s2)
{
  S1Large ret;

  for(size_t i = 0; i < LARGE_SIZE; i++)
  {
    ret.ar[i] = s1.ar[i] + s2.ar[i];
  }

  return ret;	
}

EXPORT_SYMBOL void (*get_FFI_clobber_small())(S1Small, S1Small)
{
  return &FFI_clobber_small;
}

EXPORT_SYMBOL void (*get_FFI_clobber_large())(S1Large, S1Large)
{
  return &FFI_clobber_large;
}

EXPORT_SYMBOL S1Small (*get_FFI_add_small())(S1Small, S1Small)
{
  return &FFI_add_small;
}

EXPORT_SYMBOL S1Large (*get_FFI_add_large())(S1Large, S1Large)
{
  return &FFI_add_large;
}