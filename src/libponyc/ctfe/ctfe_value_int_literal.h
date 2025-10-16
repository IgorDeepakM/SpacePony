#pragma once

#include "ctfe_types.h"

#include "../ast/lexint.h"
#include "../ast/ast.h"
#include "../pass/pass.h"

#include <vector>
#include <string>
#include <limits>
#include <type_traits>


class CtfeValue;
class CtfeValueBool;

class CtfeValueIntLiteral
{
  lexint_t m_val;

public:
  CtfeValueIntLiteral();
  CtfeValueIntLiteral(const CtfeValueIntLiteral& b);
  CtfeValueIntLiteral(const lexint_t& b);
  template<typename T> CtfeValueIntLiteral(T b);
  lexint_t& get_lexint() { return m_val; }
  void set(const CtfeValueIntLiteral& b);
  void zero();
  int cmp(const CtfeValueIntLiteral& b) const;
  int cmp64(uint64_t b) const;
  CtfeValueIntLiteral shl(uint64_t b) const;
  CtfeValueIntLiteral shr(uint64_t b) const;
  uint64_t testbit(uint8_t b) const;
  CtfeValueIntLiteral setbit(uint8_t b) const;
  CtfeValueIntLiteral add(const CtfeValueIntLiteral& b) const;
  CtfeValueIntLiteral add64(uint64_t b) const;
  CtfeValueIntLiteral sub(const CtfeValueIntLiteral& b) const;
  CtfeValueIntLiteral sub64(uint64_t b) const;
  CtfeValueIntLiteral mul(const CtfeValueIntLiteral& b) const;
  CtfeValueIntLiteral mul64(uint64_t b) const;
  CtfeValueIntLiteral div(const CtfeValueIntLiteral& b) const;
  CtfeValueIntLiteral div64( uint64_t b) const;

  void from_char(int c);
  bool accum(uint64_t digit, uint64_t base);
  double to_double() const;
  uint64_t to_uint64() const { return m_val.low; }

  CtfeValueIntLiteral op_and(const CtfeValueIntLiteral& b) const;
  CtfeValueIntLiteral and64(uint64_t b) const;
  CtfeValueIntLiteral op_or(const CtfeValueIntLiteral& b) const;
  CtfeValueIntLiteral or64(uint64_t b) const;
  CtfeValueIntLiteral op_xor(const CtfeValueIntLiteral& b) const;
  CtfeValueIntLiteral xor64(uint64_t b) const;
  CtfeValueIntLiteral op_not() const;

  CtfeValueIntLiteral negate() const;

  CtfeValueBool eq(const CtfeValueIntLiteral& b) const;
  CtfeValueBool ne(const CtfeValueIntLiteral& b) const;
  CtfeValueBool lt(const CtfeValueIntLiteral& b) const;
  CtfeValueBool le(const CtfeValueIntLiteral& b) const;
  CtfeValueBool gt(const CtfeValueIntLiteral& b) const;
  CtfeValueBool ge(const CtfeValueIntLiteral& b) const;

  ast_t* create_ast_literal_node();

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, CtfeValue& recv,
    const std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result);
};


template<typename T>
CtfeValueIntLiteral::CtfeValueIntLiteral(T b):
  m_val{ 0, 0, false }
{
  if constexpr (std::is_same<T, CtfeI128Type>::value ||
                std::is_same<T, CtfeU128Type>::value)
  {
    m_val.low = static_cast<uint64_t>(b);
    m_val.high = static_cast<uint64_t>(b >> 64);

    if constexpr (std::is_same<T, CtfeI128Type>::value)
    {
      if(b < 0)
      {
        m_val.is_negative = true;
      }
    }
  }
  else if constexpr (std::is_integral<T>::value)
  {
    m_val.low = static_cast<uint64_t>(static_cast<int64_t>(b));
    m_val.high = 0;

    if constexpr (std::is_signed_v<T>)
    {
      if(b < 0)
      {
        m_val.high = std::numeric_limits<uint64_t>::max();
        m_val.is_negative = true;
      }
    }
  }
}
