#pragma once

#include "../ast/ast.h"
#include "../pass/pass.h"

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <variant>


class CtfeValue;


class CtfeValueBool
{
  using BinOpFunction = std::function<CtfeValueBool(const CtfeValueBool*, const CtfeValueBool&)>;
  using UnaryOpFunction = std::function<CtfeValueBool(const CtfeValueBool*)>;

  using OperationFunction = std::variant<
    BinOpFunction,
    UnaryOpFunction>;

  struct BoolCallbacks
  {
    std::function<bool(pass_opt_t*, errorframe_t*, ast_t*, ast_t*, CtfeValue&,
      const std::vector<CtfeValue>&, const std::string&, CtfeValue&, const OperationFunction&)> unpack_function;
    OperationFunction operation_function;
  };

  static const std::map<std::string, BoolCallbacks> m_functions;

  bool m_val;


  static bool bin_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
    CtfeValue& result, const OperationFunction& op);
  static bool unary_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
    CtfeValue& result, const OperationFunction& op);

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

  ast_t* create_ast_literal_node();

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
    CtfeValue& result);
};
