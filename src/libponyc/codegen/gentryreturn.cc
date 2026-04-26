#include "gentryreturn.h"
#include "gentype.h"
#include "genvaluepass.h"
#include "gencall.h"
#include "genexpr.h"
#include "genfun.h"
#include "../type/subtype.h"
#include "../../libponyrt/mem/pool.h"
#include "ponyassert.h"

#include <cstring>


extern "C" TryReturnInfo init_try_return_info()
{
  TryReturnInfo ret;
  ret.return_type = TRYRETURNTYPE_NONE;
  ret.wrapped_type = NULL;
  ret.unwrapped_type = NULL;
  return ret;
}


extern "C" LLVMTypeRef generate_try_return_type(compile_t* c, TryReturnInfo* try_return_info,
  reach_type_t* type)
{
  compile_type_t* ret_c_t = (compile_type_t*)type->c_type;
  LLVMTypeKind return_type_kind = LLVMGetTypeKind(ret_c_t->use_type);
  LLVMTypeRef ret = NULL;

  try_return_info->unwrapped_type = type;

  if(is_none(type->ast))
  {
    reach_type_t* bool_reach_type = reach_type_name(c->reach, "Bool");
    compile_type_t* bool_c_t = (compile_type_t*)bool_reach_type->c_type;
    try_return_info->return_type = TRYRETURNTYPE_BOOL;
    try_return_info->wrapped_type = bool_c_t->use_type;
    ret = bool_c_t->use_type;
  }
  else if(return_type_kind == LLVMPointerTypeKind ||
          is_pointer(type->ast) ||
          is_nullable_pointer(type->ast))
  {
    try_return_info->wrapped_type = ret_c_t->use_type;
    try_return_info->return_type = TRYRETURNTYPE_POINTER;
    ret = ret_c_t->use_type;
  }
  else
  {
    try_return_info->return_type = TRYRETURNTYPE_OTHER;

    reach_type_t* bool_reach_type = reach_type_name(c->reach, "Bool");
    LLVMTypeRef bool_type = ((compile_type_t*)bool_reach_type->c_type)->mem_type;

    LLVMTypeRef elements[2];
    elements[0] = ret_c_t->mem_type;
    elements[1] = bool_type;
    ret = LLVMStructTypeInContext(c->context, elements, 2, false);
    try_return_info->wrapped_type = ret;
  }

  return ret;
}


LLVMTypeRef get_try_return_wrapped_type(TryReturnInfo* try_return_info)
{
  return try_return_info->wrapped_type;
}


LLVMTypeRef get_try_return_unwrapped_type(TryReturnInfo* try_return_info)
{
  return ((compile_type_t*)try_return_info->unwrapped_type->c_type)->use_type;
}


LLVMValueRef unwrap_try_return_bool(compile_t* c, TryReturnInfo* try_return_info,
  LLVMValueRef value)
{
  LLVMValueRef bool_expr = NULL;

  switch(try_return_info->return_type)
  {
    case TRYRETURNTYPE_BOOL:
      bool_expr = LLVMBuildNot(c->builder, value, "");
      break;

    case TRYRETURNTYPE_POINTER:
      bool_expr = LLVMBuildIsNull(c->builder, value, "");
      break;

    case TRYRETURNTYPE_OTHER:
    {
      bool_expr = LLVMBuildExtractValue(c->builder, value, 1, "");

      reach_type_t* bool_reach_type = reach_type_name(c->reach, "Bool");
      compile_type_t* bool_c_t = (compile_type_t*)bool_reach_type->c_type;

      if(LLVMGetIntTypeWidth(bool_c_t->use_type) < LLVMGetIntTypeWidth(bool_c_t->mem_type))
      {
        bool_expr = LLVMBuildTrunc(c->builder, bool_expr, bool_c_t->use_type, "");
      }

      bool_expr = LLVMBuildNot(c->builder, bool_expr, "");

      break;
    }

    default:
      pony_assert(false);
      break;
  }

  return bool_expr;
}


LLVMValueRef unwrap_try_return_value(compile_t* c, TryReturnInfo* try_return_info,
  LLVMValueRef value)
{
  LLVMValueRef result = NULL;

  switch(try_return_info->return_type)
  {
    case TRYRETURNTYPE_BOOL:
      result = c->none_instance;
      break;

    case TRYRETURNTYPE_POINTER:
      result = value;
      break;

    case TRYRETURNTYPE_OTHER:
    {
      result = LLVMBuildExtractValue(c->builder, value, 0, "");
      LLVMTypeRef use_type = ((compile_type_t*)try_return_info->unwrapped_type->c_type)->use_type;
      result = gen_assign_cast(c, use_type, result, try_return_info->unwrapped_type->ast_cap);
      break;
    }

    default:
      break;
  }

  return result;
}


extern "C" LLVMValueRef unwrap_try_return_value_jump_if_error(compile_t* c, TryReturnInfo* try_return_info,
  LLVMValueRef value)
{
  LLVMValueRef bool_expr = unwrap_try_return_bool(c, try_return_info, value);

  LLVMBasicBlockRef error_block = codegen_block(c, "partial_call_error");
  LLVMBasicBlockRef continue_block = codegen_block(c, "partial_call_continue");
  LLVMBuildCondBr(c->builder, bool_expr, error_block, continue_block);

  LLVMPositionBuilderAtEnd(c->builder, error_block);

  if(c->frame->try_else_target != NULL)
  {
    // Inside a try block: branch to the error handler.
    LLVMBuildBr(c->builder, c->frame->try_else_target);
  }
  else
  {
    reach_method_t* r_m = c->frame->m;
    compile_method_t* c_m = (compile_method_t*)r_m->c_method;

    pony_assert(c_m->try_return_info.return_type != TRYRETURNTYPE_NONE);

    LLVMValueRef error_ret = wrap_try_return_error(c, &c_m->try_return_info);
    genfun_build_ret(c, error_ret);
  }

  LLVMMoveBasicBlockAfter(continue_block, LLVMGetInsertBlock(c->builder));
  LLVMPositionBuilderAtEnd(c->builder, continue_block);

  return unwrap_try_return_value(c, try_return_info, value);
}


extern "C" LLVMValueRef wrap_try_return_success(compile_t* c, TryReturnInfo* try_return_info,
  LLVMValueRef value)
{
  LLVMValueRef ret = NULL;
  LLVMValueRef tuple = NULL;

  switch(try_return_info->return_type)
  {
    case TRYRETURNTYPE_BOOL:
    {
      reach_type_t* bool_reach_type = reach_type_name(c->reach, "Bool");
      compile_type_t* bool_c_t = (compile_type_t*)bool_reach_type->c_type;

      ret = LLVMConstInt(bool_c_t->use_type, 1, false);
      break;
    }

    case TRYRETURNTYPE_POINTER:
    {
      ret = value;
      break;
    }

    case TRYRETURNTYPE_OTHER:
    {
      reach_type_t* bool_type = reach_type_name(c->reach, "Bool");
      compile_type_t* bool_c_t = (compile_type_t*)bool_type->c_type;

      LLVMValueRef tuple = LLVMGetUndef(try_return_info->wrapped_type);
      LLVMTypeRef mem_type = ((compile_type_t*)try_return_info->unwrapped_type->c_type)->mem_type;
      LLVMValueRef value_mem = gen_assign_cast(c, mem_type, value, try_return_info->unwrapped_type->ast_cap);
      tuple = LLVMBuildInsertValue(c->builder, tuple, value_mem, 0, "");
      ret = LLVMBuildInsertValue(c->builder, tuple, LLVMConstInt(bool_c_t->mem_type, 1, false), 1, "");
      break;
    }

    default:
      pony_assert(false);
      break;
  }

  return ret;
}

extern "C" LLVMValueRef wrap_try_return_error(compile_t* c, TryReturnInfo* try_return_info)
{
  LLVMValueRef ret = NULL;
  LLVMValueRef tuple = NULL;

  switch(try_return_info->return_type)
  {
    case TRYRETURNTYPE_BOOL:
    {
      reach_type_t* bool_reach_type = reach_type_name(c->reach, "Bool");
      compile_type_t* bool_c_t = (compile_type_t*)bool_reach_type->c_type;

      ret = LLVMConstInt(bool_c_t->use_type, 0, false);
      break;
    }

    case TRYRETURNTYPE_POINTER:
    {
      ret = LLVMConstNull(c->ptr);
      break;
    }

    case TRYRETURNTYPE_OTHER:
    {
      reach_type_t* bool_type = reach_type_name(c->reach, "Bool");
      compile_type_t* bool_c_t = (compile_type_t*)bool_type->c_type;

      LLVMValueRef tuple = LLVMGetUndef(try_return_info->wrapped_type);
      LLVMTypeRef mem_type = ((compile_type_t*)try_return_info->unwrapped_type->c_type)->mem_type;
      tuple = LLVMBuildInsertValue(c->builder, tuple, LLVMConstNull(mem_type), 0, "");
      ret = LLVMBuildInsertValue(c->builder, tuple, LLVMConstInt(bool_c_t->mem_type, 0, false), 1, "");
      break;
    }

    default:
      pony_assert(false);
      break;
  }

  return ret;
}