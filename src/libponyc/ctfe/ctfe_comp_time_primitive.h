#pragma once


#include "../ast/ast.h"
#include "../pass/pass.h"

#include <vector>
#include <string>
#include <filesystem>

class CtfeValue;
class CtfeRunner;

class CtfeCompTimePrimitive
{
  static std::filesystem::path get_prepend_path(pass_opt_t* opt, const CtfeValue& arg);

public:
  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
    CtfeValue& result, CtfeRunner& ctfeRunner);
};
