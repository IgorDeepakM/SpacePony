#ifndef CODEGEN_VALUEPASS_H
#define CODEGEN_VALUEPASS_H

#include <platform.h>
#include "codegen.h"

PONY_EXTERN_C_BEGIN

LLVMValueRef gen_inlineasm(compile_t* c, ast_t* ast);

PONY_EXTERN_C_END

#endif
