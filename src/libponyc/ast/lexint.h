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

lexint_t lexint_zero();

int lexint_cmp(lexint_t const* a, lexint_t const* b);

int lexint_cmp64(lexint_t const* a, uint64_t b);

lexint_t lexint_shl(lexint_t const* a, uint64_t b);

lexint_t lexint_shr(lexint_t const* a, uint64_t b);

uint64_t lexint_testbit(lexint_t const* a, uint8_t b);

lexint_t lexint_setbit(lexint_t const* a, uint8_t b);

lexint_t lexint_add(lexint_t const* a, lexint_t const* b);

lexint_t lexint_add64(lexint_t const* a, uint64_t b);

lexint_t lexint_sub(lexint_t const* a, lexint_t const* b);

lexint_t lexint_sub64(lexint_t const* a, uint64_t b);

lexint_t lexint_mul(lexint_t const* a, lexint_t const* b);

lexint_t lexint_mul64(lexint_t const* a, uint64_t b);

lexint_t lexint_div(lexint_t const* a, lexint_t const* b);

lexint_t lexint_div64(lexint_t const* a, uint64_t b);

lexint_t lexint_char(lexint_t const *i, int c);

bool lexint_accum(lexint_t* i, uint64_t digit, uint64_t base);

double lexint_double(lexint_t const* i);

lexint_t lexint_and(lexint_t const* a, lexint_t const* b);

lexint_t lexint_and64(lexint_t const* a, uint64_t b);

lexint_t lexint_or(lexint_t const* a, lexint_t const* b);

lexint_t lexint_or64(lexint_t const* a, uint64_t b);

lexint_t lexint_xor(lexint_t const* a, lexint_t const* b);

lexint_t lexint_xor64(lexint_t const* a, uint64_t b);

lexint_t lexint_not(lexint_t const* src);

lexint_t lexint_negate(lexint_t const* src);

bool lexint_is_negative(lexint_t const* v);

PONY_EXTERN_C_END

#endif

