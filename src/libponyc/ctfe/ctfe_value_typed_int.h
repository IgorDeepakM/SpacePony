#pragma once

#include "../ast/ast.h"
#include "../ast/lexint.h"
#include "ctfe_value_int_literal.h"
#include "ctfe_value_bool.h"
#include "ctfe_exception.h"
#include "ponyassert.h"

#include <vector>
#include <string>
#include <limits>
#include <type_traits>

#include <string.h>



class CtfeValue;
class CtfeValueInt;


template <typename T>
class CtfeValueTypedInt
{
  static_assert(std::is_integral<T>::value ||
                std::is_same<T, CtfeI128Type>::value ||
                std::is_same<T, CtfeU128Type>::value, "Only integer types are allowed");

  T m_val;

public:
  CtfeValueTypedInt(): m_val{0} {}
  CtfeValueTypedInt(T val): m_val{val} {}
  CtfeValueTypedInt(const CtfeValueTypedInt<T>& b): m_val{b.m_val} {}
  CtfeValueTypedInt(const CtfeValueIntLiteral& b);
  CtfeValueTypedInt<T> shl(uint64_t b) const { return CtfeValueTypedInt<T>(m_val << b); }
  CtfeValueTypedInt<T> shr(uint64_t b) const { return CtfeValueTypedInt<T>(m_val >> b); }
  uint64_t testbit(uint8_t b) { return (static_cast<std::make_unsigned<int>::type>(m_val) >> b) & 1; }
  CtfeValueTypedInt<T> setbit(uint8_t b) { return CtfeValueTypedInt<T>((static_cast<std::make_unsigned<int>::type>(1) << b) | m_val); }
  CtfeValueTypedInt<T> add(const CtfeValueTypedInt<T>& b) const { return CtfeValueTypedInt<T>(m_val + b.m_val); }
  CtfeValueTypedInt<T> sub(const CtfeValueTypedInt<T>& b) const { return CtfeValueTypedInt<T>(m_val - b.m_val); }
  CtfeValueTypedInt<T> mul(const CtfeValueTypedInt<T>& b) const { return CtfeValueTypedInt<T>( m_val * b.m_val); }
  CtfeValueTypedInt<T> div(const CtfeValueTypedInt<T>& b) const { return CtfeValueTypedInt<T>(m_val / b.m_val); }
  CtfeValueTypedInt<T> rem(const CtfeValueTypedInt<T>& b) const { return CtfeValueTypedInt<T>(m_val % b.m_val); }

  CtfeValueTypedInt<T> op_and(const CtfeValueTypedInt<T>& b) const { return CtfeValueTypedInt<T>(m_val & b.m_val); }
  CtfeValueTypedInt<T> op_or(const CtfeValueTypedInt<T>& b) const { return CtfeValueTypedInt<T>(m_val | b.m_val); }
  CtfeValueTypedInt<T> op_xor(const CtfeValueTypedInt<T>& b) const { return CtfeValueTypedInt<T>(m_val ^ b.m_val); }
  CtfeValueTypedInt<T> op_not() const { return CtfeValueTypedInt<T>(~m_val); }

  CtfeValueTypedInt<T> negate() const { return CtfeValueTypedInt<T>(-m_val); }

  CtfeValueBool eq(const CtfeValueTypedInt<T>& b) const { return CtfeValueBool(m_val == b.m_val); }
  CtfeValueBool ne(const CtfeValueTypedInt<T>& b) const { return CtfeValueBool(m_val != b.m_val); }
  CtfeValueBool lt(const CtfeValueTypedInt<T>& b) const { return CtfeValueBool(m_val < b.m_val); }
  CtfeValueBool le(const CtfeValueTypedInt<T>& b) const { return CtfeValueBool(m_val <= b.m_val); }
  CtfeValueBool gt(const CtfeValueTypedInt<T>& b) const { return CtfeValueBool(m_val > b.m_val); }
  CtfeValueBool ge(const CtfeValueTypedInt<T>& b) const { return CtfeValueBool(m_val >= b.m_val); }

  template<typename V>
  CtfeValueTypedInt<V> cast_to() const { return CtfeValueTypedInt<V>(static_cast<V>(m_val)); }

  T get_value() const { return m_val; }
  uint64_t to_uint64() const { return static_cast<uint64_t>(m_val); }

  void write_to_memory(uint8_t* ptr) const;
  static CtfeValueTypedInt<T> read_from_memory(uint8_t* ptr);

  ast_t* create_ast_literal_node();

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, CtfeValue &recv,
    const std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result);
};


template <typename T>
CtfeValueTypedInt<T>::CtfeValueTypedInt(const CtfeValueIntLiteral& b)
{
  lexint_t& val = const_cast<CtfeValueIntLiteral&>(b).get_lexint();

  lexint_t lexint_min = lexint_zero();
  lexint_t lexint_max = lexint_zero();
  T min_val = std::numeric_limits<T>::min();
  T max_val = std::numeric_limits<T>::max();
  if constexpr (std::is_same<T, CtfeI128Type>::value ||
                std::is_same<T, CtfeU128Type>::value)
  {
    lexint_max.low = static_cast<uint64_t>(max_val);
    lexint_max.high = static_cast<uint64_t>(max_val >> 64);

    if constexpr (std::is_same<T, CtfeI128Type>::value)
    {
      lexint_min = lexint_minusone();
    }

    lexint_min.low = static_cast<uint64_t>(min_val);
    lexint_min.high = static_cast<uint64_t>(min_val >> 64);
  }
  else
  {
    lexint_max.low = max_val;

    if constexpr (std::is_signed_v<T>)
    {
      lexint_min = lexint_minusone();
    }

    lexint_min.low = min_val;
  }

  if(lexint_cmp(&val, &lexint_min) < 0)
  {
    throw CtfeValueException();
  }
  else if(lexint_cmp(&val, &lexint_max) > 0)
  {
    throw CtfeValueException();
  }

  m_val = val.low;

  if constexpr (std::is_same<T, CtfeI128Type>::value ||
                std::is_same<T, CtfeU128Type>::value)
  {
    CtfeU128Type tmp = val.high;
    tmp <<= 64;
    m_val |= tmp;
  }
}


template <typename T>
ast_t* CtfeValueTypedInt<T>::create_ast_literal_node()
{
  ast_t* new_node = ast_blank(TK_INT);
  CtfeValueIntLiteral lit = CtfeValueIntLiteral(m_val);
  ast_set_int(new_node, &lit.get_lexint());

  return new_node;
}


template <typename T>
void CtfeValueTypedInt<T>::write_to_memory(uint8_t* ptr) const
{
  memcpy(ptr, &m_val, sizeof(T));
}

template <typename T>
CtfeValueTypedInt<T> CtfeValueTypedInt<T>::read_from_memory(uint8_t* ptr)
{
  T r = 0;
  memcpy(&r, ptr, sizeof(T));
  return CtfeValueTypedInt<T>(r);
}
