#include "inlineasm.h"


extern "C" bool expr_asm(pass_opt_t* opt, ast_t* ast)
{
  ast_t* return_type = ast_childidx(ast, 3);

  ast_settype(ast, return_type);

  return true;
}
