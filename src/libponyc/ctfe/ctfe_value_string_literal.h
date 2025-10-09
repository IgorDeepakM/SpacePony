#pragma once

#include "../ast/ast.h"
#include "../pass/pass.h"

#include <string>
#include <vector>


class CtfeValue;

class CtfeValueStringLiteral
{
  std::string m_str;

public:
  CtfeValueStringLiteral();
  CtfeValueStringLiteral(const std::string& str);

  CtfeValueStringLiteral add(const CtfeValueStringLiteral& b) const;

  const std::string& get_string() const { return m_str; }

  ast_t* create_ast_literal_node(pass_opt_t* opt, ast_t* from);

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
    const std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result);
};
