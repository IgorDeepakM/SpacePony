#pragma once

#include "ctfe_value_int_literal.h"
#include "ctfe_value_typed_int.h"
#include "ctfe_value_bool.h"
#include "ctfe_value_tuple.h"
#include "ctfe_value_string_literal.h"

#include "../ast/ast.h"

#include <vector>
#include <string>
#include <type_traits>	
#include <variant>


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
    TypedIntI128,
    TypedIntU128,
    TypedIntILong,
    TypedIntULong,
    TypedIntISize,
    TypedIntUSize,
    RealLiteral,
    String,
    StructRef,
    Tuple,
    StringLiteral
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
  static uint8_t m_long_size;
  static uint8_t m_size_size;

  Type m_type;
  ControlFlowModifier m_ctrlFlow;

  std::variant<
    CtfeValueBool,
    CtfeValueIntLiteral,
    CtfeValueTypedInt<int8_t>,
    CtfeValueTypedInt<uint8_t>,
    CtfeValueTypedInt<int16_t>,
    CtfeValueTypedInt<uint16_t>,
    CtfeValueTypedInt<int32_t>,
    CtfeValueTypedInt<uint32_t>,
    CtfeValueTypedInt<int64_t>,
    CtfeValueTypedInt<uint64_t>,
    CtfeValueTypedInt<CtfeI128Type>,
    CtfeValueTypedInt<CtfeU128Type>,
    CtfeValueStruct*,
    CtfeValueTuple,
    CtfeValueStringLiteral> m_val;

  void convert_from_int_literal_to_type(const CtfeValueIntLiteral& val,
    const std::string& pony_type);

public:
  CtfeValue();
  ~CtfeValue();
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
  CtfeValue(const CtfeValueTuple& val);
  CtfeValue(const CtfeValueStringLiteral& str);

  CtfeValue& operator=(const CtfeValue& val);

  Type get_type() const { return m_type; }
  bool is_none() const { return m_type == Type::None; }

  bool is_typed_int() const;

  CtfeValueIntLiteral& get_int_literal() { return std::get<CtfeValueIntLiteral>(m_val); };
  template <typename T> CtfeValueTypedInt<T>& get_typed_int();
  CtfeValueBool& get_bool() { return std::get<CtfeValueBool>(m_val); }
  CtfeValueStringLiteral& get_string_literal() { return std::get<CtfeValueStringLiteral>(m_val); }
  CtfeValueTuple& get_tuple() { return std::get<CtfeValueTuple>(m_val); }

  const CtfeValueIntLiteral& get_int_literal() const { return std::get<CtfeValueIntLiteral>(m_val); };
  template <typename T> const CtfeValueTypedInt<T>& get_typed_int() const;
  const CtfeValueBool& get_bool() const { return std::get<CtfeValueBool>(m_val); }
  const CtfeValueStringLiteral& get_string_literal() const { return std::get<CtfeValueStringLiteral>(m_val); }
  const CtfeValueTuple& get_tuple() const { return std::get<CtfeValueTuple>(m_val); }

  uint64_t to_uint64() const;

  CtfeValueStruct* get_struct_ref() const { return std::get<CtfeValueStruct*>(m_val); }

  ast_t* create_ast_literal_node(pass_opt_t* opt, errorframe_t* errors, ast_t* from);

  std::string get_pony_type_name() const;

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
    const std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result);

  ControlFlowModifier get_control_flow_modifier() const { return m_ctrlFlow; }
  void set_control_flow_modifier(ControlFlowModifier val) { m_ctrlFlow = val; }
  void clear_control_flow_modifier() { m_ctrlFlow = ControlFlowModifier::None; }

  static void initialize(pass_opt_t* opt);
  static uint8_t get_long_size() { return m_long_size; }
  static uint8_t get_size_size() { return m_size_size; }
};


template <typename T>
CtfeValue::CtfeValue(const CtfeValueTypedInt<T>& val):
  m_type{Type::None},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{val}
{
  if constexpr (std::is_same<T, int8_t>::value)
  {
    m_type = Type::TypedIntI8;
  }
  else if constexpr (std::is_same<T, uint8_t>::value)
  {
    m_type = Type::TypedIntU8;
  }
  else if constexpr (std::is_same<T, int16_t>::value)
  {
    m_type = Type::TypedIntI16;
  }
  else if constexpr (std::is_same<T, uint16_t>::value)
  {
    m_type = Type::TypedIntU16;
  }
  else if constexpr (std::is_same<T, int32_t>::value)
  {
    m_type = Type::TypedIntI32;
  }
  else if constexpr (std::is_same<T, uint32_t>::value)
  {
    m_type = Type::TypedIntU32;
  }
  else if constexpr (std::is_same<T, int64_t>::value)
  {
    m_type = Type::TypedIntI64;
  }
  else if constexpr (std::is_same<T, uint64_t>::value)
  {
    m_type = Type::TypedIntU64;
  }
  else if constexpr (std::is_same<T, CtfeI128Type>::value)
  {
    m_type = Type::TypedIntI128;
  }
  else if constexpr (std::is_same<T, CtfeU128Type>::value)
  {
    m_type = Type::TypedIntU128;
  }
  else
  {
    static_assert(false);
  }
}


template <typename T>
CtfeValue::CtfeValue(const CtfeValueTypedInt<T>& val, Type type):
  m_type{Type::None},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{val}
{
  m_val = val;

  if constexpr (std::is_same<T, int8_t>::value)
  {
    m_type = Type::TypedIntI8;
  }
  else if constexpr (std::is_same<T, uint8_t>::value)
  {
    m_type = Type::TypedIntU8;
  }
  else if constexpr (std::is_same<T, int16_t>::value)
  {
    m_type = Type::TypedIntI16;
  }
  else if constexpr (std::is_same<T, uint16_t>::value)
  {
    m_type = Type::TypedIntU16;
  }
  else if constexpr (std::is_same<T, int32_t>::value)
  {
    m_type = type;
  }
  else if constexpr (std::is_same<T, uint32_t>::value)
  {
    m_type = type;
  }
  else if constexpr (std::is_same<T, int64_t>::value)
  {
    m_type = type;
  }
  else if constexpr (std::is_same<T, uint64_t>::value)
  {
    m_type = type;
  }
  else if constexpr (std::is_same<T, CtfeI128Type>::value)
  {
    m_type = Type::TypedIntI128;
  }
  else if constexpr (std::is_same<T, CtfeU128Type>::value)
  {
    m_type = Type::TypedIntU128;
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

  return std::get<CtfeValueTypedInt<T>>(m_val);
}


template <typename T>
const CtfeValueTypedInt<T>& CtfeValue::get_typed_int() const
{
  return const_cast<const CtfeValueTypedInt<T>&>(const_cast<CtfeValue*>(this)->get_typed_int<T>());
}
