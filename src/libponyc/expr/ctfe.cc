#include "ctfe.h"
#include "literal.h"
#include "../pass/expr.h"
#include "../type/alias.h"
#include "ponyassert.h"


extern "C" bool expr_comptime(pass_opt_t* opt, ast_t** astp)
{
  // If we see a compile time expression
  // we first evaluate it then replace this node with the result
  ast_t* ast = *astp;

  // We set this here as we may not yet be able to evaluate the expressions
  // due to references, but we need to know for assignment what we believe
  // to be a compile tim expression.
  assert(ast_id(ast) == TK_COMPTIME);

  ast_t* expression = ast_child(ast);
  ast_t* expr_type = ast_type(expression);

  if(is_typecheck_error(ast_type(expression)))
    return false;

  // See if we can recover the expression to val capability
  if(!is_type_literal(expr_type))
  {
    expr_type = recover_type(expr_type, TK_VAL);
    if(expr_type == NULL)
    {
      ast_error(opt->check.errors, expression,
        "can't recover compile-time object to val capability");
      return false;
    }
  }
  ast_settype(ast, expr_type);

  return true;
}