#pragma once

#include "ctfe_types.h"
#include "ctfe_value_int_literal.h"
#include "ctfe_value_typed_int.h"
#include "ctfe_value_typed_float.h"
#include "ctfe_value_bool.h"
#include "ctfe_value_tuple.h"
#include "ctfe_value_pointer.h"
#include "ctfe_ast_type.h"

#include "../ast/ast.h"

#include <vector>
#include <string>
#include <type_traits>	
#include <variant>


class CtfeValueStruct;

class CtfeValue
{
public:
  enum class ControlFlowModifier
  {
    None,
    Return,
    Break,
    Continue,
    Error
  };

private:
  ast_t* m_type;
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
    CtfeValueTypedFloat<float>,
    CtfeValueTypedFloat<double>,
    CtfeValueStruct*,
    CtfeValueTuple,
    CtfeValuePointer> m_val;

  void convert_from_int_literal_to_type(const CtfeValueIntLiteral& val, ast_t* type);

public:
  CtfeValue();
  CtfeValue(ast_t* type);
  CtfeValue(const CtfeValueIntLiteral& val, ast_t* ast_type);
  template <typename T>
  CtfeValue(const CtfeValueTypedInt<T>& val, ast_t* ast_type);
  template <typename T>
  CtfeValue(const CtfeValueTypedFloat<T>& val, ast_t* ast_type);
  CtfeValue(const CtfeValueBool& val, ast_t* ast_type);
  CtfeValue(CtfeValueStruct* ref, ast_t* ast_type);
  CtfeValue(const CtfeValueTuple& val, ast_t* ast_type);
  CtfeValue(const CtfeValuePointer& p, ast_t* ast_type);

  bool operator==(const CtfeValue& b) const;
  bool operator!=(const CtfeValue& b) const;
  CtfeValueBool eq(const CtfeValue& b) const;

  ast_t* get_type_ast() const { return m_type; }
  void set_type_ast(ast_t* new_type);

  bool is_empty() const { return m_type == nullptr; }
  bool is_none() const { return CtfeAstType::is_none(m_type); }
  bool is_bool() const { return CtfeAstType::is_bool(m_type); }
  bool is_typed_int() const { return CtfeAstType::is_typed_int(m_type); }
  bool is_typed_float() const { return CtfeAstType::is_typed_float(m_type); }
  bool is_machine_word() const { return CtfeAstType::is_machine_word(m_type); }
  bool is_pointer() const { return CtfeAstType::is_pointer(m_type); }
  bool is_struct() const { return CtfeAstType::is_struct(m_type); }
  bool is_tuple() const { return CtfeAstType::is_tuple(m_type); }

  CtfeValueIntLiteral& get_int_literal() { return std::get<CtfeValueIntLiteral>(m_val); };
  template <typename T> CtfeValueTypedInt<T>& get_typed_int();
  template <typename T> CtfeValueTypedFloat<T>& get_typed_float();
  CtfeValueBool& get_bool() { return std::get<CtfeValueBool>(m_val); }
  CtfeValueTuple& get_tuple() { return std::get<CtfeValueTuple>(m_val); }
  CtfeValuePointer& get_pointer() { return std::get<CtfeValuePointer>(m_val); }

  const CtfeValueIntLiteral& get_int_literal() const { return std::get<CtfeValueIntLiteral>(m_val); };
  template <typename T> const CtfeValueTypedInt<T>& get_typed_int() const;
  template <typename T> const CtfeValueTypedFloat<T>& get_typed_float() const;
  const CtfeValueBool& get_bool() const { return std::get<CtfeValueBool>(m_val); }
  const CtfeValueTuple& get_tuple() const { return std::get<CtfeValueTuple>(m_val); }
  const CtfeValuePointer& get_pointer() const { return std::get<CtfeValuePointer>(m_val); }

  uint64_t to_uint64() const;
  void write_to_memory(uint8_t* ptr) const;
  static CtfeValue read_from_memory(ast_t* type, uint8_t* ptr);

  CtfeValueStruct* get_struct_ref() const { return std::get<CtfeValueStruct*>(m_val); }

  ast_t* create_ast_literal_node(pass_opt_t* opt, errorframe_t* errors, ast_t* from);

  std::string get_type_name() const { return CtfeAstType::get_type_name(m_type); }

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
    CtfeValue& result, CtfeRunner &ctfeRunner);

  ControlFlowModifier get_control_flow_modifier() const { return m_ctrlFlow; }
  void set_control_flow_modifier(ControlFlowModifier val) { m_ctrlFlow = val; }
  void clear_control_flow_modifier() { m_ctrlFlow = ControlFlowModifier::None; }

  static uint8_t get_long_size() { return CtfeAstType::get_long_size(); }
  static uint8_t get_size_size() { return CtfeAstType::get_size_size(); }

  size_t get_size_of_type() { return CtfeAstType::get_size_of_type(m_type); }
};


template <typename T>
CtfeValue::CtfeValue(const CtfeValueTypedInt<T>& val, ast_t* type):
  m_type{type},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{val}
{

}


template <typename T>
CtfeValue::CtfeValue(const CtfeValueTypedFloat<T>& val, ast_t* type):
  m_type{type},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{val}
{

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
CtfeValueTypedFloat<T>& CtfeValue::get_typed_float()
{
  if(!is_typed_float())
  {
    pony_assert(false);
  }

  return std::get<CtfeValueTypedFloat<T>>(m_val);
}


template <typename T>
const CtfeValueTypedInt<T>& CtfeValue::get_typed_int() const
{
  return const_cast<const CtfeValueTypedInt<T>&>(const_cast<CtfeValue*>(this)->get_typed_int<T>());
}


template <typename T>
const CtfeValueTypedFloat<T>& CtfeValue::get_typed_float() const
{
  return const_cast<const CtfeValueTypedFloat<T>&>(const_cast<CtfeValue*>(this)->get_typed_float<T>());
}
