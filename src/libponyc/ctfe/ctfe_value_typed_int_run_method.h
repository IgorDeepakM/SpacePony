// Extra file for CtfeValueTypeInt<T> in order to avoid circular include files
#pragma once

#include "ctfe_value_typed_int.h"

#include <vector>
#include <string>
#include <limits>
#include <type_traits>		


template <typename T>
bool CtfeValueTypedInt<T>::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  const std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result)
{
  CtfeValue::Type rec_type = args[0].get_type();
  const CtfeValueTypedInt<T>& rec_val = args[0].get_typed_int<T>();

  if(args.size() == 2)
  {
    const CtfeValueTypedInt<T>& first_arg = args[1].get_typed_int<T>();

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
        result = CtfeValue(CtfeValueTypedInt<T>(), rec_type);
        return true;
      }
      if constexpr (std::is_signed<T>::value)
      {
        if(rec_val.get_value() == std::numeric_limits<T>::min() && first_arg.get_value() == -1)
        {
          result = CtfeValue(CtfeValueTypedInt<T>(), rec_type);
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
        result = CtfeValue(CtfeValueTypedInt<T>(), rec_type);
        return true;
      }
      if constexpr (std::is_signed<T>::value)
      {
        if(rec_val.get_value() == std::numeric_limits<T>::min() && first_arg.get_value() == -1)
        {
          result = CtfeValue(CtfeValueTypedInt<T>(), rec_type);
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
        throw CtfeValueException();
      }
      if constexpr (std::is_signed<T>::value)
      {
        if(rec_val.get_value() == std::numeric_limits<T>::min() && first_arg.get_value() == -1)
        {
          ast_error_frame(errors, ast,
            "Divide overflow in div_unsafe at compile time");
          throw CtfeValueException();
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
        throw CtfeValueException();
      }
      if constexpr (std::is_signed<T>::value)
      {
        if(rec_val.get_value() == std::numeric_limits<T>::min() && first_arg.get_value() == -1)
        {
          ast_error_frame(errors, ast,
            "Divide overflow in rem_unsafe at compile time");
          throw CtfeValueException();
        }
      }
      CtfeValueTypedInt<T> r = rec_val.rem(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "op_and")
    {
      CtfeValueTypedInt<T> r = rec_val.op_and(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "op_or")
    {
      CtfeValueTypedInt<T> r = rec_val.op_or(first_arg);
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "op_xor")
    {
      CtfeValueTypedInt<T> r = rec_val.op_xor(first_arg);
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
      CtfeValueTypedInt<T> r = rec_val.op_not();
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "neg")
    {
      CtfeValueTypedInt<T> r = rec_val.negate();
      result = CtfeValue(r, rec_type);
      return true;
    }
    else if(method_name == "i8" || method_name == "i8_unsafe")
    {
      CtfeValueTypedInt<int8_t> r = rec_val.cast_to<int8_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "u8" || method_name == "u8_unsafe")
    {
      CtfeValueTypedInt<uint8_t> r = rec_val.cast_to<uint8_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "i16" || method_name == "i16_unsafe")
    {
      CtfeValueTypedInt<int16_t> r = rec_val.cast_to<int16_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "u16" || method_name == "u16_unsafe")
    {
      CtfeValueTypedInt<uint16_t> r = rec_val.cast_to<uint16_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "i32" || method_name == "i32_unsafe")
    {
      CtfeValueTypedInt<int32_t> r = rec_val.cast_to<int32_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "u32" || method_name == "u32_unsafe")
    {
      CtfeValueTypedInt<uint32_t> r = rec_val.cast_to<uint32_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "i64" || method_name == "i64_unsafe")
    {
      CtfeValueTypedInt<int64_t> r = rec_val.cast_to<int64_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "u64" || method_name == "u64_unsafe")
    {
      CtfeValueTypedInt<uint64_t> r = rec_val.cast_to<uint64_t>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "i128" || method_name == "i128_unsafe")
    {
      CtfeValueTypedInt<int64_t> r = rec_val.cast_to<CtfeI128Type>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "u128" || method_name == "u128_unsafe")
    {
      CtfeValueTypedInt<uint64_t> r = rec_val.cast_to<CtfeU128Type>();
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "ilong" || method_name == "ilong_unsafe")
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
    else if(method_name == "ulong" || method_name == "ulong_unsafe")
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
    else if(method_name == "isize" || method_name == "isize_unsafe")
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
    else if(method_name == "usize" || method_name == "usize_unsafe")
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
