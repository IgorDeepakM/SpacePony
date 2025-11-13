// Extra file for CtfeValueInt<T> in order to avoid circular include files
#pragma once

#include "ctfe_value_int.h"
#include "ctfe_value_float.h"

#include <vector>
#include <string>
#include <limits>
#include <type_traits>


template<typename T>
template<typename V>
CtfeValueFloat<V> CtfeValueInt<T>::cast_to_float() const
{
  V fv = 0;

  if constexpr (std::is_same<T, CtfeI128Type>::value ||
                std::is_same<T, CtfeU128Type>::value)
  {
    T tmp_val = m_val;

    if constexpr (std::is_same<T, CtfeI128Type>::value)
    {
      if(m_val < 0)
      {
        tmp_val = -tmp_val;
      }
    }

    lexint_t v = lexint_zero();
    v.low = static_cast<uint64_t>(tmp_val);
    v.high = static_cast<uint64_t>(tmp_val >> 64);

    fv = static_cast<V>(lexint_double(&v));

    if constexpr (std::is_same<T, CtfeI128Type>::value)
    {
      if(m_val < 0)
      {
        fv = -fv;
      }
    }
  }
  else
  {
    fv = static_cast<V>(m_val);
  }

  return CtfeValueFloat<V>(fv);
}


template<typename T>
template<typename V>
CtfeValueFloat<V> CtfeValueInt<T>::cast_to_float_saturate() const
{
  if(std::is_same<V, float>::value)
  {
    double v = cast_to_float<V>().get_value();

    if (v < std::numeric_limits<V>::lowest())
    {
      return std::numeric_limits<V>::lowest();
    }
    else if (v > std::numeric_limits<V>::max())
    {
      return std::numeric_limits<V>::max();
    }

    return static_cast<float>(v);
  }
  else
  {
    return cast_to_float<V>();
  }
}


template <typename T>
bool CtfeValueInt<T>::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  ast_t* res_type, CtfeValue &recv, const std::vector<CtfeValue>& args,
  const std::string& method_name, CtfeValue& result)
{
  if(args.size() == 1)
  {
    if(method_name == "add" || method_name == "add_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueInt<T> r = rec_val.add(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "sub" || method_name == "sub_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueInt<T> r = rec_val.sub(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "mul" || method_name == "mul_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueInt<T> r = rec_val.mul(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "div")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();

      if(first_arg.get_value() == 0)
      {
        result = CtfeValue(CtfeValueInt<T>(), res_type);
        return true;
      }
      if constexpr (std::is_signed<T>::value)
      {
        if(rec_val.get_value() == std::numeric_limits<T>::min() && first_arg.get_value() == -1)
        {
          result = CtfeValue(CtfeValueInt<T>(), res_type);
          return true;
        }
      }
      CtfeValueInt<T> r = rec_val.div(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "rem")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();

      if(first_arg.get_value() == 0)
      {
        result = CtfeValue(CtfeValueInt<T>(), res_type);
        return true;
      }
      if constexpr (std::is_signed<T>::value)
      {
        if(rec_val.get_value() == std::numeric_limits<T>::min() && first_arg.get_value() == -1)
        {
          result = CtfeValue(CtfeValueInt<T>(), res_type);
          return true;
        }
      }
      CtfeValueInt<T> r = rec_val.rem(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "div_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();

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
      CtfeValueInt<T> r = rec_val.div(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "rem_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();

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
      CtfeValueInt<T> r = rec_val.rem(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "op_and")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueInt<T> r = rec_val.op_and(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "op_or")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueInt<T> r = rec_val.op_or(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "op_xor")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueInt<T> r = rec_val.op_xor(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "shl")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      uint64_t shift_amount = 0;
      if(CtfeAstType::is_signed(recv.get_type_ast()) &&
         !CtfeAstType::is_signed(args[0].get_type_ast()))
      {
        if constexpr (std::is_same<T, CtfeI128Type>::value || std::is_same<T, CtfeU128Type>::value)
        {
          const CtfeValueInt<CtfeU128Type>& first_arg = args[0].get_int<CtfeU128Type>();
          shift_amount = first_arg.to_uint64();
        }
        else
        {
          const CtfeValueInt<std::make_unsigned<T>::type>& first_arg = args[0].get_int<std::make_unsigned<T>::type>();
          shift_amount = first_arg.to_uint64();
        }
      }
      else
      {
        const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
        shift_amount = first_arg.to_uint64();
      }

      CtfeValueInt<T> r = rec_val.shl(shift_amount);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "shr")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      uint64_t shift_amount = 0;
      if(CtfeAstType::is_signed(recv.get_type_ast()) &&
         !CtfeAstType::is_signed(args[0].get_type_ast()))
      {
        if constexpr (std::is_same<T, CtfeI128Type>::value || std::is_same<T, CtfeU128Type>::value)
        {
          const CtfeValueInt<CtfeU128Type>& first_arg = args[0].get_int<CtfeU128Type>();
          shift_amount = first_arg.to_uint64();
        }
        else
        {
          const CtfeValueInt<std::make_unsigned<T>::type>& first_arg = args[0].get_int<std::make_unsigned<T>::type>();
          shift_amount = first_arg.to_uint64();
        }
      }
      else
      {
        const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
        shift_amount = first_arg.to_uint64();
      }

      CtfeValueInt<T> r = rec_val.shr(shift_amount);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "eq")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueBool r = rec_val.eq(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "ne")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueBool r = rec_val.ne(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "lt")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueBool r = rec_val.lt(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "le")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueBool r = rec_val.le(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "gt")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueBool r = rec_val.gt(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "ge")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
      CtfeValueBool r = rec_val.ge(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
  }
  else if(args.size() == 0)
  {
    if(method_name == "op_not")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<T> r = rec_val.op_not();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "neg")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<T> r = rec_val.negate();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i8" || method_name == "i8_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<int8_t> r = rec_val.cast_to<int8_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u8" || method_name == "u8_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<uint8_t> r = rec_val.cast_to<uint8_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i16" || method_name == "i16_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<int16_t> r = rec_val.cast_to<int16_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u16" || method_name == "u16_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<uint16_t> r = rec_val.cast_to<uint16_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i32" || method_name == "i32_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<int32_t> r = rec_val.cast_to<int32_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u32" || method_name == "u32_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<uint32_t> r = rec_val.cast_to<uint32_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i64" || method_name == "i64_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<int64_t> r = rec_val.cast_to<int64_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u64" || method_name == "u64_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<uint64_t> r = rec_val.cast_to<uint64_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i128" || method_name == "i128_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<CtfeI128Type> r = rec_val.cast_to<CtfeI128Type>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u128" || method_name == "u128_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueInt<CtfeU128Type> r = rec_val.cast_to<CtfeU128Type>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "ilong" || method_name == "ilong_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueInt<int32_t> r = rec_val.cast_to<int32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(long_size == 8)
      {
        CtfeValueInt<int64_t> r = rec_val.cast_to<int64_t>();
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "ulong" || method_name == "ulong_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueInt<uint32_t> r = rec_val.cast_to<uint32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(long_size == 8)
      {
        CtfeValueInt<uint64_t> r = rec_val.cast_to<uint64_t>();
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "isize" || method_name == "isize_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueInt<int32_t> r = rec_val.cast_to<int32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(size_size == 8)
      {
        CtfeValueInt<int64_t> r = rec_val.cast_to<int64_t>();
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "usize" || method_name == "usize_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueInt<uint32_t> r = rec_val.cast_to<uint32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(size_size == 8)
      {
        CtfeValueInt<uint64_t> r = rec_val.cast_to<uint64_t>();
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "f32")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueFloat<float> r = rec_val.cast_to_float_saturate<float>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if (method_name == "f32_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueFloat<float> r = rec_val.cast_to_float<float>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if (method_name == "f64")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueFloat<double> r = rec_val.cast_to_float_saturate<double>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if (method_name == "f64_unsafe")
    {
      const CtfeValueInt<T>& rec_val = recv.get_int<T>();
      CtfeValueFloat<double> r = rec_val.cast_to_float<double>();
      result = CtfeValue(r, res_type);
      return true;
    }
  }

  return false;
}
