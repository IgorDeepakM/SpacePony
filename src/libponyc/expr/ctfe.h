#ifndef CTFE_H
#define CTFE_H

#include <platform.h>
#include "../ast/ast.h"
#include "../pass/pass.h"

PONY_EXTERN_C_BEGIN

bool expr_comptime(pass_opt_t* opt, ast_t** astp);

PONY_EXTERN_C_END

#endif
