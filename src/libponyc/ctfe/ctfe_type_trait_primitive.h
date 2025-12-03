#pragma once


#include "../ast/ast.h"
#include "../pass/pass.h"

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <variant>

class CtfeValue;
class CtfeRunner;

class CtfeTypeTraitPrimitive
{
  using OneTypeArg = std::function<bool(ast_t*)>;
  using TwoTypeArgs = std::function<bool(ast_t*, ast_t*, pass_opt_t*)>;

  using OperationFunction = std::variant<
    OneTypeArg,
    TwoTypeArgs>;

  struct TypeTraitPrimitiveCallbacks
  {
    std::function<bool(pass_opt_t*, errorframe_t*, ast_t*, ast_t*, CtfeValue&,
      const std::vector<CtfeValue>&, ast_t*, const std::string&, CtfeValue&,
      const OperationFunction&)> unpack_function;
    OperationFunction operation_function;
  };

  static const std::map<std::string, TypeTraitPrimitiveCallbacks> m_functions;

  static bool one_type_arg(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue& recv, const std::vector<CtfeValue>& args, ast_t* typearg, const std::string& method_name,
    CtfeValue& result, const OperationFunction& op);
  static bool two_type_args(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue& recv, const std::vector<CtfeValue>& args, ast_t* typearg, const std::string& method_name,
    CtfeValue& result, const OperationFunction& op);

public:
  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue& recv, const std::vector<CtfeValue>& args, ast_t* typeargs,
    const std::string& method_name, CtfeValue& result);
};
