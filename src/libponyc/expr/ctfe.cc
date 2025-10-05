#include "ctfe.h"
#include "../ctfe/ctfe_runner.h"
#include "literal.h"


extern "C" bool expr_comptime(pass_opt_t* opt, ast_t** astp)
{
  CtfeRunner ctfeRunner(opt);
  return ctfeRunner.run(opt, astp);
}