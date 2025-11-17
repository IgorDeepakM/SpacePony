#include "genname.h"
#include "../pkg/package.h"
#include "../ast/stringtab.h"
#include "../ast/lexer.h"
#include "../../libponyrt/mem/pool.h"
#include "../ctfe/ctfe_reach.h"
#include "../type/subtype.h"
#include "ponyassert.h"
#include <string.h>

static void types_append(printbuf_t* buf, ast_t* elements, pass_opt_t* opt);

static void type_append(printbuf_t* buf, ast_t* type, bool first, pass_opt_t* opt)
{
  switch(ast_id(type))
  {
    case TK_UNIONTYPE:
    {
      // u3_Arg1_Arg2_Arg3
      printbuf(buf, "u%d", ast_childcount(type));
      types_append(buf, type, opt);
      return;
    }

    case TK_ISECTTYPE:
    {
      // i3_Arg1_Arg2_Arg3
      printbuf(buf, "i%d", ast_childcount(type));
      types_append(buf, type, opt);
      return;
    }

    case TK_TUPLETYPE:
    {
      // t3_Arg1_Arg2_Arg3
      printbuf(buf, "t%d", ast_childcount(type));
      types_append(buf, type, opt);
      return;
    }

    case TK_NOMINAL:
    {
      // pkg_Type[_Arg1_Arg2]_cap
      AST_GET_CHILDREN(type, package, name, typeargs, cap, eph);

      ast_t* def = (ast_t*)ast_data(type);
      ast_t* pkg = ast_nearest(def, TK_PACKAGE);
      const char* pkg_name = package_symbol(pkg);

      if(pkg_name != NULL)
        printbuf(buf, "%s_", pkg_name);

      printbuf(buf, "%s", ast_name(name));
      types_append(buf, typeargs, opt);

      if(!first)
        printbuf(buf, "_%s", ast_get_print(cap));

      return;
    }

    case TK_VALUEFORMALARG:
    {
      ast_t* expr = ast_child(type);
      if(!is_value_formal_arg_literal(expr))
      {
        reach_comptime(opt, &expr);
        if(opt->check.evaluation_error)
        {
          return;
        }
      }
      printbuf(buf, ast_get_print(expr));
      return;
    }

    default: {}
  }

  pony_assert(0);
}

static void types_append(printbuf_t* buf, ast_t* elements, pass_opt_t* opt)
{
  // _Arg1_Arg2
  if(elements == NULL)
    return;

  ast_t* elem = ast_child(elements);

  while(elem != NULL)
  {
    printbuf(buf, "_");
    type_append(buf, elem, false, opt);
    elem = ast_sibling(elem);
  }
}

static const char* stringtab_buf(printbuf_t* buf)
{
  const char* r = stringtab(buf->m);
  printbuf_free(buf);
  return r;
}

static const char* stringtab_two(const char* a, const char* b)
{
  if(a == NULL)
    a = "_";

  pony_assert(b != NULL);
  printbuf_t* buf = printbuf_new();
  printbuf(buf, "%s_%s", a, b);
  return stringtab_buf(buf);
}

const char* genname_type(ast_t* ast, pass_opt_t* opt)
{
  // package_Type[_Arg1_Arg2]
  printbuf_t* buf = printbuf_new();
  type_append(buf, ast, true, opt);
  return stringtab_buf(buf);
}

const char* genname_type_and_cap(ast_t* ast, pass_opt_t* opt)
{
  // package_Type[_Arg1_Arg2]_cap
  printbuf_t* buf = printbuf_new();
  type_append(buf, ast, false, opt);
  return stringtab_buf(buf);
}

const char* genname_alloc(const char* type)
{
  return stringtab_two(type, "Alloc");
}

const char* genname_traitmap(const char* type)
{
  return stringtab_two(type, "Traits");
}

const char* genname_fieldlist(const char* type)
{
  return stringtab_two(type, "Fields");
}

const char* genname_trace(const char* type)
{
  return stringtab_two(type, "Trace");
}

const char* genname_serialise_trace(const char* type)
{
  return stringtab_two(type, "SerialiseTrace");
}

const char* genname_serialise(const char* type)
{
  return stringtab_two(type, "Serialise");
}

const char* genname_deserialise(const char* type)
{
  return stringtab_two(type, "Deserialise");
}

const char* genname_dispatch(const char* type)
{
  return stringtab_two(type, "Dispatch");
}

#if defined(USE_RUNTIME_TRACING)
const char* genname_get_behavior_name(const char* type)
{
  return stringtab_two(type, "GetBehaviourName");
}

const char* genname_behavior_name(const char* type, const char* name)
{
  printbuf_t* buf = printbuf_new();
  printbuf(buf, "%s.%s", type, name);
  return stringtab_buf(buf);
}
#endif

const char* genname_descriptor(const char* type)
{
  return stringtab_two(type, "Desc");
}

const char* genname_instance(const char* type)
{
  return stringtab_two(type, "Inst");
}

const char* genname_fun(token_id cap, const char* name, ast_t* typeargs, pass_opt_t* opt)
{
  // cap_name[_Arg1_Arg2]
  printbuf_t* buf = printbuf_new();
  if(cap == TK_AT)
    printbuf(buf, "%s", name);
  else
    printbuf(buf, "%s_%s", lexer_print(cap), name);
  types_append(buf, typeargs, opt);
  return stringtab_buf(buf);
}

const char* genname_be(const char* name)
{
  return stringtab_two(name, "_send");
}

const char* genname_box(const char* name)
{
  return stringtab_two(name, "Box");
}

const char* genname_unbox(const char* name)
{
  return stringtab_two(name, "Unbox");
}

const char* genname_unsafe(const char* name)
{
  return stringtab_two(name, "unsafe");
}

const char* genname_program_fn(const char* program, const char* name)
{
  return stringtab_two(program, name);
}

const char* genname_type_with_id(const char* type, uint64_t type_id)
{
  printbuf_t* buf = printbuf_new();
  printbuf(buf, "%s_%" PRIu64, type, type_id);
  return stringtab_buf(buf);
}

static void construct_object_hygienic_name(printbuf_t* buf,
  pass_opt_t* opt, ast_t* type)
{
  switch(ast_id(type))
  {
    case TK_NOMINAL:
    {
      const char* type_name = ast_name(ast_childidx(type, 1));
      ast_t* def = (ast_t*)ast_data(type);

      frame_push(&opt->check, ast_nearest(def, TK_PACKAGE));
      const char* s = package_hygienic_id(&opt->check);
      frame_pop(&opt->check);

      printbuf(buf, "%s_%s", type_name, s);
      return;
    }

    case TK_TUPLETYPE:
    {
      printbuf(buf, "%dt", ast_childcount(type));
      ast_t* elem_type = ast_child(type);
      while (elem_type != NULL)
      {
        construct_object_hygienic_name(buf, opt, elem_type);
        elem_type = ast_sibling(elem_type);
      }
      return;
    }

    default:
      pony_assert(false);
      return;
  }

}

// generate a hygienic name for an object of a given type
const char* object_hygienic_name(pass_opt_t* opt, ast_t* type)
{
  printbuf_t* buf = printbuf_new();
  construct_object_hygienic_name(buf, opt, type);
  const char* r = stringtab(buf->m);
  printbuf_free(buf);
  return r;
}
