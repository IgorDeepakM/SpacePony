#ifndef CTFE_REACH_H
#define CTFE_REACH_H

#include <platform.h>
#include "../ast/ast.h"
#include "../pass/pass.h"
#include "../type/reify.h"

PONY_EXTERN_C_BEGIN

bool reach_comptime(pass_opt_t* opt, ast_t** astp, deferred_reification_t* reify);

PONY_EXTERN_C_END

#endif
