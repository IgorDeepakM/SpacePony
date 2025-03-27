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

#define TEST_1_MEMBER_STRUCT(T1)\
  typedef struct\
  {\
    T1 a;\
  }S_##T1;\
\
  S_##T1 FFI_Test_1_##T1(S_##T1 s1, S_##T1 s2)\
  {\
    S_##T1 s1_ret;\
    s1_ret.a = s1.a + s2.a;\
\
    s1.a = 0;\
    s2.a = 0;\
\
    return s1_ret;\
}\

#define TEST_2_MEMBER_STRUCT(T1, T2)\
  typedef struct\
  {\
    T1 a;\
    T2 b;\
  }S_##T1##_##T2;\
\
  S_##T1##_##T2 FFI_Test_2_##T1##_##T2(S_##T1##_##T2 s1, S_##T1##_##T2 s2)\
  {\
    S_##T1##_##T2 s1_ret;\
    s1_ret.a = s1.a + s2.a;\
    s1_ret.b = s1.b + s2.b;\
\
    s1.a = 0;\
    s1.b = 0;\
    s2.a = 0;\
    s2.b = 0;\
\
    return s1_ret;\
}\

#define TEST_3_MEMBER_STRUCT(T1, T2, T3)\
  typedef struct\
  {\
    T1 a;\
    T2 b;\
    T3 c;\
  }S_##T1##_##T2##_##T3;\
\
  S_##T1##_##T2##_##T3 FFI_Test_3_##T1##_##T2##_##T3(S_##T1##_##T2##_##T3 s1, S_##T1##_##T2##_##T3 s2)\
  {\
    S_##T1##_##T2##_##T3 s1_ret;\
    s1_ret.a = s1.a + s2.a;\
    s1_ret.b = s1.b + s2.b;\
    s1_ret.c = s1.c + s2.c;\
\
    s1.a = 0;\
    s1.b = 0;\
    s1.c = 0;\
    s2.a = 0;\
    s2.b = 0;\
    s2.c = 0;\
\
    return s1_ret;\
}\

#define TEST_4_MEMBER_STRUCT(T1, T2, T3, T4)\
  typedef struct\
  {\
    T1 a;\
    T2 b;\
    T3 c;\
    T4 d;\
  }S_##T1##_##T2##_##T3##_##T4;\
\
  S_##T1##_##T2##_##T3##_##T4 FFI_Test_4_##T1##_##T2##_##T3##_##T4(S_##T1##_##T2##_##T3##_##T4 s1, S_##T1##_##T2##_##T3##_##T4 s2)\
  {\
    S_##T1##_##T2##_##T3##_##T4 s1_ret;\
    s1_ret.a = s1.a + s2.a;\
    s1_ret.b = s1.b + s2.b;\
    s1_ret.c = s1.c + s2.c;\
    s1_ret.d = s1.c + s2.d;\
\
    s1.a = 0;\
    s1.b = 0;\
    s1.c = 0;\
    s1.d = 0;\
    s2.a = 0;\
    s2.b = 0;\
    s2.c = 0;\
    s2.d = 0;\
\
    return s1_ret;\
}\

TEST_1_MEMBER_STRUCT(int8_t)
TEST_1_MEMBER_STRUCT(int16_t)
TEST_1_MEMBER_STRUCT(int32_t)
TEST_1_MEMBER_STRUCT(int64_t)
TEST_1_MEMBER_STRUCT(float)
TEST_1_MEMBER_STRUCT(double)

TEST_2_MEMBER_STRUCT(int8_t, int8_t)
TEST_2_MEMBER_STRUCT(int8_t, int32_t)
TEST_2_MEMBER_STRUCT(int32_t, float)
TEST_2_MEMBER_STRUCT(int64_t, double)
TEST_2_MEMBER_STRUCT(float, float)
TEST_2_MEMBER_STRUCT(double, double)

TEST_3_MEMBER_STRUCT(int32_t, int32_t, double)
TEST_3_MEMBER_STRUCT(float, float, int64_t)
TEST_3_MEMBER_STRUCT(int32_t, float, int64_t)
TEST_3_MEMBER_STRUCT(double, int32_t, int32_t)
TEST_3_MEMBER_STRUCT(float, float, float)
TEST_3_MEMBER_STRUCT(double, double, double)

TEST_4_MEMBER_STRUCT(int32_t, int32_t, int32_t, int32_t)
TEST_4_MEMBER_STRUCT(int64_t, int64_t, int64_t, int64_t)
TEST_4_MEMBER_STRUCT(float, float, float, float)
TEST_4_MEMBER_STRUCT(double, double, double, double)
