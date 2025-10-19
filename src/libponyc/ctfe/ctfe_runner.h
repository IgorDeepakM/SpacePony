#pragma once

#include "../ast/ast.h"
#include "../pass/pass.h"

#include "ctfe_frames.h"
#include "ctfe_value.h"
#include "ctfe_exception.h"

#include <vector>

class CtfeRunner
{
  CtfeFrames m_frames;
  std::vector<CtfeValue> m_allocated;

  CtfeValue call_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast_pos, ast_t* res_type,
    const char* method_name, ast_t* recv_type, CtfeValue& recv_val,
  const std::vector<CtfeValue>& args, ast_t* typeargs, int depth);
  CtfeValue evaluate_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, int depth);
  CtfeValue evaluate(pass_opt_t* opt, errorframe_t* errors, ast_t* expression, int depth);
  bool populate_struct_members(pass_opt_t* opt, errorframe_t* errors, CtfeValueStruct* s,
    ast_t* members);
  void left_side_assign(pass_opt_t* opt, errorframe_t* errors, ast_t* left,
    CtfeValue& right_val, int depth);

  CtfeValue handle_ffi_call(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, int depth);
  CtfeValue handle_ffi_ptr_ptr_size(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
    const std::string& ffi_name, const std::vector<CtfeValue>& evaluated_args);

public:
  CtfeRunner(pass_opt_t* opt);
  ~CtfeRunner();
  bool run(pass_opt_t* opt, ast_t** astp);
  void add_allocated_reference(const CtfeValue& ref);
};
