#include "lexint.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>


lexint_t lexint_zero()
{
  lexint_t ret = { 0, 0, false };

  return ret;
}


lexint_t lexint_minusone()
{
  lexint_t ret = { UINT64_MAX, UINT64_MAX, true };

  return ret;
}


int lexint_cmp(lexint_t const* a, lexint_t const* b)
{
  if(a->is_negative && !b->is_negative)
    return -1;

  if(!a->is_negative && b->is_negative)
    return 1;

  if(a->high > b->high)
    return 1;

  if(a->high < b->high)
    return -1;

  if(a->low > b->low)
    return 1;

  if(a->low < b->low)
    return -1;

  return 0;
}

int lexint_cmp64(lexint_t const* a, uint64_t b)
{
  if(a->is_negative)
    return -1;

  if(a->high > 0)
    return 1;

  if(a->low > b)
    return 1;

  if(a->low < b)
    return -1;

  return 0;
}

lexint_t lexint_shl(lexint_t const* a, uint64_t b)
{
  lexint_t ret = lexint_zero();

  ret.is_negative = a->is_negative;

  if(b >= 128)
  {
    ret.high = 0;
    ret.low = 0;
  } else if(b > 64) {
    ret.high = a->low << (b - 64);
    ret.low = 0;
  } else if(b == 64) {
    ret.high = a->low;
    ret.low = 0;
  } else if(b > 0) {
    ret.high = (a->high << b) + (a->low >> (64 - b));
    ret.low = a->low << b;
  } else {
    ret.high = a->high;
    ret.low = a->low;
  }

  return ret;
}

lexint_t lexint_shr(lexint_t const* a, uint64_t b)
{
  lexint_t ret = lexint_zero();

  ret.is_negative = a->is_negative;

  if(b >= 128)
  {
    ret.high = 0;
    ret.low = 0;
  } else if(b > 64) {
    ret.low = a->high >> (b - 64);
    ret.high = 0;
  } else if(b == 64) {
    ret.low = a->high;
    ret.high = 0;
  } else if(b > 0) {
    ret.low = (a->high << (64 - b)) + (a->low >> b);
    ret.high = a->high >> b;
  } else {
    ret.high = a->high;
    ret.low = a->low;
  }

  // If negative we need to shift down the value from the 129's bit (is_signed)
  if(a->is_negative && b > 0)
  {
    int64_t high_shift = 0x8000000000000000;
    high_shift >>= (b - 1);

    ret.high |= (uint64_t)high_shift;
    if (b > 64)
    {
      int64_t low_shift = 0x8000000000000000;
      low_shift >>= (b - 64 - 1);
      ret.low |= (uint64_t)low_shift;
    }
  }

  return ret;
}

uint64_t lexint_testbit(lexint_t const* a, uint8_t b)
{
  if(b >= 64)
    return (a->high >> (b - 64)) & 1;

  return (a->low >> b) & 1;
}

lexint_t lexint_setbit(lexint_t const* a, uint8_t b)
{
  lexint_t ret = *a;

  if(b >= 64)
    ret.high |= (uint64_t)1 << (b - 64);
  else
    ret.low |= (uint64_t)1 << b;

  return ret;
}

lexint_t lexint_add(lexint_t const* a, lexint_t const* b)
{
  lexint_t ret = lexint_zero();

  if(a->is_negative && !b->is_negative)
  {
    lexint_t t = lexint_negate(a);
    ret.is_negative = lexint_cmp(&t, b) > 0;
  }
  else if(!a->is_negative && b->is_negative)
  {
    lexint_t t = lexint_negate(b);
    ret.is_negative = lexint_cmp(a, &t) < 0;
  }
  else
  {
    ret.is_negative = a->is_negative;
  }

  ret.high = a->high + b->high + ((a->low + b->low) < a->low);
  ret.low = a->low + b->low;

  return ret;
}

lexint_t lexint_add64(lexint_t const* a, uint64_t b)
{
  lexint_t ret = lexint_zero();

  if(a->is_negative)
  {
    lexint_t t = lexint_negate(a);
    ret.is_negative = lexint_cmp64(&t, b) > 0;
  }
  else
  {
    ret.is_negative = false;
  }

  ret.high = a->high + ((a->low + b) < a->low);
  ret.low = a->low + b;

  return ret;
}

lexint_t lexint_sub(lexint_t const* a, lexint_t const* b)
{
  lexint_t ret = lexint_zero();

  if(a->is_negative == b->is_negative)
    ret.is_negative = lexint_cmp(a, b) < 0;
  else
    ret.is_negative = a->is_negative;

  ret.high = a->high - b->high - ((a->low - b->low) > a->low);
  ret.low = a->low - b->low;

  return ret;
}

lexint_t lexint_sub64(lexint_t const* a, uint64_t b)
{
  lexint_t ret = lexint_zero();

  ret.is_negative = lexint_cmp64(a, b) < 0;
  ret.high = a->high - ((a->low - b) > a->low);
  ret.low = a->low - b;

  return ret;
}

// This method is provided only to handle negative multiplications.
lexint_t lexint_mul(lexint_t const* a, lexint_t const* b)
{
  lexint_t ret = lexint_zero();

  // If the multiplier is negative, negate both operands then multiply.
  lexint_t lt = *a;
  lexint_t rt = *b;
  if(rt.is_negative)
  {
    lt = lexint_negate(&lt);
    rt = lexint_negate(&rt);
  }

  assert(rt.high == 0);
  ret = lexint_mul64(&lt, rt.low);

  return ret;
}

lexint_t lexint_mul64(lexint_t const* a, uint64_t b)
{
  lexint_t ret = lexint_zero();

  ret.is_negative = a->is_negative;

  lexint_t t = *a;

  while(b > 0)
  {
    if((b & 1) != 0)
    {
      ret = lexint_add(&ret, &t);
    }

    t = lexint_shl(&t, 1);
    b >>= 1;
  }

  return ret;
}

// This method is provided only to handle negative divisions.
lexint_t lexint_div(lexint_t const* a, lexint_t const* b)
{
  lexint_t ret = lexint_zero();

  lexint_t lt = *a;
  lexint_t rt = *b;
  bool negate = lt.is_negative ^ rt.is_negative;

  // take the absolute value of both operands and then divide
  if(lt.is_negative)
  {
    lt = lexint_negate(&lt);
  }

  if(rt.is_negative)
  {
    rt = lexint_negate(&rt);
  }

  assert(rt.high == 0);
  ret = lexint_div64(&lt, rt.low);

  ret.is_negative = false;
  if(negate)
  {
    ret = lexint_negate(&ret);
  }

  return ret;
}

lexint_t lexint_div64(lexint_t const* a, uint64_t b)
{
  bool negate = a->is_negative;
  if(a->is_negative)
  {
    lexint_t t = lexint_negate(a);
    a = &t;
  }

  lexint_t o = *a;
  lexint_t ret = lexint_zero();

  if(b == 0)
  {
    return ret;
  }

  if(b == 1)
  {
    return o;
  }

  lexint_t r = lexint_zero();
  lexint_t t = lexint_zero();

  for(uint8_t i = 127; i < UINT8_MAX; i--)
  {
    r = lexint_shl(&r, 1);
    t = lexint_shr(&o, i);
    r.low |= t.low & 1;

    if(lexint_cmp64(&r, b) >= 0)
    {
      r = lexint_sub64(&r, b);
      ret = lexint_setbit(&ret, i);
    }
  }

  ret.is_negative = false;
  if(negate)
  {
    ret = lexint_negate(&ret);
  }

  return ret;
}

lexint_t lexint_char(lexint_t const *i, int c)
{
  lexint_t ret = lexint_zero();

  ret.high = (i->high << 8) | (i->low >> 56);
  ret.low = (i->low << 8) | c;

  return ret;
}

bool lexint_accum(lexint_t* i, uint64_t digit, uint64_t base)
{
  lexint_t v2 = lexint_mul64(i, base);

  lexint_t v3 = lexint_div64(&v2, base);

  if(lexint_cmp(&v3, i) != 0)
  {
    return false;
  }

  v2 = lexint_add64(&v2, digit);

  if(lexint_cmp(&v2, i) < 0)
  {
    return false;
  }

  *i = v2;

  return true;
}

// Support for clz (count leading zeros) is suprisingly platform, and even CPU,
// dependent. Therefore, since we're not overly concerned with performance
// within the lexer, we provide a software version here.
static int count_leading_zeros(uint64_t n)
{
  if(n == 0)
    return 64;

  int count = 0;

  if((n >> 32) == 0) { count += 32; n <<= 32; }
  if((n >> 48) == 0) { count += 16; n <<= 16; }
  if((n >> 56) == 0) { count += 8; n <<= 8; }
  if((n >> 60) == 0) { count += 4; n <<= 4; }
  if((n >> 62) == 0) { count += 2; n <<= 2; }
  if((n >> 63) == 0) { count += 1; n <<= 1; }

  return count;
}

double lexint_double(lexint_t const* i)
{
  if(i->low == 0 && i->high == 0)
    return 0;

  int sig_bit_count = 128 - count_leading_zeros(i->high);

  if(i->high == 0)
    sig_bit_count = 64 - count_leading_zeros(i->low);

  uint64_t exponent = sig_bit_count - 1;
  uint64_t mantissa = i->low;

  if(sig_bit_count <= 53)
  {
    // We can represent this number exactly.
    mantissa <<= (53 - sig_bit_count);
  }
  else
  {
    // We can't exactly represents numbers of this size, have to truncate bits.
    // We have to round, first shift so we have 55 bits of mantissa.
    if(sig_bit_count == 54)
    {
      mantissa <<= 1;
    }
    else if(sig_bit_count > 55)
    {
      lexint_t t = lexint_shr(i, sig_bit_count - 55);
      mantissa = t.low;
      t = lexint_shl(&t, sig_bit_count - 55);

      if(lexint_cmp(&t, i) != 0)
      {
        // Some of the bits we're discarding are non-0. Round up mantissa.
        mantissa |= 1;
      }
    }

    // Round first 53 bits of mantissa to even an ditch extra 2 bits.
    if((mantissa & 4) != 0)
      mantissa |= 1;

    mantissa = (mantissa + 1) >> 2;

    if((mantissa & (1ULL << 53)) != 0)
    {
      mantissa >>= 1;
      exponent += 1;
    }
  }

  uint64_t raw_bits = ((exponent + 1023) << 52) | (mantissa & 0xFFFFFFFFFFFFF);
  double* fp_bits = (double*)&raw_bits;
  return *fp_bits;
}

lexint_t lexint_and(lexint_t const* a, lexint_t const* b)
{
  lexint_t ret = *a;

  ret.high = a->high & b->high;
  ret.low = a->low & b->low;

  return ret;
}

lexint_t lexint_and64(lexint_t const* a, uint64_t b)
{
  lexint_t ret = *a;

  ret.high = 0;
  ret.low = a->low & b;

  return ret;
}

lexint_t lexint_or(lexint_t const* a, lexint_t const* b)
{
  lexint_t ret = *a;

  ret.high = a->high | b->high;
  ret.low = a->low | b->low;

  return ret;
}

lexint_t lexint_or64(lexint_t const* a, uint64_t b)
{
  lexint_t ret = *a;

  ret.high = a->high;
  ret.low = a->low | b;

  return ret;
}

lexint_t lexint_xor(lexint_t const* a, lexint_t const* b)
{
  lexint_t ret = *a;

  ret.high = a->high ^ b->high;
  ret.low = a->low ^ b->low;

  return ret;
}

lexint_t lexint_xor64(lexint_t const* a, uint64_t b)
{
  lexint_t ret = *a;

  ret.high = a->high;
  ret.low = a->low ^ b;

  return ret;
}

lexint_t lexint_not(lexint_t const* src)
{
  lexint_t ret = *src;

  ret.high =~ src->high;
  ret.low =~ src->low;

  return ret;
}

lexint_t lexint_negate(lexint_t const* src)
{
  lexint_t t = lexint_zero();
  return lexint_sub(&t, src);
}

bool lexint_is_negative(lexint_t const* v)
{
  return v->is_negative;
}
