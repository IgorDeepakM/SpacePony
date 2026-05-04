#ifndef VALUEFORMALEQ_H
#define VALUEFORMALEQ_H

#include <platform.h>
#include "../ast/ast.h"
#include "../pass/pass.h"
#include "../type/reify.h"

typedef struct typecheck_t typecheck_t;

PONY_EXTERN_C_BEGIN

void delete_value_formal_arg_eq(pass_opt_t* opt);

size_t get_comptime_id(pass_opt_t* opt);

void add_value_formal_arg_expr(pass_opt_t* opt, ast_t* a, ast_t* b);

bool check_value_formal_eq_list(pass_opt_t* opt, ast_t* node, size_t comptime_id,
  deferred_reification_t* dr);

PONY_EXTERN_C_END

#endif
