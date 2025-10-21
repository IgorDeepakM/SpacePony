#pragma once

#include <map>
#include <string>

#include "ctfe_value.h"

#include "../ast/ast.h"


class CtfeValueStruct
{
  std::map<std::string, CtfeValue> m_vars;
  ast_t* m_type;

public:
  CtfeValueStruct(ast_t *type);
  ~CtfeValueStruct();
  bool new_value(const std::string& name, const CtfeValue& value);
  bool update_value(const std::string& name, const CtfeValue& value);
  bool get_value(const std::string& name, CtfeValue& value) const;
  ast_t* get_type_ast() const { return m_type; }

  void write_to_memory(uint8_t* ptr) const;
  static CtfeValueStruct* read_from_memory(uint8_t* ptr);
};
