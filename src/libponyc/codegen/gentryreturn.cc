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
  ret.t = NULL;
  ret.return_type = TRYRETURNTYPE_NONE;
  ret.c_ffi = false;
  ret.return_by_value = false;
  ret.return_value_lowered = false;
  return ret;
}


extern "C" void delete_try_return_info(TryReturnInfo* try_return_info)
{
  if(try_return_info->t != NULL)
  {
    if(try_return_info->t->c_type != NULL)
    {
      POOL_FREE(compile_type_t, try_return_info->t->c_type);
      try_return_info->t->c_type = NULL;
    }

    POOL_FREE(reach_type_t, try_return_info->t);
    try_return_info->t = NULL;
  }
}


extern "C" LLVMTypeRef generate_try_return_type(compile_t* c, TryReturnInfo* try_return_info,
  reach_type_t* type, LLVMTypeRef use_type, bool c_ffi, bool return_by_value)
{
  compile_type_t* ret_c_t = (compile_type_t*)type->c_type;
  LLVMTypeKind return_type_kind = LLVMGetTypeKind(ret_c_t->use_type);

  // Create a fake reach_type_t in order to get it through the pass by value lowering
  reach_type_t* fake_reach_type = POOL_ALLOC(reach_type_t);
  memset(fake_reach_type, 0, sizeof(reach_type_t));

  compile_type_t* fake_c_t = POOL_ALLOC(compile_type_t);
  memset(fake_c_t, 0, sizeof(compile_type_t));
  fake_c_t->free_fn = NULL;
  fake_reach_type->c_type = (compile_opaque_t*)fake_c_t;

  try_return_info->t = fake_reach_type;
  try_return_info->c_ffi = c_ffi;

  if(is_none(type->ast))
  {
    reach_type_t* bool_reach_type = reach_type_name(c->reach, "Bool");
    compile_type_t* bool_c_t = (compile_type_t*)bool_reach_type->c_type;

    fake_c_t->use_type = bool_c_t->use_type;
    fake_c_t->mem_type = bool_c_t->use_type;
    fake_c_t->structure_ptr = NULL;
    fake_c_t->structure = NULL;
    fake_c_t->abi_size = LLVMABISizeOfType(c->target_data, bool_c_t->use_type);

    try_return_info->return_type = TRYRETURNTYPE_BOOL;
  }
  else if((return_type_kind == LLVMPointerTypeKind ||
           is_pointer(type->ast) ||
           is_nullable_pointer(type->ast)) && !return_by_value)
  {
    compile_type_t* p_c_t = (compile_type_t*)type->c_type;

    fake_c_t->use_type = p_c_t->use_type;
    fake_c_t->mem_type = p_c_t->mem_type;
    fake_c_t->structure_ptr = NULL;
    fake_c_t->structure = NULL;
    fake_c_t->abi_size = LLVMABISizeOfType(c->target_data, c->ptr);
    
    try_return_info->return_type = TRYRETURNTYPE_POINTER;
  }
  else
  {
    compile_type_t* p_c_t = (compile_type_t*)type->c_type;

    LLVMTypeRef ret_type = NULL;
    if(return_by_value)
    {
      ret_type = p_c_t->structure;
      try_return_info->return_type = TRYRETURNTYPE_STRUCT;
    }
    else
    {
      ret_type = p_c_t->mem_type;
      try_return_info->return_type = TRYRETURNTYPE_OTHER;
    }

    if(c_ffi)
    {
      try_return_info->return_by_value = true;
    }
    
    reach_type_t* bool_reach_type = reach_type_name(c->reach, "Bool");
    LLVMTypeRef bool_type = ((compile_type_t*)bool_reach_type->c_type)->mem_type;

    LLVMTypeRef elements[2];
    elements[0] = ret_type;
    elements[1] = bool_type;
    fake_c_t->structure = LLVMStructTypeInContext(c->context, elements, 2, false);

    fake_c_t->use_type = c->ptr;
    fake_c_t->mem_type = c->ptr;
    fake_c_t->structure_ptr = c->ptr;
    fake_c_t->abi_size = LLVMABISizeOfType(c->target_data, fake_c_t->structure);

    if(c_ffi)
    {
      if(is_return_value_lowering_needed(c, fake_reach_type))
      {
        fake_c_t->use_type = lower_return_value_from_structure_type(c, fake_reach_type);
        try_return_info->return_value_lowered = true;
      }
    }
    else
    {
      fake_c_t->use_type = fake_c_t->structure;
      fake_c_t->mem_type = fake_c_t->structure;
    }
  }

  if(try_return_info->return_by_value && !try_return_info->return_value_lowered)
  {
    return c->void_type;
  }
  else
  {
    return fake_c_t->use_type;
  }
}


extern "C" LLVMValueRef unwrap_try_return_value(compile_t* c, TryReturnInfo* try_return_info,
  LLVMValueRef value, LLVMValueRef wrapper_ptr, reach_type_t* return_reach_type)
{
  LLVMValueRef bool_expr = NULL;
  LLVMValueRef tuple = NULL;

  switch(try_return_info->return_type)
  {
    case TRYRETURNTYPE_BOOL:
    {
      bool_expr = LLVMBuildNot(c->builder, value, "");
      break;
    }

    case TRYRETURNTYPE_POINTER:
      bool_expr = LLVMBuildIsNull(c->builder, value, "");
      break;

    case TRYRETURNTYPE_STRUCT:
    case TRYRETURNTYPE_OTHER:
    {
      compile_type_t* wrapper_c_t = (compile_type_t*)try_return_info->t->c_type;

      if(try_return_info->c_ffi)
      {
        if(!try_return_info->return_value_lowered)
        {
          tuple = load_lowered_return_value_from_ptr(c, wrapper_ptr, wrapper_c_t->structure,
            try_return_info->t);
          bool_expr = LLVMBuildExtractValue(c->builder, tuple, 1, "");
        }
        else
        {
          LLVMValueRef tmp_alloc = LLVMBuildAlloca(c->builder, wrapper_c_t->structure, "");
          copy_lowered_return_value_to_ptr(c, tmp_alloc, value, try_return_info->t);
          tuple = LLVMBuildLoad2(c->builder, wrapper_c_t->structure, tmp_alloc, "");
          bool_expr = LLVMBuildExtractValue(c->builder, tuple, 1, "");
        }
      }
      else
      {
        bool_expr = LLVMBuildExtractValue(c->builder, value, 1, "");
      }

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

  LLVMBasicBlockRef error_block = codegen_block(c, "call_error");
  LLVMBasicBlockRef continue_block = codegen_block(c, "call_continue");
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

    LLVMValueRef error_ret = wrap_try_return_error(c, &c_m->try_return_info, r_m->result);
    genfun_build_ret(c, error_ret);
  }


  LLVMPositionBuilderAtEnd(c->builder, continue_block);

  LLVMValueRef result = NULL;

  switch(try_return_info->return_type)
  {
    case TRYRETURNTYPE_BOOL:
      result = c->none_instance;
      break;

    case TRYRETURNTYPE_POINTER:
      result = value;
      break;

    case TRYRETURNTYPE_STRUCT:
    case TRYRETURNTYPE_OTHER:
    {
      if(try_return_info->c_ffi)
      {
        result = LLVMBuildExtractValue(c->builder, tuple, 0, "");
      }
      else
      {
        result = LLVMBuildExtractValue(c->builder, value, 0, "");
      }

      compile_type_t* ret_c_t = (compile_type_t*)return_reach_type->c_type;
      result = gen_assign_cast(c, ret_c_t->use_type, result, return_reach_type->ast);

      break;
    }

    default:
      break;
  }

  return result;
}


extern "C" LLVMValueRef wrap_try_return_success(compile_t* c, TryReturnInfo* try_return_info,
  LLVMValueRef value, reach_type_t* return_reach_type)
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

    case TRYRETURNTYPE_STRUCT:
    case TRYRETURNTYPE_OTHER:
    {
      compile_type_t* wrapper_c_t = (compile_type_t*)try_return_info->t->c_type;
      compile_type_t* ret_c_t = (compile_type_t*)return_reach_type->c_type;

      reach_type_t* bool_type = reach_type_name(c->reach, "Bool");
      compile_type_t* bool_c_t = (compile_type_t*)bool_type->c_type;

      LLVMValueRef tuple = LLVMGetUndef(wrapper_c_t->structure);
      tuple = LLVMBuildInsertValue(c->builder, tuple,
        gen_assign_cast(c, ret_c_t->mem_type, value, return_reach_type->ast), 0, "");
      ret = LLVMBuildInsertValue(c->builder, tuple, LLVMConstInt(bool_c_t->mem_type, 1, false), 1, "");
      break;
    }

    default:
      pony_assert(false);
      break;
  }

  return ret;
}

extern "C" LLVMValueRef wrap_try_return_error(compile_t* c, TryReturnInfo* try_return_info,
  reach_type_t* return_reach_type)
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

    case TRYRETURNTYPE_STRUCT:
    case TRYRETURNTYPE_OTHER:
    {
      compile_type_t* wrapper_c_t = (compile_type_t*)try_return_info->t->c_type;
      compile_type_t* ret_c_t = (compile_type_t*)return_reach_type->c_type;

      reach_type_t* bool_type = reach_type_name(c->reach, "Bool");
      compile_type_t* bool_c_t = (compile_type_t*)bool_type->c_type;

      LLVMValueRef tuple = LLVMGetUndef(wrapper_c_t->structure);
      tuple = LLVMBuildInsertValue(c->builder, tuple, LLVMConstNull(ret_c_t->mem_type), 0, "");
      ret = LLVMBuildInsertValue(c->builder, tuple, LLVMConstInt(bool_c_t->mem_type, 0, false), 1, "");
      break;
    }

    default:
      pony_assert(false);
      break;
  }

  return ret;
}