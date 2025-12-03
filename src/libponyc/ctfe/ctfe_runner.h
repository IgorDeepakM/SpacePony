#pragma once

#include "../ast/ast.h"
#include "../pass/pass.h"
#include "../type/reify.h"

#include "ctfe_frames.h"
#include "ctfe_value.h"
#include "ctfe_exception.h"

#include <vector>
#include <map>
#include <chrono>
#include <condition_variable>

class CtfeRunner
{
  CtfeFrames m_frames;
  std::vector<CtfeValue> m_allocated;
  std::map<uint64_t, ast_t*> m_cached_ast;

  size_t m_max_recursion_depth;
  size_t m_current_recursion_depth;
  volatile bool m_max_duration_exceeded;
  std::condition_variable m_terminate;
  std::mutex m_mutex;
  volatile bool m_stop_thread;

  CtfeValue call_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast_pos, ast_t* res_type,
    const char* method_name, ast_t* recv_type, CtfeValue& recv_val,
  const std::vector<CtfeValue>& args, ast_t* typeargs);
  CtfeValue evaluate_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast);
  CtfeValue evaluate(pass_opt_t* opt, errorframe_t* errors, ast_t* expression);
  bool populate_struct_members(pass_opt_t* opt, errorframe_t* errors, CtfeValueStruct* s,
    ast_t* members);
  void left_side_assign(pass_opt_t* opt, errorframe_t* errors, ast_t* left,
    CtfeValue& right_val);
  bool match_eq_element(pass_opt_t* opt, errorframe_t* errors, ast_t* ast_pos,
    CtfeValue &match, ast_t* pattern, ast_t* the_case);
  bool is_operator(pass_opt_t* opt, errorframe_t* errors, ast_t* ast_pos, CtfeValue& left,
    const CtfeValue& right);
  bool contains_valueparamref(ast_t* ast);

  CtfeValue handle_ffi_call(pass_opt_t* opt, errorframe_t* errors, ast_t* ast);
  CtfeValue handle_ffi_ptr_ptr_size(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
    ast_t* return_type, const std::string& ffi_name, const std::vector<CtfeValue>& evaluated_args);
  CtfeValue handle_llvm_ffi(pass_opt_t* opt, errorframe_t* errors,
    ast_t* ast, ast_t* return_type, const std::string& ffi_name,
    const std::vector<CtfeValue>& evaluated_args);

  ast_t* get_reified_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast_pos,
    ast_t* recv_type, ast_t* typeargs, const char* method_name);
  ast_t* get_builtin_type(pass_opt_t* opt, ast_t* ast_pos, const char* type_name);

public:
  CtfeRunner(pass_opt_t* opt);
  ~CtfeRunner();
  bool run(pass_opt_t* opt, ast_t** astp);
  void add_allocated_reference(const CtfeValue& ref);

  ast_t* cache_and_get_built_type(ast_t* ast_type);
};
