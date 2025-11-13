// Extra file for CtfeValueFloat<T> in order to avoid circular include files
#pragma once

#include "ctfe_value_float.h"

#include <vector>
#include <string>
#include <limits>
#include <type_traits>		


template <typename T>
bool CtfeValueFloat<T>::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  ast_t* res_type, CtfeValue &recv, const std::vector<CtfeValue>& args,
  const std::string& method_name, CtfeValue& result)
{
  if(args.size() == 1)
  {
    if(method_name == "add" || method_name == "add_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueFloat<T> r = rec_val.add(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "sub" || method_name == "sub_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueFloat<T> r = rec_val.sub(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "mul" || method_name == "mul_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueFloat<T> r = rec_val.mul(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "div" || method_name == "div_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueFloat<T>& first_arg = args[0].get_typed_float<T>();

      CtfeValueFloat<T> r = rec_val.div(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "rem" || method_name == "rem_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueFloat<T>& first_arg = args[0].get_typed_float<T>();

      CtfeValueFloat<T> r = rec_val.rem(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "eq")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.eq(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "ne")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.ne(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "lt")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.lt(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "le")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.le(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "gt")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.gt(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "ge")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.ge(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "from_bits")
    {
      if constexpr (std::is_same<T, double>::value)
      {
        const CtfeValueInt<uint64_t>& arg_val = args[0].get_typed_int<uint64_t>();
        uint64_t val = arg_val.get_value();
        result = CtfeValue(CtfeValueFloat<double>(reinterpret_cast<double&>(val)), res_type);
        return true;
      }
      if constexpr (std::is_same<T, float>::value)
      {
        const CtfeValueInt<uint32_t>& arg_val = args[0].get_typed_int<uint32_t>();
        uint32_t val = arg_val.get_value();
        result = CtfeValue(CtfeValueFloat<float>(reinterpret_cast<float&>(val)), res_type);
        return true;
      }
      else
      {
        return false;
      }
    }
    else if(method_name == "_inf")
    {
      bool sign = args[0].get_bool().get_value();
      if (sign)
      {
        result = CtfeValue(CtfeValueFloat<T>(-std::numeric_limits<T>::infinity()), res_type);
      }
      else
      {
        result = CtfeValue(CtfeValueFloat<T>(std::numeric_limits<T>::infinity()), res_type);
      }
      return true;
    }
  }
  else if(args.size() == 0)
  {
    if(method_name == "neg")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueFloat<T> r = rec_val.negate();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i8")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<int8_t> r = rec_val.saturation_convert<int8_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i8_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<int8_t> r = static_cast<int8_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u8")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<uint8_t> r = rec_val.saturation_convert<uint8_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u8_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<uint8_t> r = static_cast<uint8_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i16")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<int16_t> r = rec_val.saturation_convert<int16_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i16_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<int16_t> r = static_cast<int16_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u16")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<uint16_t> r = rec_val.saturation_convert<uint16_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u16_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<uint16_t> r = static_cast<uint16_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i32")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<int32_t> r = rec_val.saturation_convert<int32_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i32_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<int32_t> r = static_cast<int32_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u32")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<uint32_t> r = rec_val.saturation_convert<uint32_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u32_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<uint32_t> r = static_cast<uint32_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i64")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<int64_t> r = rec_val.saturation_convert<int64_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i64_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<int64_t> r = static_cast<int64_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u64")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<uint64_t> r = rec_val.saturation_convert<uint64_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u64_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<uint64_t> r = static_cast<uint64_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i128")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<CtfeI128Type> r = rec_val.saturation_convert<CtfeI128Type>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i128_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      // This is wrong but MSVC std::_Signed128 and std::_Unsigned128 has no conversion from
      // floating point types directly to a 128-bit integer type. Needs to be fixed in the future.
      CtfeValueInt<CtfeI128Type> r = static_cast<CtfeI128Type>(static_cast<int64_t>(rec_val.m_val));
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u128")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<CtfeU128Type> r = rec_val.saturation_convert<CtfeU128Type>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u128_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueInt<CtfeU128Type> r = static_cast<CtfeU128Type>(static_cast<uint64_t>(rec_val.m_val));
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "ilong")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueInt<int32_t> r = rec_val.saturation_convert<int32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(long_size == 8)
      {
        CtfeValueInt<int64_t> r = rec_val.saturation_convert<int64_t>();
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "ilong_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueInt<int32_t> r = static_cast<int32_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else if(long_size == 8)
      {
        CtfeValueInt<int64_t> r = static_cast<int64_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "ulong")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueInt<uint32_t> r = rec_val.saturation_convert<uint32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(long_size == 8)
      {
        CtfeValueInt<uint64_t> r = rec_val.saturation_convert<uint64_t>();
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "ulong_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueInt<uint32_t> r = static_cast<uint32_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else if(long_size == 8)
      {
        CtfeValueInt<uint64_t> r = static_cast<uint64_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "isize")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueInt<int32_t> r = rec_val.saturation_convert<int32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(size_size == 8)
      {
        CtfeValueInt<int64_t> r = rec_val.saturation_convert<int64_t>();
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "isize_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueInt<int32_t> r = static_cast<int32_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else if(size_size == 8)
      {
        CtfeValueInt<int64_t> r = static_cast<int64_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "usize")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueInt<uint32_t> r = rec_val.saturation_convert<uint32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(size_size == 8)
      {
        CtfeValueInt<uint64_t> r = rec_val.saturation_convert<uint64_t>();
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
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueFloat<float> r = rec_val.saturation_convert<float>();
      result = CtfeValue(r, res_type);
      return true;
      }
    else if(method_name == "f32_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueFloat<float> r = static_cast<float>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "f64")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueFloat<double> r = rec_val.saturation_convert<double>();
      result = CtfeValue(r, res_type);
      return true;
      }
    else if(method_name == "f64_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueFloat<double> r = static_cast<double>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "usize_unsafe")
    {
      const CtfeValueFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueInt<uint32_t> r = static_cast<uint32_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else if(size_size == 8)
      {
        CtfeValueInt<uint64_t> r = static_cast<uint64_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "_inf")
    {
      result = CtfeValue(CtfeValueFloat<T>(std::numeric_limits<T>::quiet_NaN()), res_type);
      return true;
    }
  }

  return false;
}
