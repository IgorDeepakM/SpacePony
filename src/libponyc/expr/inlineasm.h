#ifndef EXPR_INLINEASM_H
#define EXPR_INLINEASM_H

#include <platform.h>
#include "../ast/ast.h"
#include "../pass/pass.h"

PONY_EXTERN_C_BEGIN

bool expr_asm(pass_opt_t* opt, ast_t* ast);

PONY_EXTERN_C_END

#endif
