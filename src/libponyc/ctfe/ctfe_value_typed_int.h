#pragma once

#include "../ast/ast.h"
#include "../ast/lexint.h"
#include "ctfe_value_int_literal.h"
#include "ctfe_value_bool.h"
#include "ponyassert.h"

#include <vector>
#include <string>
#include <limits>
#include <type_traits>		


class CtfeValue;
class CtfeValueInt;


template <typename T>
class CtfeValueTypedInt
{
  static_assert(std::is_integral<T>::value, "Only integer types are allowed");

  T m_val;

public:
  CtfeValueTypedInt() { m_val = 0; }
  CtfeValueTypedInt(T val): m_val{val} {}
  CtfeValueTypedInt(const CtfeValueTypedInt<T>& b) { m_val = b.m_val; }
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

  CtfeValueTypedInt<T> and(const CtfeValueTypedInt<T>& b) const { return CtfeValueTypedInt<T>(m_val & b.m_val); }
  CtfeValueTypedInt<T> or(const CtfeValueTypedInt<T>& b) const { return CtfeValueTypedInt<T>(m_val | b.m_val); }
  CtfeValueTypedInt<T> xor(const CtfeValueTypedInt<T>& b) const { return CtfeValueTypedInt<T>(m_val ^ b.m_val); }
  CtfeValueTypedInt<T> not() const { return CtfeValueTypedInt<T>(~m_val); }

  CtfeValueTypedInt<T> negate() { return CtfeValueTypedInt<T>(-m_val); }

  CtfeValueBool eq(const CtfeValueTypedInt<T>& b) { return CtfeValueBool(m_val == b.m_val); }
  CtfeValueBool ne(const CtfeValueTypedInt<T>& b) { return CtfeValueBool(m_val != b.m_val); }
  CtfeValueBool lt(const CtfeValueTypedInt<T>& b) { return CtfeValueBool(m_val < b.m_val); }
  CtfeValueBool le(const CtfeValueTypedInt<T>& b) { return CtfeValueBool(m_val <= b.m_val); }
  CtfeValueBool gt(const CtfeValueTypedInt<T>& b) { return CtfeValueBool(m_val > b.m_val); }
  CtfeValueBool ge(const CtfeValueTypedInt<T>& b) { return CtfeValueBool(m_val >= b.m_val); }

  template<typename V>
  CtfeValueTypedInt<V> cast_to() { return CtfeValueTypedInt<V>(static_cast<V>(m_val)); }

  T get_value() const { return m_val; }
  uint64_t to_uint64() const { return static_cast<uint64_t>(m_val); }

  ast_t* create_ast_literal_node(pass_opt_t* opt, ast_t* from);

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
    std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result);
};


template <typename T>
CtfeValueTypedInt<T>::CtfeValueTypedInt(const CtfeValueIntLiteral& b)
{
  lexint_t& val = const_cast<CtfeValueIntLiteral&>(b).get_lexint();

  T min_val = std::numeric_limits<T>::min();
  T max_val = std::numeric_limits<T>::max();

  lexint_t lexint_min;
  lexint_zero(&lexint_min);
  if constexpr (sizeof(T) > 8)
  {
    lexint_min.high = min_val >> 64;
  }
  else
  {
    lexint_min.high = 0;
  }
  lexint_min.low = min_val;
  if constexpr (std::is_signed_v<T>)
  {
    lexint_min.is_negative = true;
  }

  lexint_t lexint_max;
  lexint_zero(&lexint_max);
  if constexpr (sizeof(T) > 8)
  {
    lexint_max.high = max_val >> 64;
  }
  else
  {
    lexint_min.high = 0;
  }
  lexint_max.low = max_val;

  if(lexint_cmp(&val, &lexint_min) < 0)
  {
    throw new CtfeValueException;
  }
  else if(lexint_cmp(&val, &lexint_max) > 0)
  {
    throw new CtfeValueException;
  }

  m_val = val.low;

  if constexpr (sizeof(T) > 8)
  {
    m_val |= val.high << 64;
  }
}


template <typename T>
bool CtfeValueTypedInt<T>::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result)
{
  CtfeValue::Type rec_type = args[0].get_type();
  CtfeValueTypedInt<T>& rec_val = args[0].get_typed_int<T>();

  if(args.size() == 2)
  {
    CtfeValueTypedInt<T>& first_arg = args[1].get_typed_int<T>();

    if(method_name == "add" || method_name == "add_unsafe")
    {
      CtfeValueTypedInt<T> r = rec_val.add(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "sub" || method_name == "sub_unsafe")
    {
      CtfeValueTypedInt<T> r = rec_val.sub(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "mul" || method_name == "mul_unsafe")
    {
      CtfeValueTypedInt<T> r = rec_val.mul(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "div")
    {
      if(first_arg.get_value() == 0)
      {
        result = CtfeValue(CtfeValueTypedInt<T>(0), rec_type);
        return true;
      }
      if constexpr (is_signed<T>::value)
      {
        if(rec_val.get_value() == numeric_limits<T>::min() && first_arg.get_value() == -1)
        {
          result = CtfeValue(CtfeValueTypedInt<T>(0), rec_type);
          return true;
        }
      }
      CtfeValueTypedInt<T> r = rec_val.div(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "rem")
    {
      if(first_arg.get_value() == 0)
      {
        result = CtfeValue(CtfeValueTypedInt<T>(0), rec_type);
        return true;
      }
      if constexpr (is_signed<T>::value)
      {
        if(rec_val.get_value() == numeric_limits<T>::min() && first_arg.get_value() == -1)
        {
          result = CtfeValue(CtfeValueTypedInt<T>(0), rec_type);
          return true;
        }
      }
      CtfeValueTypedInt<T> r = rec_val.rem(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "div_unsafe")
    {
      if(first_arg.get_value() == 0)
      {
        ast_error_frame(errors, ast,
          "Divide by zero in div_unsafe at compile time");
        result = CtfeValue(CtfeValue::Type::ComptimeError);
        return true;
      }
      if constexpr (is_signed<T>::value)
      {
        if(rec_val.get_value() == numeric_limits<T>::min() && first_arg.get_value() == -1)
        {
          ast_error_frame(errors, ast,
            "Divide overflow in div_unsafe at compile time");
          result = CtfeValue(CtfeValue::Type::ComptimeError);
          return true;
        }
      }
      CtfeValueTypedInt<T> r = rec_val.div(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "rem_unsafe")
    {
      if(first_arg.get_value() == 0)
      {
        ast_error_frame(errors, ast,
          "Divide by zero in rem_unsafe at compile time");
        result = CtfeValue(CtfeValue::Type::ComptimeError);
        return true;
      }
      if constexpr (is_signed<T>::value)
      {
        if(rec_val.get_value() == numeric_limits<T>::min() && first_arg.get_value() == -1)
        {
          ast_error_frame(errors, ast,
            "Divide overflow in rem_unsafe at compile time");
          result = CtfeValue(CtfeValue::Type::ComptimeError);
          return true;
        }
      }
      CtfeValueTypedInt<T> r = rec_val.rem(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "op_and")
    {
      CtfeValueTypedInt<T> r = rec_val.and(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "op_or")
    {
      CtfeValueTypedInt<T> r = rec_val.or(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "op_xor")
    {
      CtfeValueTypedInt<T> r = rec_val.xor(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "shl")
    {
      CtfeValueTypedInt<T> r = rec_val.shl(first_arg.to_uint64());
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "shr")
    {
      CtfeValueTypedInt<T> r = rec_val.shr(first_arg.to_uint64());
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "eq")
    {
      CtfeValueBool r = rec_val.eq(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "ne")
    {
      CtfeValueBool r = rec_val.ne(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "lt")
    {
      CtfeValueBool r = rec_val.lt(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "le")
    {
      CtfeValueBool r = rec_val.le(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "gt")
    {
      CtfeValueBool r = rec_val.gt(first_arg);
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "ge")
    {
      CtfeValueBool r = rec_val.ge(first_arg);
      result = CtfeValue(r);
      return true;
    }
  }
  else if(args.size() == 1)
  {
    if(method_name == "op_not")
    {
      CtfeValueTypedInt<T> r = rec_val.not();
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "neg")
    {
      CtfeValueTypedInt<T> r = rec_val.negate();
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "i8")
    {
      CtfeValueTypedInt<int8_t> r = rec_val.cast_to<int8_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "u8")
    {
      CtfeValueTypedInt<uint8_t> r = rec_val.cast_to<uint8_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "i16")
    {
      CtfeValueTypedInt<int16_t> r = rec_val.cast_to<int16_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "u16")
    {
      CtfeValueTypedInt<uint16_t> r = rec_val.cast_to<uint16_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "i32")
    {
      CtfeValueTypedInt<int32_t> r = rec_val.cast_to<int32_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "u32")
    {
      CtfeValueTypedInt<uint32_t> r = rec_val.cast_to<uint32_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "i64")
    {
      CtfeValueTypedInt<int64_t> r = rec_val.cast_to<int64_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "u64")
    {
      CtfeValueTypedInt<uint64_t> r = rec_val.cast_to<uint64_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "ilong")
    {
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueTypedInt<int32_t> r = rec_val.cast_to<int32_t>();
        result = CtfeValue(r, CtfeValue::Type::TypedIntILong);
      }
      else if(long_size == 8)
      {
        CtfeValueTypedInt<int64_t> r = rec_val.cast_to<int64_t>();
        result = CtfeValue(r, CtfeValue::Type::TypedIntILong);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "ulong")
    {
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueTypedInt<uint32_t> r = rec_val.cast_to<uint32_t>();
        result = CtfeValue(r, CtfeValue::Type::TypedIntULong);
      }
      else if(long_size == 8)
      {
        CtfeValueTypedInt<uint64_t> r = rec_val.cast_to<uint64_t>();
        result = CtfeValue(r, CtfeValue::Type::TypedIntULong);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "isize")
    {
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueTypedInt<int32_t> r = rec_val.cast_to<int32_t>();
        result = CtfeValue(r, CtfeValue::Type::TypedIntISize);
      }
      else if(size_size == 8)
      {
        CtfeValueTypedInt<int64_t> r = rec_val.cast_to<int64_t>();
        result = CtfeValue(r, CtfeValue::Type::TypedIntISize);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "usize")
    {
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueTypedInt<uint32_t> r = rec_val.cast_to<uint32_t>();
        result = CtfeValue(r, CtfeValue::Type::TypedIntUSize);
      }
      else if(size_size == 8)
      {
        CtfeValueTypedInt<uint64_t> r = rec_val.cast_to<uint64_t>();
        result = CtfeValue(r, CtfeValue::Type::TypedIntUSize);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
  }

  return false;
}


template <typename T>
ast_t* CtfeValueTypedInt<T>::create_ast_literal_node(pass_opt_t* opt, ast_t* from)
{
  ast_t* new_node = ast_blank(TK_INT);
  CtfeValueIntLiteral lit = CtfeValueIntLiteral(m_val);
  ast_set_int(new_node, &lit.get_lexint());

  return new_node;
}