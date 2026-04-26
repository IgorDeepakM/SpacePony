#ifndef CODEGEN_TRYRETURN_H
#define CODEGEN_TRYRETURN_H

#include <platform.h>
#include "../reach/reach.h"
#include "codegen.h"

PONY_EXTERN_C_BEGIN

typedef enum
{
  TRYRETURNTYPE_NONE,
  TRYRETURNTYPE_BOOL,
  TRYRETURNTYPE_POINTER,
  TRYRETURNTYPE_OTHER
}TryReturnType;


typedef struct TryReturnInfo_s
{
  TryReturnType return_type;
  LLVMTypeRef wrapped_type;
  reach_type_t* unwrapped_type;
}TryReturnInfo;


TryReturnInfo init_try_return_info();

LLVMTypeRef generate_try_return_type(compile_t* c, TryReturnInfo* try_return_info,
  reach_type_t* type);

LLVMTypeRef get_try_return_wrapped_type(TryReturnInfo* try_return_info);

LLVMTypeRef get_try_return_unwrapped_type(TryReturnInfo* try_return_info);

LLVMTypeRef get_try_return_unwrapped_type(TryReturnInfo* try_return_info);

LLVMValueRef unwrap_try_return_bool(compile_t* c, TryReturnInfo* try_return_info,
  LLVMValueRef value);

LLVMValueRef unwrap_try_return_value(compile_t* c, TryReturnInfo* try_return_info,
  LLVMValueRef value);

LLVMValueRef unwrap_try_return_value_jump_if_error(compile_t* c, TryReturnInfo* try_return_info,
  LLVMValueRef value);

LLVMValueRef wrap_try_return_success(compile_t* c, TryReturnInfo* try_return_info,
  LLVMValueRef value);

LLVMValueRef wrap_try_return_error(compile_t* c, TryReturnInfo* try_return_info);

PONY_EXTERN_C_END

#endif
