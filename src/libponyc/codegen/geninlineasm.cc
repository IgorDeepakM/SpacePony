#include "geninlineasm.h"
#include "gentype.h"
#include "genexpr.h"
#include "genopt.h"

#include "llvm_config_begin.h"

#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>

using namespace llvm;

extern "C" LLVMValueRef gen_inlineasm(compile_t* c, ast_t* ast)
{
  LLVMContext *ctx = unwrap(c->context);
  auto builder = unwrap(c->builder);

  ast_t* ast_return_type = ast_childidx(ast, 3);
  ast_t* reified_ret = deferred_reify(c->frame->reify, ast_return_type, c->opt);
  reach_type_t* reach_ret = reach_type(c->reach, reified_ret);
  ast_free_unattached(reified_ret);

  compile_type_t* ret_cp = (compile_type_t*)reach_ret->c_type;
  Type* return_type = unwrap(ret_cp->use_type);

  std::vector<Type*> asm_func_type_params;
  std::vector<Value*> asm_func_value_params;

  ast_t* pos_args = ast_childidx(ast, 4);
  if(ast_id(pos_args) != TK_NONE)
  {
    ast_t* pos_arg = ast_child(pos_args);
    while(pos_arg != NULL)
    {
      ast_t* ast_param = ast_child(pos_arg);
      ast_t* ast_param_type = ast_type(ast_param);
      ast_t* reified_param = deferred_reify(c->frame->reify, ast_param_type, c->opt);
      reach_type_t* reach_param = reach_type(c->reach, reified_param);
      ast_free_unattached(reified_param);
    
      compile_type_t* param_cp = (compile_type_t*)reach_param->c_type;
      asm_func_type_params.push_back(unwrap(param_cp->use_type));

      Value* val = unwrap(gen_expr(c, ast_param));
      asm_func_value_params.push_back(val);

      pos_arg = ast_sibling(pos_arg);
    }
  }

  FunctionType* asm_function = FunctionType::get(return_type, asm_func_type_params, false);

  ast_t* ast_asm_string = ast_child(ast);
  const char* asm_string_c = ast_name(ast_asm_string);
  size_t asm_string_len = ast_name_len(ast_asm_string);
  StringRef asm_string(asm_string_c, asm_string_len);

  ast_t* ast_constraints_string = ast_childidx(ast, 1);
  const char* constraints_string_c = ast_name(ast_constraints_string);
  size_t constraints_string_len = ast_name_len(ast_constraints_string);
  StringRef constraints_string(constraints_string_c, constraints_string_len);

  InlineAsm::AsmDialect dialect = InlineAsm::AsmDialect::AD_ATT;
  if(target_is_x86(c->opt->triple))
  {
    // Default assembly language dialect for x86 is Intel
    dialect = InlineAsm::AsmDialect::AD_Intel;
  }

  InlineAsm *ia = InlineAsm::get(asm_function, asm_string, constraints_string,
    true, false, dialect);
  
  CallInst *call_inst = builder->CreateCall(ia, asm_func_value_params);

  return wrap(call_inst);
}
