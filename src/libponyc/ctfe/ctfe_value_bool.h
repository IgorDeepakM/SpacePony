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

  CtfeValueBool and(const CtfeValueBool& b);
  CtfeValueBool or(const CtfeValueBool& b);
  CtfeValueBool xor(const CtfeValueBool& b);
  CtfeValueBool not();

  bool get_value() const { return m_val; }

  ast_t* create_ast_literal_node(pass_opt_t* opt, ast_t* from);

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
    std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result);
};
