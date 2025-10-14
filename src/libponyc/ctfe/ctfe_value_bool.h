#pragma once

#include "../ast/ast.h"
#include "../pass/pass.h"

#include <vector>
#include <string>


class CtfeValue;

class CtfeValueBool
{
  bool m_val;

public:
  CtfeValueBool();
  CtfeValueBool(bool val);

  CtfeValueBool op_and(const CtfeValueBool& b) const;
  CtfeValueBool op_or(const CtfeValueBool& b) const;
  CtfeValueBool op_xor(const CtfeValueBool& b) const;
  CtfeValueBool op_not() const;

  bool get_value() const { return m_val; }

  void write_to_memory(uint8_t* ptr) const;
  static CtfeValueBool read_from_memory(uint8_t* ptr);

  ast_t* create_ast_literal_node(pass_opt_t* opt, ast_t* from);

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, CtfeValue& recv,
    const std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result);
};
