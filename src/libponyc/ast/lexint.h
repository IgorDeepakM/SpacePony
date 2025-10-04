#ifndef AST_LEXINT_H
#define AST_LEXINT_H

#include <platform.h>

PONY_EXTERN_C_BEGIN

typedef struct lexint_t
{
  uint64_t low;
  uint64_t high;

  // is_negative is used to determine whether a
  // lexint represents a negative value in the
  // evaluation of compile time literals.
  bool is_negative;
} lexint_t;

void lexint_zero(lexint_t* i);

int lexint_cmp(lexint_t const* a, lexint_t const* b);

int lexint_cmp64(lexint_t* a, uint64_t b);

void lexint_shl(lexint_t* dst, lexint_t* a, uint64_t b);

void lexint_shr(lexint_t* dst, lexint_t* a, uint64_t b);

uint64_t lexint_testbit(lexint_t* a, uint8_t b);

void lexint_setbit(lexint_t* dst, lexint_t* a, uint8_t b);

void lexint_add(lexint_t* dst, lexint_t* a, lexint_t* b);

void lexint_add64(lexint_t* dst, lexint_t* a, uint64_t b);

void lexint_sub(lexint_t* dst, lexint_t* a, lexint_t* b);

void lexint_sub64(lexint_t* dst, lexint_t* a, uint64_t b);

void lexint_mul(lexint_t* dst, lexint_t* a, lexint_t* b);

void lexint_mul64(lexint_t* dst, lexint_t* a, uint64_t b);

void lexint_div(lexint_t* dst, lexint_t* a, lexint_t* b);

void lexint_div64(lexint_t* dst, lexint_t* a, uint64_t b);

void lexint_char(lexint_t* i, int c);

bool lexint_accum(lexint_t* i, uint64_t digit, uint64_t base);

double lexint_double(lexint_t* i);

void lexint_and(lexint_t* dst, lexint_t* a, lexint_t* b);

void lexint_and64(lexint_t* dst, lexint_t* a, uint64_t b);

void lexint_or(lexint_t* dst, lexint_t* a, lexint_t* b);

void lexint_or64(lexint_t* dst, lexint_t* a, uint64_t b);

void lexint_xor(lexint_t* dst, lexint_t* a, lexint_t* b);

void lexint_xor64(lexint_t* dst, lexint_t* a, uint64_t b);

void lexint_not(lexint_t* dst, lexint_t* src);

void lexint_negate(lexint_t* dst, lexint_t* src);

bool lexint_is_negative(lexint_t const* v);

PONY_EXTERN_C_END

#endif

