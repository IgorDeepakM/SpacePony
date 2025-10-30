#pragma once

#include <map>
#include <string>

#include "ctfe_value.h"

#include "../ast/ast.h"


class CtfeValueStruct
{
  static std::map<uint64_t, std::string> m_stored_obj_names;

  std::map<std::string, CtfeValue> m_vars;
  ast_t* m_type;

public:
  CtfeValueStruct(ast_t *type);
  ~CtfeValueStruct();
  bool new_value(const std::string& name, const CtfeValue& value);
  bool update_value(const std::string& name, const CtfeValue& value);
  bool get_value(const std::string& name, CtfeValue& value) const;
  ast_t* get_type_ast() const { return m_type; }

  ast_t* create_ast_literal_node(pass_opt_t* opt, errorframe_t* errors, ast_t* from);

  void write_to_memory(uint8_t* ptr) const;
  static CtfeValueStruct* read_from_memory(uint8_t* ptr);
};
