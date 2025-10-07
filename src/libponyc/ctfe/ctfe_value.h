#pragma once

#include "ctfe_value_int_literal.h"
#include "ctfe_value_exception.h"
#include "ctfe_value_typed_int.h"
#include "ctfe_value_bool.h"

#include "../ast/ast.h"

#include <vector>
#include <string>
#include <type_traits>	


class CtfeValueStruct;

class CtfeValue
{
public:
  enum class Type
  {
    None,
    Bool,
    IntLiteral,
    TypedIntI8,
    TypedIntU8,
    TypedIntI16,
    TypedIntU16,
    TypedIntI32,
    TypedIntU32,
    TypedIntI64,
    TypedIntU64,
    TypedIntILong,
    TypedIntULong,
    TypedIntISize,
    TypedIntUSize,
    RealLiteral,
    String,
    StructRef,
    ComptimeError,
  };

  enum class ControlFlowModifier
  {
    None,
    Return,
    Break,
    Continue,
    Error
  };

private:
  static bool m_static_initialized;
  static std::string m_ilong_type_name;
  static std::string m_ulong_type_name;
  static std::string m_isize_type_name;
  static std::string m_usize_type_name;
  static uint8_t m_long_size;
  static uint8_t m_size_size;

  Type m_type;
  ControlFlowModifier m_ctrlFlow;

  union
  {
    CtfeValueBool m_bool_value;
    CtfeValueIntLiteral m_int_literal_value;
    CtfeValueTypedInt<int8_t> m_i8_value;
    CtfeValueTypedInt<uint8_t> m_u8_value;
    CtfeValueTypedInt<int16_t> m_i16_value;
    CtfeValueTypedInt<uint16_t> m_u16_value;
    CtfeValueTypedInt<int32_t> m_i32_value;
    CtfeValueTypedInt<uint32_t> m_u32_value;
    CtfeValueTypedInt<int64_t> m_i64_value;
    CtfeValueTypedInt<uint64_t> m_u64_value;
    CtfeValueStruct* m_struct_ref;
  };

  void convert_from_int_literal_to_type(const CtfeValueIntLiteral& val,
    const std::string& pony_type);

public:
  CtfeValue();
  CtfeValue(Type type);
  CtfeValue(const CtfeValue& val);
  CtfeValue(const CtfeValueIntLiteral& val);
  CtfeValue(const CtfeValueIntLiteral& val, const std::string& pony_type);
  template <typename T>
  CtfeValue(const CtfeValueTypedInt<T>& val);
  template <typename T>
  CtfeValue(const CtfeValueTypedInt<T>& val, Type type);
  CtfeValue(const CtfeValue& val, const std::string& pony_type);
  CtfeValue(const CtfeValueBool& val);
  CtfeValue(CtfeValueStruct* ref);
  CtfeValue(ast_t *ast);

  Type get_type() const { return m_type; }
  bool is_none() const { return m_type == Type::None; }

  bool has_comptime_error() const { return m_type == Type::ComptimeError; }

  CtfeValueIntLiteral& get_int_literal() { return m_int_literal_value; };

  bool is_typed_int() const;
  template <typename T>
  CtfeValueTypedInt<T>& get_typed_int();
  CtfeValueBool& get_bool() { return m_bool_value; }

  uint64_t to_uint64() const;

  CtfeValueStruct* get_struct_ref() const { return m_struct_ref; }

  ast_t* create_ast_literal_node(pass_opt_t* opt, errorframe_t* errors, ast_t* from);

  std::string get_pony_type_name() const;

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
    std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result);

  ControlFlowModifier get_control_flow_modifier() const { return m_ctrlFlow; }
  void set_control_flow_modifier(ControlFlowModifier val) { m_ctrlFlow = val; }
  void clear_control_flow_modifier() { m_ctrlFlow = ControlFlowModifier::None; }

  static void initialize(pass_opt_t* opt);
  static uint8_t get_long_size() { return m_long_size; }
  static uint8_t get_size_size() { return m_size_size; }
};


template <typename T>
CtfeValue::CtfeValue(const CtfeValueTypedInt<T>& val):
  m_ctrlFlow{ControlFlowModifier::None}
{
  if constexpr (std::is_same<T, int8_t>::value)
  {
    m_type = Type::TypedIntI8;
    m_i8_value = val;
  }
  else if constexpr (std::is_same<T, uint8_t>::value)
  {
    m_type = Type::TypedIntU8;
    m_u8_value = val;
  }
  else if constexpr (std::is_same<T, int16_t>::value)
  {
    m_type = Type::TypedIntI16;
    m_i16_value = val;
  }
  else if constexpr (std::is_same<T, uint16_t>::value)
  {
    m_type = Type::TypedIntU16;
    m_u16_value = val;
  }
  else if constexpr (std::is_same<T, int32_t>::value)
  {
    m_type = Type::TypedIntI32;
    m_i32_value = val;
  }
  else if constexpr (std::is_same<T, uint32_t>::value)
  {
    m_type = Type::TypedIntU32;
    m_u32_value = val;
  }
  else if constexpr (std::is_same<T, int64_t>::value)
  {
    m_type = Type::TypedIntI64;
    m_i64_value = val;
  }
  else if constexpr (std::is_same<T, uint64_t>::value)
  {
    m_type = Type::TypedIntU64;
    m_u64_value = val;
  }
  else
  {
    static_assert(false);
  }
}


template <typename T>
CtfeValue::CtfeValue(const CtfeValueTypedInt<T>& val, Type type):
  m_ctrlFlow{ControlFlowModifier::None}
{
  if constexpr (std::is_same<T, int8_t>::value)
  {
    m_i8_value = val;
    m_type = Type::TypedIntI8;
  }
  else if constexpr (std::is_same<T, uint8_t>::value)
  {
    m_u8_value = val;
    m_type = Type::TypedIntU8;
  }
  else if constexpr (std::is_same<T, int16_t>::value)
  {
    m_i16_value = val;
    m_type = Type::TypedIntI16;
  }
  else if constexpr (std::is_same<T, uint16_t>::value)
  {
    m_u16_value = val;
    m_type = Type::TypedIntU16;
  }
  else if constexpr (std::is_same<T, int32_t>::value)
  {
    m_i32_value = val;
    m_type = type;
  }
  else if constexpr (std::is_same<T, uint32_t>::value)
  {
    m_u32_value = val;
    m_type = type;
  }
  else if constexpr (std::is_same<T, int64_t>::value)
  {
    m_i64_value = val;
    m_type = type;
  }
  else if constexpr (std::is_same<T, uint64_t>::value)
  {
    m_u64_value = val;
    m_type = type;
  }
  else
  {
    static_assert(false);
  }
}


template <typename T>
CtfeValueTypedInt<T>& CtfeValue::get_typed_int()
{
  if(!is_typed_int())
  {
    pony_assert(false);
  }

  if constexpr (std::is_same<T, int8_t>::value)
  {
    return m_i8_value;
  }
  else if constexpr (std::is_same<T, uint8_t>::value)
  {
    return m_u8_value;
  }
  else if constexpr (std::is_same<T, int16_t>::value)
  {
    return m_i16_value;
  }
  else if constexpr (std::is_same<T, uint16_t>::value)
  {
    return m_u16_value;
  }
  else if constexpr (std::is_same<T, int32_t>::value)
  {
    return m_i32_value;
  }
  else if constexpr (std::is_same<T, uint32_t>::value)
  {
    return m_u32_value;
  }
  else if constexpr (std::is_same<T, int64_t>::value)
  {
    return m_i64_value;
  }
  else if constexpr (std::is_same<T, uint64_t>::value)
  {
    return m_u64_value;
  }
  else
  {
    static_assert(false);
  }
}
