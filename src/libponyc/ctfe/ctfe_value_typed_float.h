#pragma once

#include "../ast/ast.h"
#include "../ast/lexint.h"
#include "ctfe_value_bool.h"
#include "ctfe_value_int_literal.h"

#include <vector>
#include <string>
#include <limits>
#include <type_traits>
#include <cstdint>
#include <cmath>

#include <string.h>



class CtfeValue;


template <typename T>
class CtfeValueTypedFloat
{
  static_assert(std::is_floating_point<T>::value, "Only floating point types are allowed");

  T m_val;

public:
  CtfeValueTypedFloat(): m_val{0} {}
  CtfeValueTypedFloat(T val): m_val{val} {}
  CtfeValueTypedFloat(const CtfeValueIntLiteral& b);
  CtfeValueTypedFloat(const CtfeValueTypedFloat<T>& b): m_val{b.m_val} {}
  CtfeValueTypedFloat<T> add(const CtfeValueTypedFloat<T>& b) const { return CtfeValueTypedFloat<T>(m_val + b.m_val); }
  CtfeValueTypedFloat<T> sub(const CtfeValueTypedFloat<T>& b) const { return CtfeValueTypedFloat<T>(m_val - b.m_val); }
  CtfeValueTypedFloat<T> mul(const CtfeValueTypedFloat<T>& b) const { return CtfeValueTypedFloat<T>( m_val * b.m_val); }
  CtfeValueTypedFloat<T> div(const CtfeValueTypedFloat<T>& b) const { return CtfeValueTypedFloat<T>(m_val / b.m_val); }
  CtfeValueTypedFloat<T> rem(const CtfeValueTypedFloat<T>& b) const { return CtfeValueTypedFloat<T>(fmod(m_val, b.m_val)); }

  CtfeValueTypedFloat<T> negate() const { return CtfeValueTypedFloat<T>(-m_val); }

  CtfeValueBool eq(const CtfeValueTypedFloat<T>& b) const { return CtfeValueBool(m_val == b.m_val); }
  CtfeValueBool ne(const CtfeValueTypedFloat<T>& b) const { return CtfeValueBool(m_val != b.m_val); }
  CtfeValueBool lt(const CtfeValueTypedFloat<T>& b) const { return CtfeValueBool(m_val < b.m_val); }
  CtfeValueBool le(const CtfeValueTypedFloat<T>& b) const { return CtfeValueBool(m_val <= b.m_val); }
  CtfeValueBool gt(const CtfeValueTypedFloat<T>& b) const { return CtfeValueBool(m_val > b.m_val); }
  CtfeValueBool ge(const CtfeValueTypedFloat<T>& b) const { return CtfeValueBool(m_val >= b.m_val); }

  template<typename V>
  CtfeValueTypedFloat<V> cast_to() const { return CtfeValueTypedFloat<V>(static_cast<V>(m_val)); }

  T get_value() const { return m_val; }
  template<typename V> V saturation_convert() const;

  void write_to_memory(uint8_t* ptr) const;
  static CtfeValueTypedFloat<T> read_from_memory(uint8_t* ptr);

  ast_t* create_ast_literal_node();

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue &recv, const std::vector<CtfeValue>& args, const std::string& method_name,
    CtfeValue& result);
};


template <typename T>
CtfeValueTypedFloat<T>::CtfeValueTypedFloat(const CtfeValueIntLiteral& b)
{
  m_val = b.to_double();
}


template <typename T>
ast_t* CtfeValueTypedFloat<T>::create_ast_literal_node()
{
  ast_t* new_node = ast_blank(TK_FLOAT);
  double lit = m_val;
  ast_set_float(new_node, lit);

  return new_node;
}


template <typename T>
void CtfeValueTypedFloat<T>::write_to_memory(uint8_t* ptr) const
{
  memcpy(ptr, &m_val, sizeof(T));
}

template <typename T>
CtfeValueTypedFloat<T> CtfeValueTypedFloat<T>::read_from_memory(uint8_t* ptr)
{
  T r = 0;
  memcpy(&r, ptr, sizeof(T));
  return CtfeValueTypedFloat<T>(r);
}


template<typename T>
template<typename V>
V CtfeValueTypedFloat<T>::saturation_convert() const
{
  double to_max{};
  double to_min{};
  
  
  if constexpr (std::is_same<V, CtfeI128Type>::value ||
                std::is_same<V, CtfeU128Type>::value)
  {
    lexint_t v_min = lexint_zero();
    V min_val = std::numeric_limits<V>::min();
    v_min.low = static_cast<uint64_t>(min_val);
    v_min.high = static_cast<uint64_t>(min_val >> 64);

    if constexpr (std::is_same<T, CtfeI128Type>::value)
    {
      if (min_val < 0)
      {
        v_min.is_negative = true;
      }
    }

    to_min = lexint_double(&v_min);

    lexint_t v_max = lexint_zero();
    V max_val = std::numeric_limits<V>::max();
    v_max.low = static_cast<uint64_t>(max_val);
    v_max.high = static_cast<uint64_t>(max_val >> 64);

    if constexpr (std::is_same<T, CtfeI128Type>::value)
    {
      if (max_val < 0)
      {
        v_max.is_negative = true;
      }
    }

    to_max = lexint_double(&v_max);
  }
  else
  {
    to_min = std::numeric_limits<V>::min();
    to_max = std::numeric_limits<V>::max();
  }

  if(m_val < to_min)
  {
    return std::numeric_limits<V>::min();
  }
  else if(m_val > to_max)
  {
    return std::numeric_limits<V>::max();
  }
  else
  {
    if constexpr (std::is_same<V, CtfeI128Type>::value ||
                  std::is_same<V, CtfeU128Type>::value)
    {
      return static_cast<V>(0);
    }
    else
    {
      return static_cast<V>(m_val);
    }
  }
}
