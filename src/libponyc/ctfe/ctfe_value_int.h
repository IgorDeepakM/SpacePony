#pragma once

#include "../ast/ast.h"
#include "../ast/lexint.h"
#include "ctfe_value_bool.h"
#include "ctfe_exception.h"
#include "ponyassert.h"

#include <vector>
#include <string>
#include <limits>
#include <type_traits>

#include <string.h>



class CtfeValue;
template <typename T> class CtfeValueFloat;


template <typename T>
class CtfeValueInt
{
  static_assert(std::is_integral<T>::value ||
                std::is_same<T, CtfeI128Type>::value ||
                std::is_same<T, CtfeU128Type>::value, "Only integer types are allowed");

  T m_val;

public:
  CtfeValueInt(): m_val{0} {}
  CtfeValueInt(T val): m_val{val} {}
  CtfeValueInt(const CtfeValueInt<T>& b): m_val{b.m_val} {}
  CtfeValueInt(const lexint_t& val);
  CtfeValueInt<T> shl(uint64_t b) const { return CtfeValueInt<T>(m_val << b); }
  CtfeValueInt<T> shr(uint64_t b) const { return CtfeValueInt<T>(m_val >> b); }
  uint64_t testbit(uint8_t b) { return (static_cast<std::make_unsigned<int>::type>(m_val) >> b) & 1; }
  CtfeValueInt<T> setbit(uint8_t b) { return CtfeValueInt<T>((static_cast<std::make_unsigned<int>::type>(1) << b) | m_val); }
  CtfeValueInt<T> add(const CtfeValueInt<T>& b) const { return CtfeValueInt<T>(m_val + b.m_val); }
  CtfeValueInt<T> sub(const CtfeValueInt<T>& b) const { return CtfeValueInt<T>(m_val - b.m_val); }
  CtfeValueInt<T> mul(const CtfeValueInt<T>& b) const { return CtfeValueInt<T>( m_val * b.m_val); }
  CtfeValueInt<T> div(const CtfeValueInt<T>& b) const { return CtfeValueInt<T>(m_val / b.m_val); }
  CtfeValueInt<T> rem(const CtfeValueInt<T>& b) const { return CtfeValueInt<T>(m_val % b.m_val); }

  CtfeValueInt<T> op_and(const CtfeValueInt<T>& b) const { return CtfeValueInt<T>(m_val & b.m_val); }
  CtfeValueInt<T> op_or(const CtfeValueInt<T>& b) const { return CtfeValueInt<T>(m_val | b.m_val); }
  CtfeValueInt<T> op_xor(const CtfeValueInt<T>& b) const { return CtfeValueInt<T>(m_val ^ b.m_val); }
  CtfeValueInt<T> op_not() const { return CtfeValueInt<T>(~m_val); }

  CtfeValueInt<T> negate() const { return CtfeValueInt<T>(-m_val); }

  CtfeValueBool eq(const CtfeValueInt<T>& b) const { return CtfeValueBool(m_val == b.m_val); }
  CtfeValueBool ne(const CtfeValueInt<T>& b) const { return CtfeValueBool(m_val != b.m_val); }
  CtfeValueBool lt(const CtfeValueInt<T>& b) const { return CtfeValueBool(m_val < b.m_val); }
  CtfeValueBool le(const CtfeValueInt<T>& b) const { return CtfeValueBool(m_val <= b.m_val); }
  CtfeValueBool gt(const CtfeValueInt<T>& b) const { return CtfeValueBool(m_val > b.m_val); }
  CtfeValueBool ge(const CtfeValueInt<T>& b) const { return CtfeValueBool(m_val >= b.m_val); }

  template<typename V>
  CtfeValueInt<V> cast_to() const { return CtfeValueInt<V>(static_cast<V>(m_val)); }
  template<typename V>
  CtfeValueFloat<V> cast_to_float() const;
  template<typename V>
  CtfeValueFloat<V> cast_to_float_saturate() const;

  T get_value() const { return m_val; }
  uint64_t to_uint64() const { return static_cast<uint64_t>(m_val); }

  void write_to_memory(uint8_t* ptr) const;
  static CtfeValueInt<T> read_from_memory(uint8_t* ptr);

  ast_t* create_ast_literal_node();

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue &recv, const std::vector<CtfeValue>& args, const std::string& method_name,
    CtfeValue& result);
};


template <typename T>
CtfeValueInt<T>::CtfeValueInt(const lexint_t& val)
{
  m_val = val.low;

  if constexpr (std::is_same<T, CtfeI128Type>::value ||
                std::is_same<T, CtfeU128Type>::value)
  {
    CtfeU128Type tmp = val.high;
    tmp <<= 64;
    m_val |= tmp;
  }
}


template<typename T>
ast_t* CtfeValueInt<T>::create_ast_literal_node()
{
  ast_t* new_node = ast_blank(TK_INT);
  lexint_t lit = lexint_zero();
  lit.low = static_cast<uint64_t>(m_val);

  if constexpr(std::is_same<T, CtfeI128Type>::value ||
               std::is_same<T, CtfeU128Type>::value)
  {
    lit.high = static_cast<uint64_t>(m_val >> 64);
  }

  ast_set_int(new_node, &lit);

  return new_node;
}


template <typename T>
void CtfeValueInt<T>::write_to_memory(uint8_t* ptr) const
{
  memcpy(ptr, &m_val, sizeof(T));
}

template <typename T>
CtfeValueInt<T> CtfeValueInt<T>::read_from_memory(uint8_t* ptr)
{
  T r = 0;
  memcpy(&r, ptr, sizeof(T));
  return CtfeValueInt<T>(r);
}
