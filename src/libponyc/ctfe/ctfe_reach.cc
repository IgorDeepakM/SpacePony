#include "ctfe_reach.h"
#include "ctfe_runner.h"

#include "../ast/ast.h"


extern "C" bool reach_comptime(pass_opt_t* opt, ast_t** astp)
{
  ast_unfreeze(ast_parent(*astp));

  if(ast_id(*astp) == TK_COMPTIME)
  {
    ast_t* child = ast_pop(*astp);
    ast_replace(astp, child);
  }

  CtfeRunner ctfeRunner(opt);
  bool ret = ctfeRunner.run(opt, astp);
  ast_freeze(ast_parent(*astp));
  return ret;
}