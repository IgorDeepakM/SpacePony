#include "gentryreturn.h"
#include "gentype.h"
#include "genvaluepass.h"
#include "gencall.h"
#include "../type/subtype.h"
#include "../reach/reach.h"
#include "../../libponyrt/mem/pool.h"
#include "ponyassert.h"


#include "llvm_config_begin.h"


extern "C" TryReturnInfo init_try_return_info()
{
  TryReturnInfo ret;
  ret.t = NULL;
  ret.return_type = TRYRETURNTYPE_NONE;
  ret.return_by_value = false;
  ret.return_value_lowered = false;
  return ret;
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

    try_return_info->return_by_value = true;
    
    reach_type_t* bool_reach_type = reach_type_name(c->reach, "Bool");
    LLVMTypeRef bool_type = ((compile_type_t*)bool_reach_type->c_type)->use_type;

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
      bool_expr = LLVMBuildNot(c->builder, value, "");
      break;

    case TRYRETURNTYPE_POINTER:
      bool_expr = LLVMBuildIsNull(c->builder, value, "");
      break;

    case TRYRETURNTYPE_STRUCT:
    case TRYRETURNTYPE_OTHER:
    {
      compile_type_t* wrapper_c_t = (compile_type_t*)try_return_info->t->c_type;

      if(!try_return_info->return_value_lowered)
      {
        tuple = load_lowered_return_value_from_ptr(c, wrapper_ptr, wrapper_c_t->structure,
          try_return_info->t);
        bool_expr = LLVMBuildExtractValue(c->builder, tuple, 1, "");
        bool_expr = LLVMBuildNot(c->builder, bool_expr, "");
      }
      else
      {
        LLVMValueRef tmp_alloc = LLVMBuildAlloca(c->builder, wrapper_c_t->structure, "");
        copy_lowered_return_value_to_ptr(c, tmp_alloc, value, try_return_info->t);
        tuple = LLVMBuildLoad2(c->builder, wrapper_c_t->structure, tmp_alloc, "");
        bool_expr = LLVMBuildExtractValue(c->builder, tuple, 1, "");
        bool_expr = LLVMBuildNot(c->builder, bool_expr, "");
      }

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

  gencall_error(c);

  LLVMPositionBuilderAtEnd(c->builder, continue_block);


  LLVMValueRef result = GEN_NOTNEEDED;

  switch(try_return_info->return_type)
  {
    case TRYRETURNTYPE_POINTER:
      result = value;
      break;

    case TRYRETURNTYPE_STRUCT:
    case TRYRETURNTYPE_OTHER:
    {
      result = LLVMBuildExtractValue(c->builder, tuple, 0, "");
      break;
    }

    default:
      break;
  }

  return result;
}