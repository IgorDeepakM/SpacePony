#ifndef CODEGEN_TRYRETURN_H
#define CODEGEN_TRYRETURN_H

#include <platform.h>
#include "codegen.h"

PONY_EXTERN_C_BEGIN

enum TryReturnType
{
  TRYRETURNTYPE_NONE,
  TRYRETURNTYPE_BOOL,
  TRYRETURNTYPE_POINTER,
  TRYRETURNTYPE_STRUCT,
  TRYRETURNTYPE_OTHER
};


typedef struct
{
  reach_type_t* t;
  TryReturnType return_type;
  bool return_by_value;
  bool return_value_lowered;
}TryReturnInfo;


TryReturnInfo init_try_return_info();

LLVMTypeRef generate_try_return_type(compile_t* c, TryReturnInfo* try_return_info,
  reach_type_t* type, LLVMTypeRef use_type, bool c_ffi, bool return_by_value);

LLVMValueRef unwrap_try_return_value(compile_t* c, TryReturnInfo* try_return_info,
  LLVMValueRef value, LLVMValueRef wrapper_ptr, reach_type_t* return_reach_type);

PONY_EXTERN_C_END

#endif
