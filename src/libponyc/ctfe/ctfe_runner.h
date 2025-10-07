#pragma once

#include "../ast/ast.h"
#include "../pass/pass.h"

#include "ctfe_frames.h"
#include "ctfe_value.h"

#include <vector>

class CtfeRunner
{
  CtfeFrames m_frames;
  std::vector<CtfeValue> m_allocated;

  CtfeValue evaluate_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, int depth);
  CtfeValue evaluate(pass_opt_t* opt, errorframe_t* errors, ast_t* expression, int depth);
  void add_allocated_reference(CtfeValue& ref);
  bool populate_struct_members(pass_opt_t* opt, errorframe_t* errors, CtfeValueStruct* s,
    ast_t* members);

public:
  CtfeRunner(pass_opt_t* opt);
  ~CtfeRunner();
  bool run(pass_opt_t* opt, ast_t** astp);
};
