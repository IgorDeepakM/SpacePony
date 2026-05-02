#ifndef EXPR2_H
#define EXPR2_H

#include <platform.h>
#include "../ast/ast.h"
#include "../pass/pass.h"


PONY_EXTERN_C_BEGIN

void delete_expr2_data(pass_opt_t* opt);

void add_to_expr2(pass_opt_t* opt, ast_t* a, ast_t* b);

PONY_EXTERN_C_END

#endif
