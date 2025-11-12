// Extra file for CtfeValueTypedFloat<T> in order to avoid circular include files
#pragma once

#include "ctfe_value_typed_float.h"

#include <vector>
#include <string>
#include <limits>
#include <type_traits>		


template <typename T>
bool CtfeValueTypedFloat<T>::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  ast_t* res_type, CtfeValue &recv, const std::vector<CtfeValue>& args,
  const std::string& method_name, CtfeValue& result)
{
  if(args.size() == 1)
  {
    if(method_name == "add" || method_name == "add_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueTypedFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueTypedFloat<T> r = rec_val.add(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "sub" || method_name == "sub_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueTypedFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueTypedFloat<T> r = rec_val.sub(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "mul" || method_name == "mul_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueTypedFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueTypedFloat<T> r = rec_val.mul(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "div" || method_name == "div_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueTypedFloat<T>& first_arg = args[0].get_typed_float<T>();

      CtfeValueTypedFloat<T> r = rec_val.div(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "rem" || method_name == "rem_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueTypedFloat<T>& first_arg = args[0].get_typed_float<T>();

      CtfeValueTypedFloat<T> r = rec_val.rem(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "eq")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueTypedFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.eq(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "ne")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueTypedFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.ne(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "lt")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueTypedFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.lt(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "le")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueTypedFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.le(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "gt")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueTypedFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.gt(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "ge")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      const CtfeValueTypedFloat<T>& first_arg = args[0].get_typed_float<T>();
      CtfeValueBool r = rec_val.ge(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
  }
  else if(args.size() == 0)
  {
    if(method_name == "neg")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedFloat<T> r = rec_val.negate();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i8")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<int8_t> r = rec_val.saturation_convert<int8_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i8_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<int8_t> r = static_cast<int8_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u8")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<uint8_t> r = rec_val.saturation_convert<uint8_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u8_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<uint8_t> r = static_cast<uint8_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i16")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<int16_t> r = rec_val.saturation_convert<int16_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i16_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<int16_t> r = static_cast<int16_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u16")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<uint16_t> r = rec_val.saturation_convert<uint16_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u16_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<uint16_t> r = static_cast<uint16_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i32")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<int32_t> r = rec_val.saturation_convert<int32_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i32_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<int32_t> r = static_cast<int32_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u32")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<uint32_t> r = rec_val.saturation_convert<uint32_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u32_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<uint32_t> r = static_cast<uint32_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i64")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<int64_t> r = rec_val.saturation_convert<int64_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i64_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<int64_t> r = static_cast<int64_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u64")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<uint64_t> r = rec_val.saturation_convert<uint64_t>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u64_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<uint64_t> r = static_cast<uint64_t>(rec_val.m_val);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i128")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<CtfeI128Type> r = rec_val.saturation_convert<CtfeI128Type>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "i128_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      // This is wrong but MSVC std::_Signed128 and std::_Unsigned128 has no conversion from
      // floating point types directly to a 128-bit integer type. Needs to be fixed in the future.
      CtfeValueTypedInt<CtfeI128Type> r = static_cast<CtfeI128Type>(static_cast<int64_t>(rec_val.m_val));
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u128")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<CtfeU128Type> r = rec_val.saturation_convert<CtfeU128Type>();
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "u128_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      CtfeValueTypedInt<CtfeU128Type> r = static_cast<CtfeU128Type>(static_cast<uint64_t>(rec_val.m_val));
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "ilong")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueTypedInt<int32_t> r = rec_val.saturation_convert<int32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(long_size == 8)
      {
        CtfeValueTypedInt<int64_t> r = rec_val.saturation_convert<int64_t>();
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
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueTypedInt<int32_t> r = static_cast<int32_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else if(long_size == 8)
      {
        CtfeValueTypedInt<int64_t> r = static_cast<int64_t>(rec_val.m_val);
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
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueTypedInt<uint32_t> r = rec_val.saturation_convert<uint32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(long_size == 8)
      {
        CtfeValueTypedInt<uint64_t> r = rec_val.saturation_convert<uint64_t>();
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
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t long_size = CtfeValue::get_long_size();
      if(long_size == 4)
      {
        CtfeValueTypedInt<uint32_t> r = static_cast<uint32_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else if(long_size == 8)
      {
        CtfeValueTypedInt<uint64_t> r = static_cast<uint64_t>(rec_val.m_val);
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
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueTypedInt<int32_t> r = rec_val.saturation_convert<int32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(size_size == 8)
      {
        CtfeValueTypedInt<int64_t> r = rec_val.saturation_convert<int64_t>();
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
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueTypedInt<int32_t> r = static_cast<int32_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else if(size_size == 8)
      {
        CtfeValueTypedInt<int64_t> r = static_cast<int64_t>(rec_val.m_val);
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
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueTypedInt<uint32_t> r = rec_val.saturation_convert<uint32_t>();
        result = CtfeValue(r, res_type);
      }
      else if(size_size == 8)
      {
        CtfeValueTypedInt<uint64_t> r = rec_val.saturation_convert<uint64_t>();
        result = CtfeValue(r, res_type);
      }
      else
      {
        pony_assert(false);
      }
      return true;
    }
    else if(method_name == "usize_unsafe")
    {
      const CtfeValueTypedFloat<T>& rec_val = recv.get_typed_float<T>();
      uint8_t size_size = CtfeValue::get_size_size();
      if(size_size == 4)
      {
        CtfeValueTypedInt<uint32_t> r = static_cast<uint32_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
      }
      else if(size_size == 8)
      {
        CtfeValueTypedInt<uint64_t> r = static_cast<uint64_t>(rec_val.m_val);
        result = CtfeValue(r, res_type);
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
