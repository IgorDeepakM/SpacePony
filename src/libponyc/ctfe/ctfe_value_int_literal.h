#pragma once

#include "../ast/lexint.h"
#include "../ast/ast.h"
#include "../pass/pass.h"

#include <vector>
#include <string>


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
  int cmp64(uint64_t b);
  CtfeValueIntLiteral shl(uint64_t b);
  CtfeValueIntLiteral shr(uint64_t b);
  uint64_t testbit(uint8_t b);
  CtfeValueIntLiteral setbit(uint8_t b);
  CtfeValueIntLiteral add(CtfeValueIntLiteral& b);
  CtfeValueIntLiteral add64(uint64_t b);
  CtfeValueIntLiteral sub(CtfeValueIntLiteral& b);
  CtfeValueIntLiteral sub64(uint64_t b);
  CtfeValueIntLiteral mul(CtfeValueIntLiteral& b);
  CtfeValueIntLiteral mul64(uint64_t b);
  CtfeValueIntLiteral div(CtfeValueIntLiteral& b);
  CtfeValueIntLiteral div64( uint64_t b);

  void from_char(int c);
  bool accum(uint64_t digit, uint64_t base);
  double to_double();
  uint64_t to_uint64() const { return m_val.low; }

  CtfeValueIntLiteral and(CtfeValueIntLiteral& b);
  CtfeValueIntLiteral and64(uint64_t b);
  CtfeValueIntLiteral or(CtfeValueIntLiteral& b);
  CtfeValueIntLiteral or64(uint64_t b);
  CtfeValueIntLiteral xor(CtfeValueIntLiteral& b);
  CtfeValueIntLiteral xor64(uint64_t b);
  CtfeValueIntLiteral not();

  CtfeValueIntLiteral negate();

  CtfeValueBool eq(CtfeValueIntLiteral& b);
  CtfeValueBool ne(CtfeValueIntLiteral& b);
  CtfeValueBool lt(CtfeValueIntLiteral& b);
  CtfeValueBool le(CtfeValueIntLiteral& b);
  CtfeValueBool gt(CtfeValueIntLiteral& b);
  CtfeValueBool ge(CtfeValueIntLiteral& b);

  ast_t* create_ast_literal_node();

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
    std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result);
};


template<typename T>
CtfeValueIntLiteral::CtfeValueIntLiteral(T b)
{
  lexint_zero(&m_val);

  if constexpr (std::is_integral<T>::value)
  {
    if constexpr (std::is_signed<T>::value)
    {
      if constexpr (sizeof(T) <= 8)
      {
        m_val.low = static_cast<int64_t>(b);
        if(b < 0)
        {
          m_val.high = 0xffffffffffffffff;
        }
      }
      else
      {
        m_val.low = static_cast<int64_t>(b);
        m_val.high = static_cast<int64_t>(b >> 64);
      }

      if(b < 0)
      {
        m_val.is_negative = true;
      }
    }
    else
    {
      if constexpr (sizeof(T) <= 8)
      {
        m_val.low = static_cast<uint64_t>(b);
      }
      else
      {
        m_val.low = static_cast<uint64_t>(b);
        m_val.high = static_cast<uint64_t>(b >> 64);
      }
    }
  }
}
