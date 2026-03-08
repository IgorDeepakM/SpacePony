#include "ctfe.h"
#include "literal.h"
#include "../pass/expr.h"
#include "../type/alias.h"
#include "../ctfe/ctfe_runner.h"
#include "ponyassert.h"


extern "C" bool expr_comptime(pass_opt_t* opt, ast_t** astp)
{
  // If we see a compile time expression
  // we first evaluate it then replace this node with the result
  ast_t* ast = *astp;

  // We set this here as we may not yet be able to evaluate the expressions
  // due to references, but we need to know for assignment what we believe
  // to be a compile tim expression.
  pony_assert(ast_id(ast) == TK_COMPTIME);

  ast_t* expression = ast_child(ast);
  ast_t* expr_type = ast_type(expression);

  if(is_typecheck_error(ast_type(expression)))
    return false;

  ast_settype(ast, expr_type);

  return true;
}


extern "C" bool expr_ctfe_run(pass_opt_t* opt, ast_t** astp)
{
  if(ast_id(*astp) == TK_COMPTIME)
  {
    ast_t* child = ast_pop(*astp);
    ast_replace(astp, child);
  }

  if(CtfeRunner::contains_any_typeref(*astp))
  {
    return true;
  }

  CtfeRunner ctfeRunner(opt);
  return ctfeRunner.run(opt, astp);
}