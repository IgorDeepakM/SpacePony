#ifndef CODEGEN_VALUEPASS_H
#define CODEGEN_VALUEPASS_H

#include <platform.h>
#include "codegen.h"

PONY_EXTERN_C_BEGIN

bool is_pass_by_value_lowering_supported(compile_t* c);

bool is_param_value_lowering_needed(compile_t* c, reach_type_t* pt);

bool is_return_value_lowering_needed(compile_t* c, reach_type_t* pt);

LLVMTypeRef lower_param_value_from_structure_type(compile_t* c, reach_type_t* pt);

LLVMTypeRef lower_return_value_from_structure_type(compile_t* c, reach_type_t* pt);

void copy_lowered_return_value(compile_t* c, LLVMValueRef destination,
  LLVMValueRef return_value);

void apply_function_value_param_attribute(compile_t* c, reach_type_t* pt, LLVMValueRef func,
  LLVMAttributeIndex param_nr);

void apply_call_site_value_param_attribute(compile_t* c, reach_type_t* pt, LLVMValueRef func,
  LLVMAttributeIndex param_nr);

void apply_function_value_return_attribute(compile_t* c, reach_type_t* pt, LLVMValueRef func);

void apply_call_site_value_return_attribute(compile_t* c, reach_type_t* pt,
  LLVMValueRef func);

PONY_EXTERN_C_END

#endif
