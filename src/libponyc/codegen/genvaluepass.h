#ifndef CODEGEN_VALUEPASS_H
#define CODEGEN_VALUEPASS_H

#include <platform.h>
#include "codegen.h"

PONY_EXTERN_C_BEGIN

bool is_pass_by_value_lowering_supported(pass_opt_t* opt);

bool is_param_value_lowering_needed(compile_t* c, reach_type_t* pt);

bool is_return_value_lowering_needed(compile_t* c, reach_type_t* pt);

LLVMTypeRef lower_param_value_from_structure_type(compile_t* c, reach_type_t* pt);

LLVMTypeRef lower_return_value_from_structure_type(compile_t* c, reach_type_t* pt);

LLVMValueRef load_lowered_param_value(compile_t* c, LLVMValueRef ptr,
  LLVMTypeRef param_type, reach_type_t* real_type);

LLVMValueRef load_lowered_return_value(compile_t* c, LLVMValueRef ptr,
  LLVMTypeRef return_type, reach_type_t* real_type);

void copy_lowered_param_value(compile_t* c, LLVMValueRef dest_ptr,
  LLVMValueRef param_value, reach_type_t* real_target_type);

void copy_lowered_return_value(compile_t* c, LLVMValueRef dest_ptr,
  LLVMValueRef return_value, reach_type_t* real_target_type);

void apply_function_value_param_attribute(compile_t* c, reach_type_t* pt, LLVMValueRef func,
  LLVMAttributeIndex param_nr);

void apply_call_site_value_param_attribute(compile_t* c, reach_type_t* pt, LLVMValueRef func,
  LLVMAttributeIndex param_nr);

void apply_function_value_return_attribute(compile_t* c, reach_type_t* pt, LLVMValueRef func);

void apply_call_site_value_return_attribute(compile_t* c, reach_type_t* pt,
  LLVMValueRef func);

PONY_EXTERN_C_END

#endif
