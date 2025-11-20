// Extra file for CtfeValueFloat<T> in order to avoid circular include files
#pragma once

#include "ctfe_value_float.h"

#include <vector>
#include <string>
#include <limits>
#include <type_traits>		


using namespace std::placeholders;

template<typename T>
const std::map<std::string, typename CtfeValueFloat<T>::FloatCallbacks> CtfeValueFloat<T>::m_functions {
  { "add", { std::bind(&CtfeValueFloat<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueFloat<T>::add, _1, _2)) } },
  { "add_unsafe", { std::bind(&CtfeValueFloat<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueFloat<T>::add, _1, _2)) } },
  { "sub", { std::bind(&CtfeValueFloat<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueFloat<T>::sub, _1, _2)) } },
  { "sub_unsafe", { std::bind(&CtfeValueFloat<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueFloat<T>::sub, _1, _2)) } },
  { "mul", { std::bind(&CtfeValueFloat<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueFloat<T>::mul, _1, _2)) } },
  { "mul_unsafe", { std::bind(&CtfeValueFloat<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueFloat<T>::mul, _1, _2)) } },
  { "div", { std::bind(&CtfeValueFloat<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueFloat<T>::div, _1, _2)) } },
  { "div_unsafe", { std::bind(&CtfeValueFloat<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueFloat<T>::div, _1, _2)) } },
  { "rem", { std::bind(&CtfeValueFloat<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueFloat<T>::rem, _1, _2)) } },
  { "rem_unsafe", { std::bind(&CtfeValueFloat<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueFloat<T>::rem, _1, _2)) } },
  { "neg", { std::bind(&CtfeValueFloat<T>::unary_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             UnaryOpFunction(std::bind(&CtfeValueFloat<T>::negate, _1)) } },
  { "eq", { std::bind(&CtfeValueFloat<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueFloat<T>::eq, _1, _2)) } },
  { "ne", { std::bind(&CtfeValueFloat<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueFloat<T>::ne, _1, _2)) } },
  { "lt", { std::bind(&CtfeValueFloat<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueFloat<T>::lt, _1, _2)) } },
  { "le", { std::bind(&CtfeValueFloat<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueFloat<T>::le, _1, _2)) } },
  { "gt", { std::bind(&CtfeValueFloat<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueFloat<T>::gt, _1, _2)) } },
  { "ge", { std::bind(&CtfeValueFloat<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueFloat<T>::ge, _1, _2)) } },
  { "i8", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->saturation_convert<int8_t>(), res_type); }) } },
  { "i8_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(static_cast<int8_t>(recv->m_val), res_type); }) } },
  { "u8", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->saturation_convert<uint8_t>(), res_type); }) } },
  { "u8_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(static_cast<uint8_t>(recv->m_val), res_type); }) } },
  { "i16", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->saturation_convert<int16_t>(), res_type); }) } },
  { "i16_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(static_cast<int16_t>(recv->m_val), res_type); }) } },
  { "u16", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->saturation_convert<uint16_t>(), res_type); }) } },
  { "u16_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(static_cast<uint16_t>(recv->m_val), res_type); }) } },
  { "i32", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->saturation_convert<int32_t>(), res_type); }) } },
  { "i32_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(static_cast<int32_t>(recv->m_val), res_type); }) } },
  { "u32", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->saturation_convert<uint32_t>(), res_type); }) } },
  { "u32_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(static_cast<uint32_t>(recv->m_val), res_type); }) } },
  { "i64", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->saturation_convert<int64_t>(), res_type); }) } },
  { "i64_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(static_cast<int64_t>(recv->m_val), res_type); }) } },
  { "u64", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->saturation_convert<uint64_t>(), res_type); }) } },
  { "u64_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(static_cast<uint64_t>(recv->m_val), res_type); }) } },
  { "ilong", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_long_size(), true); }) } },
  { "ilong_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
              { return recv->unsafe_cast_target_specific_int(res_type, CtfeValue::get_long_size(), true); }) } },
  { "ulong", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_long_size(), false); }) } },
  { "ulong_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
              { return recv->unsafe_cast_target_specific_int(res_type, CtfeValue::get_long_size(), false); }) } },
  { "isize", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_size_size(), true); }) } },
  { "isize_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
              { return recv->unsafe_cast_target_specific_int(res_type, CtfeValue::get_size_size(), true); }) } },
  { "usize", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_size_size(), false); }) } },
  { "usize_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
              { return recv->unsafe_cast_target_specific_int(res_type, CtfeValue::get_size_size(), false); }) } },
  { "f32", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
              { return CtfeValue(recv->saturation_convert<float>(), res_type); }) } },
  { "f32_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(static_cast<float>(recv->m_val), res_type); }) } },
  { "f64", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
              { return CtfeValue(recv->saturation_convert<double>(), res_type); }) } },
  { "f64_unsafe", { std::bind(&CtfeValueFloat<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueFloat<T>* recv, ast_t* res_type)
               { return CtfeValue(static_cast<double>(recv->m_val), res_type); }) } }
};


template <typename T>
bool CtfeValueFloat<T>::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  ast_t* res_type, CtfeValue &recv, const std::vector<CtfeValue>& args,
  const std::string& method_name, CtfeValue& result)
{
  auto it = m_functions.find(method_name);
  if(it != m_functions.end())
  {
    return it->second.unpack_function(opt, errors, ast, res_type, recv, args, method_name, result,
      it->second.operation_function);
  }

  // Rest is miscellaneous functions which can be left outside the function map

  if(args.size() == 1)
  {
    if(method_name == "from_bits")
    {
      if constexpr (std::is_same<T, double>::value)
      {
        const CtfeValueInt<uint64_t>& arg_val = args[0].get_int<uint64_t>();
        uint64_t val = arg_val.get_value();
        result = CtfeValue(CtfeValueFloat<double>(reinterpret_cast<double&>(val)), res_type);
        return true;
      }
      else if constexpr (std::is_same<T, float>::value)
      {
        const CtfeValueInt<uint32_t>& arg_val = args[0].get_int<uint32_t>();
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
    if(method_name == "_inf")
    {
      result = CtfeValue(CtfeValueFloat<T>(std::numeric_limits<T>::quiet_NaN()), res_type);
      return true;
    }
    else if(method_name == "bits")
    {
      if constexpr (std::is_same<T, double>::value)
      {
        double val = recv.get_float<double>().get_value();
        result = CtfeValue(CtfeValueInt<uint64_t>(reinterpret_cast<uint64_t&>(val)), res_type);
        return true;
      }
      else if constexpr (std::is_same<T, float>::value)
      {
        float val = recv.get_float<float>().get_value();
        result = CtfeValue(CtfeValueInt<uint32_t>(reinterpret_cast<uint32_t&>(val)), res_type);
        return true;
      }
      else
      {
        return false;
      }
    }
  }

  return false;
}


template <typename T>
bool CtfeValueFloat<T>::bin_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
{
  const CtfeValueFloat<T>& rec_val = recv.get_float<T>();
  const CtfeValueFloat<T>& first_arg = args[0].get_float<T>();
  std::function f = std::get<BinOpFunction>(op);
  CtfeValueFloat<T> r = f(&rec_val, first_arg);
  result = CtfeValue(r, res_type);
  return true;
}


template <typename T>
bool CtfeValueFloat<T>::unary_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
{
  const CtfeValueFloat<T>& rec_val = recv.get_float<T>();
  std::function f = std::get<UnaryOpFunction>(op);
  CtfeValueFloat<T> r = f(&rec_val);
  result = CtfeValue(r, res_type);
  return true;
}


template <typename T>
bool CtfeValueFloat<T>::cmp_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
{
  const CtfeValueFloat<T>& rec_val = recv.get_float<T>();
  const CtfeValueFloat<T>& first_arg = args[0].get_float<T>();
  std::function f = std::get<CmpOpFunction>(op);
  CtfeValueBool r = f(&rec_val, first_arg);
  result = CtfeValue(r, res_type);
  return true;
}


template <typename T>
bool CtfeValueFloat<T>::cast_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
{
  const CtfeValueFloat<T>& rec_val = recv.get_float<T>();

  std::function f = std::get<CastOpFunction>(op);
  result = f(&rec_val, res_type);
  return true;
}


template <typename T>
CtfeValue CtfeValueFloat<T>::cast_target_specific_int(ast_t* res_type, uint8_t size, bool sign) const
{
  CtfeValue result;

  if(sign)
  {
    if(size == 4)
    {
      CtfeValueInt<int32_t> r = saturation_convert<int32_t>();
      result = CtfeValue(r, res_type);
    }
    else if(size == 8)
    {
      CtfeValueInt<int64_t> r = saturation_convert<int64_t>();
      result = CtfeValue(r, res_type);
    }
    else
    {
      pony_assert(false);
    }
  }
  else
  {
    if(size == 4)
    {
      CtfeValueInt<uint32_t> r = saturation_convert<uint32_t>();
      result = CtfeValue(r, res_type);
    }
    else if(size == 8)
    {
      CtfeValueInt<uint64_t> r = saturation_convert<uint64_t>();
      result = CtfeValue(r, res_type);
    }
    else
    {
      pony_assert(false);
    }
  }

  return result;
}


template <typename T>
CtfeValue CtfeValueFloat<T>::unsafe_cast_target_specific_int(ast_t* res_type, uint8_t size, bool sign) const
{
  CtfeValue result;

  if(sign)
  {
    if(size == 4)
    {
      CtfeValueInt<int32_t> r = static_cast<uint32_t>(m_val);
      result = CtfeValue(r, res_type);
    }
    else if(size == 8)
    {
      CtfeValueInt<int64_t> r = static_cast<int64_t>(m_val);
      result = CtfeValue(r, res_type);
    }
    else
    {
      pony_assert(false);
    }
  }
  else
  {
    if(size == 4)
    {
      CtfeValueInt<uint32_t> r = static_cast<uint32_t>(m_val);
      result = CtfeValue(r, res_type);
    }
    else if(size == 8)
    {
      CtfeValueInt<uint64_t> r = static_cast<uint64_t>(m_val);
      result = CtfeValue(r, res_type);
    }
    else
    {
      pony_assert(false);
    }
  }

  return result;
}