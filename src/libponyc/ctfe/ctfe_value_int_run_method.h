// Extra file for CtfeValueInt<T> in order to avoid circular include files
#pragma once

#include "ctfe_value_int.h"
#include "ctfe_value_float.h"

#include <vector>
#include <string>
#include <limits>
#include <type_traits>


using namespace std::placeholders;

template<typename T>
const std::map<std::string, typename CtfeValueInt<T>::IntCallbacks> CtfeValueInt<T>::m_functions {
  { "add", { std::bind(&CtfeValueInt<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::add, _1, _2)) } },
  { "add_unsafe", { std::bind(&CtfeValueInt<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::add, _1, _2)) } },
  { "sub", { std::bind(&CtfeValueInt<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::sub, _1, _2)) } },
  { "sub_unsafe", { std::bind(&CtfeValueInt<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::sub, _1, _2)) } },
  { "mul", { std::bind(&CtfeValueInt<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::mul, _1, _2)) } },
  { "mul_unsafe", { std::bind(&CtfeValueInt<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::mul, _1, _2)) } },
  { "div", { std::bind(&CtfeValueInt<T>::divrem_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::div, _1, _2)) } },
  { "div_unsafe", { std::bind(&CtfeValueInt<T>::unsafe_divrem_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::div, _1, _2)) } },
  { "rem", { std::bind(&CtfeValueInt<T>::divrem_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::rem, _1, _2)) } },
  { "rem_unsafe", { std::bind(&CtfeValueInt<T>::unsafe_divrem_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::rem, _1, _2)) } },
  { "op_and", { std::bind(&CtfeValueInt<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::op_and, _1, _2)) } },
  { "op_or", { std::bind(&CtfeValueInt<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::op_or, _1, _2)) } },
  { "op_xor", { std::bind(&CtfeValueInt<T>::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             BinOpFunction(std::bind(&CtfeValueInt<T>::op_xor, _1, _2)) } },
  { "shl", { std::bind(&CtfeValueInt<T>::shift_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             ShiftOpFunction(std::bind(&CtfeValueInt<T>::shl, _1, _2)) } },
  { "shr", { std::bind(&CtfeValueInt<T>::shift_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             ShiftOpFunction(std::bind(&CtfeValueInt<T>::shr, _1, _2)) } },
  { "eq", { std::bind(&CtfeValueInt<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueInt<T>::eq, _1, _2)) } },
  { "ne", { std::bind(&CtfeValueInt<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueInt<T>::ne, _1, _2)) } },
  { "lt", { std::bind(&CtfeValueInt<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueInt<T>::lt, _1, _2)) } },
  { "le", { std::bind(&CtfeValueInt<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueInt<T>::le, _1, _2)) } },
  { "gt", { std::bind(&CtfeValueInt<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueInt<T>::gt, _1, _2)) } },
  { "ge", { std::bind(&CtfeValueInt<T>::cmp_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CmpOpFunction(std::bind(&CtfeValueInt<T>::ge, _1, _2)) } },
  { "op_not", { std::bind(&CtfeValueInt<T>::unary_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             UnaryOpFunction(std::bind(&CtfeValueInt<T>::op_not, _1)) } },
  { "neg", { std::bind(&CtfeValueInt<T>::unary_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             UnaryOpFunction(std::bind(&CtfeValueInt<T>::negate, _1)) } },
  { "i8", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<int8_t>(), res_type); }) } },
  { "i8_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<int8_t>(), res_type); }) } },
  { "u8", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<uint8_t>(), res_type); }) } },
  { "u8_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<uint8_t>(), res_type); }) } },
  { "i16", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<int16_t>(), res_type); }) } },
  { "i16_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<int16_t>(), res_type); }) } },
  { "u16", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<uint16_t>(), res_type); }) } },
  { "u16_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<uint16_t>(), res_type); }) } },
  { "i32", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<int32_t>(), res_type); }) } },
  { "i32_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<int32_t>(), res_type); }) } },
  { "u32", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<uint32_t>(), res_type); }) } },
  { "u32_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<uint32_t>(), res_type); }) } },
  { "i64", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<int64_t>(), res_type); }) } },
  { "i64_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<int64_t>(), res_type); }) } },
  { "u64", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<uint64_t>(), res_type); }) } },
  { "u64_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<uint64_t>(), res_type); }) } },
  { "i128", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<CtfeI128Type>(), res_type); }) } },
  { "i128_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<CtfeI128Type>(), res_type); }) } },
  { "u128", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<CtfeU128Type>(), res_type); }) } },
  { "u128_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to<CtfeU128Type>(), res_type); }) } },
  { "ilong", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_long_size(), true); }) } },
  { "ilong_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_long_size(), true); }) } },
  { "ulong", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_long_size(), false); }) } },
  { "ulong_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_long_size(), false); }) } },
  { "isize", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_size_size(), true); }) } },
  { "isize_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_size_size(), true); }) } },
  { "usize", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_size_size(), false); }) } },
  { "usize_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
              { return recv->cast_target_specific_int(res_type, CtfeValue::get_size_size(), false); }) } },
  { "f32", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
              { return CtfeValue(recv->cast_to_float_saturate<float>(), res_type); }) } },
  { "f32_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to_float<float>(), res_type); }) } },
  { "f64", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
            CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
              { return CtfeValue(recv->cast_to_float_saturate<double>(), res_type); }) } },
  { "f64_unsafe", { std::bind(&CtfeValueInt<T>::cast_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
             CastOpFunction([](const CtfeValueInt<T>* recv, ast_t* res_type)
               { return CtfeValue(recv->cast_to_float<double>(), res_type); }) } }
};


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
  auto it = m_functions.find(method_name);
  if(it != m_functions.end())
  {
    return it->second.unpack_function(opt, errors, ast, res_type, recv, args, method_name, result,
      it->second.operation_function);
  }

  return false;
}


template <typename T>
bool CtfeValueInt<T>::bin_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
{
  const CtfeValueInt<T>& rec_val = recv.get_int<T>();
  const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
  std::function f = std::get<BinOpFunction>(op);
  CtfeValueInt<T> r = f(&rec_val, first_arg);
  result = CtfeValue(r, res_type);
  return true;
}


template <typename T>
bool CtfeValueInt<T>::unary_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
{
  const CtfeValueInt<T>& rec_val = recv.get_int<T>();
  std::function f = std::get<UnaryOpFunction>(op);
  CtfeValueInt<T> r = f(&rec_val);
  result = CtfeValue(r, res_type);
  return true;
}


template <typename T>
bool CtfeValueInt<T>::shift_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
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
      const CtfeValueInt<typename std::make_unsigned<T>::type>& first_arg = args[0].get_int<typename std::make_unsigned<T>::type>();
      shift_amount = first_arg.to_uint64();
    }
  }
  else
  {
    const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
    shift_amount = first_arg.to_uint64();
  }

  std::function f = std::get<ShiftOpFunction>(op);
  CtfeValueInt<T> r = f(&rec_val, shift_amount);
  result = CtfeValue(r, res_type);
  return true;
}


template <typename T>
bool CtfeValueInt<T>::cmp_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
{
  const CtfeValueInt<T>& rec_val = recv.get_int<T>();
  const CtfeValueInt<T>& first_arg = args[0].get_int<T>();
  std::function f = std::get<CmpOpFunction>(op);
  CtfeValueBool r = f(&rec_val, first_arg);
  result = CtfeValue(r, res_type);
  return true;
}


template <typename T>
bool CtfeValueInt<T>::divrem_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
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

  std::function f = std::get<BinOpFunction>(op);
  CtfeValueInt<T> r = f(&rec_val, first_arg);
  result = CtfeValue(r, res_type);
  return true;
}


template <typename T>
bool CtfeValueInt<T>::unsafe_divrem_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
{
  const CtfeValueInt<T>& rec_val = recv.get_int<T>();
  const CtfeValueInt<T>& first_arg = args[0].get_int<T>();

  if(first_arg.get_value() == 0)
  {
    ast_error_frame(errors, ast,
      "Divide by zero in '%s' operation at compile time", method_name.c_str());
    throw CtfeValueException();
  }
  if constexpr(std::is_signed<T>::value)
  {
    if(rec_val.get_value() == std::numeric_limits<T>::min() && first_arg.get_value() == -1)
    {
      ast_error_frame(errors, ast,
        "Divide overflow in '%s' operation at compile time", method_name.c_str());
      throw CtfeValueException();
    }
  }

  std::function f = std::get<BinOpFunction>(op);
  CtfeValueInt<T> r = f(&rec_val, first_arg);
  result = CtfeValue(r, res_type);
  return true;
}


template <typename T>
bool CtfeValueInt<T>::cast_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
{
  const CtfeValueInt<T>& rec_val = recv.get_int<T>();

  std::function f = std::get<CastOpFunction>(op);
  result = f(&rec_val, res_type);
  return true;
}


template <typename T>
CtfeValue CtfeValueInt<T>::cast_target_specific_int(ast_t* res_type, uint8_t size, bool sign) const
{
  CtfeValue result;

  if(sign)
  {
    if(size == 4)
    {
      CtfeValueInt<int32_t> r = cast_to<int32_t>();
      result = CtfeValue(r, res_type);
    }
    else if(size == 8)
    {
      CtfeValueInt<int64_t> r = cast_to<int64_t>();
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
      CtfeValueInt<uint32_t> r = cast_to<uint32_t>();
      result = CtfeValue(r, res_type);
    }
    else if(size == 8)
    {
      CtfeValueInt<uint64_t> r = cast_to<uint64_t>();
      result = CtfeValue(r, res_type);
    }
    else
    {
      pony_assert(false);
    }
  }

  return result;
}
