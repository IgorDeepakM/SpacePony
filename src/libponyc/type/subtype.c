#include "subtype.h"
#include "alias.h"
#include "assemble.h"
#include "cap.h"
#include "matchtype.h"
#include "reify.h"
#include "subtype_cache.h"
#include "type_assume.h"
#include "typealias.h"
#include "typeparam.h"
#include "viewpoint.h"
#include "valueformaleq.h"
#include "../ast/astbuild.h"
#include "../expr/literal.h"
#include "ponyassert.h"
#include "pony_defines.h"
#include <limits.h>
#include <string.h>


typedef enum
{
  CHECK_CAP_SUB,
  CHECK_CAP_EQ,
  CHECK_CAP_BOUND,
  CHECK_CAP_IGNORE
} check_cap_t;


static bool is_eq_typeargs(ast_t* a, ast_t* b, errorframe_t* errorf,
  pass_opt_t* opt);
static bool is_x_sub_typeparam(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt);
static bool is_x_sub_arrow(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt);
static bool is_isect_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt);
static bool is_nominal_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt);
static bool is_typeparam_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt);
static bool is_x_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt);

static void struct_cant_be_x(ast_t* sub, ast_t* super, errorframe_t* errorf,
  const char* entity, pass_opt_t* opt)
{
  if(errorf == NULL)
    return;

  ast_error_frame(errorf, sub,
    "%s is not a subtype of %s: a struct can't be a subtype of %s",
    ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab), entity);
}

static ast_t* get_valueformalarg_def(ast_t* ast)
{
  if(ast_id(ast) != TK_VALUEFORMALPARAMREF && ast_id(ast) != TK_VALUEFORMALARG)
  {
    return NULL;
  }

  ast_t* parent = ast_parent(ast);
  ast_t* parent2 = ast_parent(parent);

  ast_t* def = NULL;

  switch(ast_id(parent2))
  {
    case TK_NOMINAL:
    case TK_TYPEREF:
      def = (ast_t*)ast_data(parent2);
      break;

    default:
      pony_assert(false);
      break;
  }

  size_t index = ast_index(ast);

  ast_t* type_params = ast_childidx(def, 1);

  ast_t* typeparam = ast_childidx(type_params, index);

  return typeparam;
}

static bool is_sub_cap_and_eph(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  (void)opt;

  ast_t* sub_cap = cap_fetch(sub);
  ast_t* sub_eph = ast_sibling(sub_cap);
  ast_t* super_cap = cap_fetch(super);
  ast_t* super_eph = ast_sibling(super_cap);

  switch(check_cap)
  {
    case CHECK_CAP_IGNORE:
      return true;

    case CHECK_CAP_SUB:
    {
      if(is_cap_sub_cap(ast_id(sub_cap), ast_id(sub_eph), ast_id(super_cap),
        ast_id(super_eph)))
        return true;

      if(errorf != NULL)
      {
        ast_error_frame(errorf, sub,
          "%s is not a subtype of %s: %s%s is not a subcap of %s%s",
          ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab),
          ast_print_type(sub_cap, opt->strtab), ast_print_type(sub_eph, opt->strtab),
          ast_print_type(super_cap, opt->strtab), ast_print_type(super_eph, opt->strtab));

        if(is_cap_sub_cap(ast_id(sub_cap), TK_EPHEMERAL, ast_id(super_cap),
          ast_id(super_eph)))
          ast_error_frame(errorf, sub_cap,
            "this would be possible if the subcap were more ephemeral. "
            "Perhaps you meant to consume this variable");

        const char* generic = NULL;
        switch(ast_id(super_cap))
        {
          case TK_CAP_READ:  generic = "ref, val, box"; break;
          case TK_CAP_SEND:  generic = "iso, val, tag"; break;
          case TK_CAP_SHARE: generic = "val, tag"; break;
          case TK_CAP_ANY:   generic = "iso, trn, ref, val, box, tag"; break;
          default: break;
        }

        if(generic != NULL)
          ast_error_frame(errorf, sub_cap,
            "%s is a generic capability standing for {%s}; a capability is a "
            "subcap of it only if it is a subcap of every one of those",
            ast_print_type(super_cap, opt->strtab), generic);
      }

      return false;
    }

    case CHECK_CAP_EQ:
    {
      if(is_cap_sub_cap_constraint(ast_id(sub_cap), ast_id(sub_eph),
        ast_id(super_cap), ast_id(super_eph)))
        return true;

      if(errorf != NULL)
      {
        ast_error_frame(errorf, sub,
          "%s is not in constraint %s: %s%s is not in constraint %s%s",
          ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab),
          ast_print_type(sub_cap, opt->strtab), ast_print_type(sub_eph, opt->strtab),
          ast_print_type(super_cap, opt->strtab), ast_print_type(super_eph, opt->strtab));
      }

      return false;
    }

    case CHECK_CAP_BOUND:
    {
      if(is_cap_sub_cap_bound(ast_id(sub_cap), ast_id(sub_eph),
        ast_id(super_cap), ast_id(super_eph)))
        return true;

      if(errorf != NULL)
      {
        ast_error_frame(errorf, sub,
          "%s is not a bound subtype of %s: %s%s is not a bound subcap of %s%s",
          ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab),
          ast_print_type(sub_cap, opt->strtab), ast_print_type(sub_eph, opt->strtab),
          ast_print_type(super_cap, opt->strtab), ast_print_type(super_eph, opt->strtab));
      }

      return false;
    }
  }

  pony_assert(0);
  return false;
}

bool is_literal_equal(ast_t* a, ast_t* b, pass_opt_t* opt, bool allow_eq_list)
{
  if(!is_value_formal_arg_literal(a) || !is_value_formal_arg_literal(b))
  {
    // if we have an expression that is a not a literal but a comptime expression
    // we need to put this check on a list and run it in the reach pass.
    if(allow_eq_list)
    {
      add_value_formal_arg_expr(opt, a, b);
      return true;
    }
    else
    {
      return false;
    }
  }

  switch(ast_id(a))
  {
    case TK_INT:
      if (ast_id(b) == TK_INT)
      {
        lexint_t* lex_a = ast_int(a);
        lexint_t* lex_b = ast_int(b);
        if (lexint_cmp(lex_a, lex_b) == 0)
        {
          return true;
        }
      }
      break;

    case TK_FLOAT:
      if (ast_id(b) == TK_FLOAT)
      {
        double a_float = ast_float(a);
        double b_float = ast_float(b);
        if (a_float == b_float)
        {
          return true;
        }
      }
      break;

    case TK_TRUE:
      if (ast_id(b) == TK_TRUE)
      {
        return true;
      }
      break;

    case TK_FALSE:
      if (ast_id(b) == TK_FALSE)
      {
        return true;
      }
      break;

    case TK_STRING:
      if (ast_id(b) == TK_STRING)
      {
        if (strcmp(ast_name(a), ast_name(b)) == 0)
        {
          return true;
        }
      }
      break;

    case TK_CONSTANT_OBJECT:
      if(ast_id(b) == TK_CONSTANT_OBJECT)
      {
        uint64_t a_hash = ast_hash(a);
        uint64_t b_hash = ast_hash(b);

        if(a_hash == b_hash)
        {
          return true;
        }
        else
        {
          // Workaround for lambdas. Each lambda instance is sugared
          // to get its own autogenerated type with autogenerated type
          // name (like $1$1). This leads that lambdas that are completely
          // equal have different hashes. Currently we use the generated type
          // and ast_hash when TK_ID has ast_data it uses the nice name instead
          // that is equal to the lambda function signature.
          // However, this doesn't completely solve this issue becuase autogenerated
          // types can also be inside the body and be equal.
          // This means that each lambda instance needs to be checked if it is already
          // available (same hash) and the same type should be reused. This needs to be
          // added in the future.

          ast_t* type_a = ast_type(a);
          ast_t* type_b = ast_type(b);

          ast_t* data_a = (ast_t*)ast_data(type_a);
          ast_t* data_b = (ast_t*)ast_data(type_b);

          uint64_t a_hash = ast_hash(data_a);
          uint64_t b_hash = ast_hash(data_b);

          if(a_hash == b_hash)
          {
            return true;
          }
        }
      }
      break;

    default:
      pony_assert(0);
      break;
  }

  return false;
}

static bool is_eq_typeargs(ast_t* a, ast_t* b, errorframe_t* errorf,
  pass_opt_t* opt)
{
  pony_assert(ast_id(a) == TK_NOMINAL);
  pony_assert(ast_id(b) == TK_NOMINAL);

  // Check typeargs are the same.
  ast_t* a_arg = ast_child(ast_childidx(a, 2));
  ast_t* b_arg = ast_child(ast_childidx(b, 2));
  bool ret = true;

  while((a_arg != NULL) && (b_arg != NULL))
  {
    if(ast_id(a_arg) != TK_VALUEFORMALARG && ast_id(b_arg) != TK_VALUEFORMALARG)
    {
      if (!is_eqtype(a_arg, b_arg, errorf, opt))
        ret = false;
    }
    else if(ast_id(a_arg) == TK_VALUEFORMALARG && ast_id(b_arg) == TK_VALUEFORMALARG)
    {
      ast_t* lit_a = ast_child(a_arg);
      ast_t* lit_b = ast_child(b_arg);

      if (!is_literal_equal(lit_a, lit_b, opt, true))
        ret = false;

      ast_t* typeparam_a = get_valueformalarg_def(a_arg);
      ast_t* typeparam_b = get_valueformalarg_def(b_arg);

      if (!is_eqtype(ast_childidx(typeparam_a, 1), ast_childidx(typeparam_b, 1), errorf, opt))
        ret = false;
    }
    else
    {
      ret = false;
    }

    a_arg = ast_sibling(a_arg);
    b_arg = ast_sibling(b_arg);
  }

  if(!ret && errorf != NULL)
  {
    ast_error_frame(errorf, a,
      "%s has different type arguments than %s (the type arguments must be equivalent, not covariant nor contravariant)",
      ast_print_type(a, opt->strtab), ast_print_type(b, opt->strtab));
    ast_error_frame(errorf, a,
      "this might be possible if either %s or %s were an interface rather than a concrete type",
      ast_print_type(a, opt->strtab), ast_print_type(b, opt->strtab));
  }

  // Make sure we had the same number of typeargs.
  if((a_arg != NULL) || (b_arg != NULL))
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, a,
        "%s has a different number of type arguments than %s",
        ast_print_type(a, opt->strtab), ast_print_type(b, opt->strtab));
    }

    ret = false;
  }

  return ret;
}

static bool is_reified_fun_sub_fun(ast_t* sub, ast_t* super,
  errorframe_t* errorf, pass_opt_t* opt)
{
  AST_GET_CHILDREN(sub, sub_cap, sub_id, sub_typeparams, sub_params,
    sub_result, sub_throws);

  AST_GET_CHILDREN(super, super_cap, super_id, super_typeparams, super_params,
    super_result, super_throws);

  switch(ast_id(sub))
  {
    case TK_NEW:
    {
      // Covariant receiver.
      if(!is_cap_sub_cap(ast_id(sub_cap), TK_NONE, ast_id(super_cap), TK_NONE))
      {
        if(errorf != NULL)
        {
          ast_error_frame(errorf, sub,
            "%s constructor is not a subtype of %s constructor",
            ast_print_type(sub_cap, opt->strtab), ast_print_type(super_cap, opt->strtab));
        }

        return false;
      }

      // Covariant result. See the coupling note on the TK_FUN/TK_BE
      // covariant-result branch below about divergence on recursive
      // generic interfaces and the divergence guard in is_x_sub_x.
      if(!is_subtype(sub_result, super_result, errorf, opt))
      {
        if(errorf != NULL)
        {
          ast_error_frame(errorf, sub,
            "constructor result %s is not a subtype of %s",
            ast_print_type(sub_result, opt->strtab), ast_print_type(super_result, opt->strtab));
        }

        return false;
      }
      break;
    }

    case TK_FUN:
    case TK_BE:
    {
      bool sub_bare = ast_id(sub_cap) == TK_AT;
      pony_assert(sub_bare == (ast_id(super_cap) == TK_AT));

      // Contravariant receiver.
      if(!sub_bare &&
        !is_cap_sub_cap(ast_id(super_cap), TK_NONE, ast_id(sub_cap), TK_NONE))
      {
        if(errorf != NULL)
        {
          ast_error_frame(errorf, sub,
            "%s method is not a subtype of %s method",
            ast_print_type(sub_cap, opt->strtab), ast_print_type(super_cap, opt->strtab));
        }

        return false;
      }

      // Covariant result.
      // NOTE: is_reified_fun_sub_fun recurses into subtype checks at
      // four sites — the TK_NEW covariant result above, this
      // TK_FUN/TK_BE covariant result, the contravariant type parameter
      // constraints below, and the contravariant parameter types below.
      // Any of these can produce drifting same-def chains on
      // structurally-recursive generic interfaces (e.g. an interface
      // method whose return type, parameter type, or type-parameter
      // constraint references the same interface with strictly larger
      // type arguments). Termination relies on the recursion-divergence
      // guard in is_x_sub_x. If you change how any of these four
      // recursions work, revisit that guard. See ponylang/ponyc#1216.
      if(!is_subtype(sub_result, super_result, errorf, opt))
      {
        if(errorf != NULL)
        {
          ast_error_frame(errorf, sub,
            "method result %s is not a subtype of %s",
            ast_print_type(sub_result, opt->strtab), ast_print_type(super_result, opt->strtab));
        }

        return false;
      }

      // pass by value return types must have the same byval annotation
      // otherwise it is not the same type
      if(ast_has_annotation(sub_result, PONY_BYVAL_ANNOTATION, opt->strtab) !=
         ast_has_annotation(super_result, PONY_BYVAL_ANNOTATION, opt->strtab))
      {
        if(errorf != NULL)
        {
          ast_error_frame(errorf, sub,
            "%s and %s has different '" PONY_BYVAL_ANNOTATION "' return type annotations",
            ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
        }

        return false;
      }
      break;
    }

    default: {}
  }

  // Contravariant type parameter constraints.
  ast_t* sub_typeparam = ast_child(sub_typeparams);
  ast_t* super_typeparam = ast_child(super_typeparams);

  while((sub_typeparam != NULL) && (super_typeparam != NULL))
  {
    ast_t* sub_constraint = ast_childidx(sub_typeparam, 1);
    ast_t* super_constraint = ast_childidx(super_typeparam, 1);

    // Contravariant recursion. See the coupling note on the TK_FUN/TK_BE
    // covariant-result branch above about divergence on recursive
    // generic interfaces and the divergence guard in is_x_sub_x.
    if(!is_x_sub_x(super_constraint, sub_constraint, CHECK_CAP_EQ, errorf,
      opt))
    {
      if(errorf != NULL)
      {
        ast_error_frame(errorf, sub,
          "type parameter constraint %s is not a supertype of %s",
          ast_print_type(sub_constraint, opt->strtab), ast_print_type(super_constraint, opt->strtab));
      }

      return false;
    }

    sub_typeparam = ast_sibling(sub_typeparam);
    super_typeparam = ast_sibling(super_typeparam);
  }

  // Contravariant parameters.
  ast_t* sub_param = ast_child(sub_params);
  ast_t* super_param = ast_child(super_params);

  while((sub_param != NULL) && (super_param != NULL))
  {
    // observational: must pass K^ to argument of type K
    ast_t* sub_type = consume_type(ast_childidx(sub_param, 1), TK_NONE, false, opt);
    ast_t* super_type = consume_type(ast_childidx(super_param, 1), TK_NONE, false, opt);
    if (sub_type == NULL || super_type == NULL)
    {
      // invalid function types
      return false;
    }

    // pass by value types must have the same byval annotation
    // otherwise it is not the same type
    if(ast_has_annotation(sub_type, PONY_BYVAL_ANNOTATION, opt->strtab) !=
       ast_has_annotation(super_type, PONY_BYVAL_ANNOTATION, opt->strtab))
    {
      ast_error_frame(errorf, sub,
        "%s and %s has different '" PONY_BYVAL_ANNOTATION "' annotations",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
      return false;
    }

    // Contravariant: the super type must be a subtype of the sub type.
    // See the coupling note on the TK_FUN/TK_BE covariant-result branch
    // above about divergence on recursive generic interfaces and the
    // divergence guard in is_x_sub_x.
    if(!is_x_sub_x(super_type, sub_type, CHECK_CAP_SUB, errorf, opt))
    {
      if(errorf != NULL)
      {
        ast_error_frame(errorf, sub, "parameter %s is not a supertype of %s",
          ast_print_type(sub_type, opt->strtab), ast_print_type(super_type, opt->strtab));
      }

      ast_free_unattached(sub_type);
      ast_free_unattached(super_type);
      return false;
    }
    ast_free_unattached(sub_type);
    ast_free_unattached(super_type);

    sub_param = ast_sibling(sub_param);
    super_param = ast_sibling(super_param);
  }

  // Covariant throws.
  if((ast_id(sub_throws) == TK_QUESTION) &&
    (ast_id(super_throws) != TK_QUESTION))
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "a partial function is not a subtype of a total function");
    }

    return false;
  }

  return true;
}

static bool is_fun_sub_fun(ast_t* sub, ast_t* super, errorframe_t* errorf,
  pass_opt_t* opt)
{
  token_id tsub = ast_id(sub);
  token_id tsuper = ast_id(super);

  switch(tsub)
  {
    case TK_NEW:
    case TK_BE:
    case TK_FUN:
      break;

    default:
      pony_assert(0);
      return false;
  }

  switch(tsuper)
  {
    case TK_NEW:
    case TK_BE:
    case TK_FUN:
      break;

    default:
      pony_assert(0);
      return false;
  }

  AST_GET_CHILDREN(sub, sub_cap, sub_id, sub_typeparams, sub_params);
  AST_GET_CHILDREN(super, super_cap, super_id, super_typeparams, super_params);

  // Must have the same name.
  if(ast_name(sub_id) != ast_name(super_id))
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "method %s is not a subtype of method %s: they have different names",
        ast_name(sub_id), ast_name(super_id));
    }

    return false;
  }

  // A constructor can only be a subtype of a constructor.
  if(((tsub == TK_NEW) || (tsuper == TK_NEW)) && (tsub != tsuper))
  {
    if(errorf != NULL)
      ast_error_frame(errorf, sub,
        "only a constructor can be a subtype of a constructor");

    return false;
  }

  // Must have the same number of type parameters.
  if(ast_childcount(sub_typeparams) != ast_childcount(super_typeparams))
  {
    if(errorf != NULL)
      ast_error_frame(errorf, sub,
        "methods have a different number of type parameters");

    return false;
  }

  // Must have the same number of parameters.
  if(ast_childcount(sub_params) != ast_childcount(super_params))
  {
    if(errorf != NULL)
      ast_error_frame(errorf, sub,
        "methods have a different number of parameters");

    return false;
  }

  bool sub_bare = ast_id(sub_cap) == TK_AT;
  bool super_bare = ast_id(super_cap) == TK_AT;

  if(sub_bare != super_bare)
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "method %s is not a subtype of method %s: their bareness differ",
        ast_name(sub_id), ast_name(super_id));
    }

    return false;
  }

  ast_t* r_sub = sub;

  if(ast_id(super_typeparams) != TK_NONE)
  {
    // Reify sub with the type parameters of super.
    BUILD(typeargs, super_typeparams, NODE(TK_TYPEARGS));
    ast_t* super_typeparam = ast_child(super_typeparams);

    while(super_typeparam != NULL)
    {
      AST_GET_CHILDREN(super_typeparam, super_id, super_constraint);

      BUILD(typearg, super_typeparam,
        NODE(TK_TYPEPARAMREF, TREE(super_id) NONE NONE));

      ast_t* def = ast_get(super_typeparam, ast_name(super_id), NULL);
      ast_setdata(typearg, def);
      typeparam_set_cap(typearg);
      ast_append(typeargs, typearg);

      super_typeparam = ast_sibling(super_typeparam);
    }

    r_sub = reify_method_def(sub, sub_typeparams, typeargs, opt);
    ast_free_unattached(typeargs);
  }

  bool ok = is_reified_fun_sub_fun(r_sub, super, errorf, opt);

  if(r_sub != sub)
    ast_free_unattached(r_sub);

  return ok;
}

static bool is_x_sub_isect(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  // T1 <: T2
  // T1 <: T3
  // ---
  // T1 <: (T2 & T3)
  for(ast_t* child = ast_child(super);
    child != NULL;
    child = ast_sibling(child))
  {
    if(!is_x_sub_x(sub, child, check_cap, errorf, opt))
    {
      if(errorf != NULL)
      {
        ast_error_frame(errorf, sub,
          "%s is not a subtype of every element of %s",
          ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
      }

      return false;
    }
  }

  return true;
}

// Expand a type by distributing tuples over unions into a union of tuples.
// Returns a TK_UNIONTYPE containing all expanded alternatives, or NULL if
// the expansion would exceed max_alternatives.
//
// For TK_UNIONTYPE: collects alternatives from all members.
// For TK_TUPLETYPE: computes the cross-product of each element's alternatives.
// For TK_TYPEALIASREF: unfolds the alias and recurses.
// For anything else: returns a 1-element union containing a copy of the type.
static ast_t* expand_type_alternatives(ast_t* type, size_t max_alternatives)
{
  switch(ast_id(type))
  {
    case TK_UNIONTYPE:
    {
      ast_t* result = ast_from(type, TK_UNIONTYPE);

      for(ast_t* child = ast_child(type);
        child != NULL;
        child = ast_sibling(child))
      {
        ast_t* child_alts = expand_type_alternatives(child, max_alternatives);

        if(child_alts == NULL)
        {
          ast_free_unattached(result);
          return NULL;
        }

        for(ast_t* alt = ast_child(child_alts);
          alt != NULL;
          alt = ast_sibling(alt))
        {
          ast_append(result, ast_dup(alt));
        }

        ast_free_unattached(child_alts);

        if(ast_childcount(result) > max_alternatives)
        {
          ast_free_unattached(result);
          return NULL;
        }
      }

      return result;
    }

    case TK_TUPLETYPE:
    {
      // Start with a single empty tuple.
      ast_t* result = ast_from(type, TK_UNIONTYPE);
      ast_append(result, ast_from(type, TK_TUPLETYPE));

      for(ast_t* child = ast_child(type);
        child != NULL;
        child = ast_sibling(child))
      {
        ast_t* child_alts = expand_type_alternatives(child, max_alternatives);

        if(child_alts == NULL)
        {
          ast_free_unattached(result);
          return NULL;
        }

        ast_t* new_result = ast_from(type, TK_UNIONTYPE);

        for(ast_t* partial = ast_child(result);
          partial != NULL;
          partial = ast_sibling(partial))
        {
          for(ast_t* alt = ast_child(child_alts);
            alt != NULL;
            alt = ast_sibling(alt))
          {
            ast_t* new_tuple = ast_dup(partial);
            ast_append(new_tuple, ast_dup(alt));
            ast_append(new_result, new_tuple);
          }
        }

        ast_free_unattached(result);
        ast_free_unattached(child_alts);
        result = new_result;

        if(ast_childcount(result) > max_alternatives)
        {
          ast_free_unattached(result);
          return NULL;
        }
      }

      return result;
    }

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(type);

      if(unfolded == NULL)
      {
        ast_t* result = ast_from(type, TK_UNIONTYPE);
        ast_append(result, ast_dup(type));
        return result;
      }

      ast_t* result = expand_type_alternatives(unfolded, max_alternatives);
      ast_free_unattached(unfolded);
      return result;
    }

    default:
    {
      ast_t* result = ast_from(type, TK_UNIONTYPE);
      ast_append(result, ast_dup(type));
      return result;
    }
  }
}

// If type is a tuple containing union elements, expand it into an equivalent
// union of tuples. Returns the expanded union, or NULL if no expansion is
// needed or the expansion would be too large.
static ast_t* expand_tuple_unions(ast_t* type)
{
  if(ast_id(type) != TK_TUPLETYPE)
    return NULL;

  ast_t* expanded = expand_type_alternatives(type, 256);

  if((expanded == NULL) || (ast_childcount(expanded) <= 1))
  {
    ast_free_unattached(expanded);
    return NULL;
  }

  return expanded;
}

static bool is_x_sub_union(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  // T1 <: T2 or T1 <: T3
  // ---
  // T1 <: (T2 | T3)
  for(ast_t* child = ast_child(super);
    child != NULL;
    child = ast_sibling(child))
  {
    if(is_x_sub_x(sub, child, check_cap, NULL, opt))
      return true;
  }

  // A tuple of unions may be a subtype of a union of tuples without being a
  // subtype of any one element. Expand the tuple into a union of tuples
  // (cross-product) and check if every alternative is covered.
  if(ast_id(sub) == TK_TUPLETYPE)
  {
    ast_t* expanded = expand_tuple_unions(sub);

    if(expanded != NULL)
    {
      bool all_match = true;

      for(ast_t* alt = ast_child(expanded);
        alt != NULL;
        alt = ast_sibling(alt))
      {
        if(!is_x_sub_x(alt, super, check_cap, NULL, opt))
        {
          all_match = false;
          break;
        }
      }

      ast_free_unattached(expanded);

      if(all_match)
        return true;
    }
  }

  if(errorf != NULL)
  {
    for(ast_t* child = ast_child(super);
      child != NULL;
      child = ast_sibling(child))
    {
      is_x_sub_x(sub, child, check_cap, errorf, opt);
    }

    ast_error_frame(errorf, sub, "%s is not a subtype of any element of %s",
      ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
  }

  return false;
}

static bool is_union_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  // T1 <: T3
  // T2 <: T3
  // ---
  // (T1 | T2) <: T3
  for(ast_t* child = ast_child(sub);
    child != NULL;
    child = ast_sibling(child))
  {
    if(!is_x_sub_x(child, super, check_cap, errorf, opt))
    {
      if(errorf != NULL)
      {
        ast_error_frame(errorf, child,
          "not every element of %s is a subtype of %s",
          ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
      }

      return false;
    }
  }

  return true;
}

static bool is_isect_sub_isect(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  // (T1 & T2) <: T3
  // (T1 & T2) <: T4
  // ---
  // (T1 & T2) <: (T3 & T4)
  ast_t* super_child = ast_child(super);

  while(super_child != NULL)
  {
    if(!is_isect_sub_x(sub, super_child, check_cap, errorf, opt))
      return false;

    super_child = ast_sibling(super_child);
  }

  return true;
}

static bool is_isect_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  switch(ast_id(super))
  {
    case TK_ISECTTYPE:
      // (T1 & T2) <: (T3 & T4)
      return is_isect_sub_isect(sub, super, check_cap, errorf, opt);

    case TK_NOMINAL:
    {
      ast_t* super_def = (ast_t*)ast_data(super);

      // TODO: can satisfy the interface in aggregate
      // (T1 & T2) <: I k
      if(ast_id(super_def) == TK_INTERFACE)
      {
        // return is_isect_sub_interface(sub, super, errorf);
      }
      break;
    }

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(super);
      if(unfolded == NULL)
        return false;
      bool ok = is_x_sub_x(sub, unfolded, check_cap, errorf, opt);
      ast_free_unattached(unfolded);
      return ok;
    }

    default: {}
  }

  // T1 <: T3 or T2 <: T3
  // ---
  // (T1 & T2) <: T3
  for(ast_t* child = ast_child(sub);
    child != NULL;
    child = ast_sibling(child))
  {
    if(is_x_sub_x(child, super, check_cap, NULL, opt))
      return true;
  }

  if(errorf != NULL)
  {
    ast_error_frame(errorf, sub, "no element of %s is a subtype of %s",
      ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
  }

  return false;
}

static bool is_tuple_sub_tuple(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  // T1 <: T3
  // T2 <: T4
  // ---
  // (T1, T2) <: (T3, T4)
  if(ast_childcount(sub) != ast_childcount(super))
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "%s is not a subtype of %s: they have a different number of elements",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    }

    return false;
  }

  ast_t* sub_child = ast_child(sub);
  ast_t* super_child = ast_child(super);
  bool ret = true;

  while(sub_child != NULL)
  {
    if(!is_x_sub_x(sub_child, super_child, check_cap, errorf, opt))
      ret = false;

    sub_child = ast_sibling(sub_child);
    super_child = ast_sibling(super_child);
  }

  if(!ret && errorf != NULL)
  {
    ast_error_frame(errorf, sub, "%s is not a pairwise subtype of %s",
      ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
  }

  return ret;
}

static bool is_single_sub_tuple(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  (void)check_cap;
  (void)opt;

  if(errorf != NULL)
  {
    ast_error_frame(errorf, sub,
      "%s is not a subtype of %s: the supertype is a tuple",
      ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
  }

  return false;
}

static bool is_tuple_sub_nominal(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  if(is_top_type(super, true, opt))
  {
    for(ast_t* child = ast_child(sub);
      child != NULL;
      child = ast_sibling(child))
    {
      if(!is_x_sub_x(child, super, check_cap, errorf, opt))
      {
        if(errorf != NULL)
        {
          ast_error_frame(errorf, child,
            "%s is not a subtype of %s: %s is not a subtype of %s",
            ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab),
            ast_print_type(child, opt->strtab), ast_print_type(super, opt->strtab));
        }
        return false;
      }
    }

    return true;
  }

  if(errorf != NULL)
  {
    ast_t* super_def = (ast_t*)ast_data(super);

    ast_error_frame(errorf, sub,
      "%s is not a subtype of %s: the subtype is a tuple",
      ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    ast_error_frame(errorf, super_def, "this might be possible if the supertype"
      " were an empty interface, such as the Any type.");
  }

  return false;
}

static bool is_tuple_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  switch(ast_id(super))
  {
    case TK_UNIONTYPE:
      return is_x_sub_union(sub, super, check_cap, errorf, opt);

    case TK_ISECTTYPE:
      return is_x_sub_isect(sub, super, check_cap, errorf, opt);

    case TK_TUPLETYPE:
      return is_tuple_sub_tuple(sub, super, check_cap, errorf, opt);

    case TK_NOMINAL:
      return is_tuple_sub_nominal(sub, super, check_cap, errorf, opt);

    case TK_TYPEPARAMREF:
      return is_x_sub_typeparam(sub, super, check_cap, errorf, opt);

    case TK_ARROW:
      return is_x_sub_arrow(sub, super, check_cap, errorf, opt);

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(super);
      if(unfolded == NULL)
        return false;
      bool ok = is_x_sub_x(sub, unfolded, check_cap, errorf, opt);
      ast_free_unattached(unfolded);
      return ok;
    }

    case TK_FUNTYPE:
    case TK_INFERTYPE:
    case TK_ERRORTYPE:
    case TK_ENTITY_TYPE_CLASS:
    case TK_ENTITY_TYPE_STRUCT:
    case TK_ENTITY_TYPE_PRIMITIVE:
    case TK_ENTITY_TYPE_ACTOR:
      return false;

    default: {}
  }

  pony_assert(0);
  return false;
}

static bool is_nominal_sub_entity(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  // N = C
  // k <: k'
  // ---
  // N k <: C k'
  ast_t* sub_def = (ast_t*)ast_data(sub);
  ast_t* super_def = (ast_t*)ast_data(super);
  bool ret = true;

  if(sub_def != super_def)
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub, "%s is not a subtype of %s",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    }

    return false;
  }

  if(!is_eq_typeargs(sub, super, errorf, opt))
    ret = false;

  if(!is_sub_cap_and_eph(sub, super, check_cap, errorf, opt))
    ret = false;

  return ret;
}

static bool is_nominal_sub_structural(ast_t* sub, ast_t* super,
  errorframe_t* errorf, pass_opt_t* opt)
{
  // implements(N, I)
  // k <: k'
  // ---
  // N k <: I k
  ast_t* sub_def = (ast_t*)ast_data(sub);
  ast_t* super_def = (ast_t*)ast_data(super);

  // We could be reporting false negatives if the traits pass hasn't processed
  // our defs yet.
  pass_id sub_pass = (pass_id)ast_checkflag(sub_def, AST_FLAG_PASS_MASK);
  pass_id super_pass = (pass_id)ast_checkflag(super_def, AST_FLAG_PASS_MASK);
  pony_assert((sub_pass >= PASS_TRAITS) && (super_pass >= PASS_TRAITS));
  (void)sub_pass; (void)super_pass;

  if(ast_has_annotation(sub_def, "nosupertype", opt->strtab))
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "%s is not a subtype of %s: it is marked 'nosupertype'",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    }

    return false;
  }

  if(is_bare(sub, opt) != is_bare(super, opt))
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "%s is not a subtype of %s: their bareness differ",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    }

    return false;
  }

  bool ret = true;

  ast_t* sub_typeargs = ast_childidx(sub, 2);
  ast_t* sub_typeparams = ast_childidx(sub_def, 1);

  ast_t* super_typeargs = ast_childidx(super, 2);
  ast_t* super_typeparams = ast_childidx(super_def, 1);

  ast_t* super_members = ast_childidx(super_def, 4);
  ast_t* super_member = ast_child(super_members);

  while(super_member != NULL)
  {
    ast_t* super_member_id = ast_childidx(super_member, 1);
    ast_t* sub_member = ast_get(sub_def, ast_name(super_member_id), NULL);

    // If we don't provide a method, we aren't a subtype.
    if((sub_member == NULL) || (ast_id(sub_member) != TK_FUN &&
      ast_id(sub_member) != TK_BE && ast_id(sub_member) != TK_NEW))
    {
      if(errorf != NULL)
      {
        ast_error_frame(errorf, sub,
          "%s is not a subtype of %s: it has no method '%s'",
          ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab),
          ast_name(super_member_id));
      }

      ret = false;
      super_member = ast_sibling(super_member);
      continue;
    }

    // Reify the method on the subtype.
    ast_t* r_sub_member = reify_method_def(sub_member, sub_typeparams,
      sub_typeargs, opt);
    pony_assert(r_sub_member != NULL);

    // Reify the method on the supertype.
    ast_t* r_super_member = reify_method_def(super_member, super_typeparams,
      super_typeargs, opt);
    pony_assert(r_super_member != NULL);

    // Check the reified methods.
    bool ok = is_fun_sub_fun(r_sub_member, r_super_member, errorf, opt);
    ast_free_unattached(r_sub_member);
    ast_free_unattached(r_super_member);

    if(!ok)
    {
      ret = false;

      if(errorf != NULL)
      {
        ast_error_frame(errorf, sub_member,
          "%s is not a subtype of %s: "
          "method '%s' has an incompatible signature",
          ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab),
          ast_name(super_member_id));
      }
    }

    super_member = ast_sibling(super_member);
  }

  return ret;
}

static bool is_nominal_sub_interface(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  bool ret = true;

  // A struct has no descriptor, so can't be a subtype of an interface.
  ast_t* sub_def = (ast_t*)ast_data(sub);

  if(ast_id(sub_def) == TK_STRUCT)
  {
    struct_cant_be_x(sub, super, errorf, "an interface", opt);
    ret = false;
  }

  if(!is_sub_cap_and_eph(sub, super, check_cap, errorf, opt))
    ret = false;

  if(!is_nominal_sub_structural(sub, super, errorf, opt))
    ret = false;

  return ret;
}

static bool nominal_provides_trait(ast_t* type, ast_t* trait,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  pony_assert(!is_bare(trait, opt));
  if(is_bare(type, opt))
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, type,
        "%s is not a subtype of %s: their bareness differ",
        ast_print_type(type, opt->strtab), ast_print_type(trait, opt->strtab));
    }

    return false;
  }

  // Get our typeparams and typeargs.
  ast_t* def = (ast_t*)ast_data(type);
  AST_GET_CHILDREN(def, id, typeparams, defcap, traits);
  ast_t* typeargs = ast_childidx(type, 2);

  // Get cap and eph of the trait.
  AST_GET_CHILDREN(trait, t_pkg, t_name, t_typeparams, cap, eph);
  token_id tcap = ast_id(cap);
  token_id teph = ast_id(eph);

  // Check traits, depth first.
  ast_t* child = ast_child(traits);

  while(child != NULL)
  {
    // Reify the child with our typeargs.
    ast_t* r_child = reify(child, typeparams, typeargs, opt, true);
    pony_assert(r_child != NULL);

    // Use the cap and ephemerality of the trait.
    ast_t* rr_child = set_cap_and_ephemeral(r_child, tcap, teph);
    bool is_sub = is_x_sub_x(rr_child, trait, check_cap, NULL, opt);
    ast_free_unattached(rr_child);

    if(r_child != child)
      ast_free_unattached(r_child);

    if(is_sub)
      return true;

    child = ast_sibling(child);
  }

  if(errorf != NULL)
  {
    ast_error_frame(errorf, type, "%s does not implement trait %s",
      ast_print_type(type, opt->strtab), ast_print_type(trait, opt->strtab));
  }

  return false;
}

static bool is_entity_sub_trait(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  // implements(C, R)
  // k <: k'
  // ---
  // C k <: R k'
  if(!nominal_provides_trait(sub, super, check_cap, errorf, opt))
    return false;

  if(!is_sub_cap_and_eph(sub, super, check_cap, errorf, opt))
    return false;

  return true;
}

static bool is_struct_sub_trait(ast_t* sub, ast_t* super, errorframe_t* errorf, pass_opt_t* opt)
{
  struct_cant_be_x(sub, super, errorf, "a trait", opt);
  return false;
}

static bool is_trait_sub_trait(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  // R = R' or implements(R, R')
  // k <: k'
  // ---
  // R k <: R' k'
  ast_t* sub_def = (ast_t*)ast_data(sub);
  ast_t* super_def = (ast_t*)ast_data(super);
  bool ret = true;

  if(sub_def == super_def)
  {
    if(!is_eq_typeargs(sub, super, errorf, opt))
      ret = false;
  }
  else if(!nominal_provides_trait(sub, super, check_cap, errorf, opt))
  {
    ret = false;
  }

  if(!is_sub_cap_and_eph(sub, super, check_cap, errorf, opt))
    ret = false;

  return ret;
}

static bool is_interface_sub_trait(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  (void)check_cap;

  if(errorf != NULL)
  {
    ast_error_frame(errorf, sub,
      "%s is not a subtype of %s: an interface can't be a subtype of a trait",
      ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
  }

  return false;
}

static bool is_nominal_sub_trait(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  // N k <: I k'
  ast_t* sub_def = (ast_t*)ast_data(sub);

  // A struct has no descriptor, so can't be a subtype of a trait.
  switch(ast_id(sub_def))
  {
    case TK_PRIMITIVE:
    case TK_CLASS:
    case TK_ACTOR:
      return is_entity_sub_trait(sub, super, check_cap, errorf, opt);

    case TK_STRUCT:
      return is_struct_sub_trait(sub, super, errorf, opt);

    case TK_TRAIT:
      return is_trait_sub_trait(sub, super, check_cap, errorf, opt);

    case TK_INTERFACE:
      return is_interface_sub_trait(sub, super, check_cap, errorf, opt);

    default: {}
  }

  pony_assert(0);
  return false;
}

static bool is_nominal_sub_nominal(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  // N k <: N' k'. Cycle protection lives at is_x_sub_x's entry — the
  // outer wrapper catches every pair, including this one, before the
  // dispatch reaches here.
  ast_t* super_def = (ast_t*)ast_data(super);

  switch(ast_id(super_def))
  {
    case TK_PRIMITIVE:
    case TK_STRUCT:
    case TK_CLASS:
    case TK_ACTOR:
      return is_nominal_sub_entity(sub, super, check_cap, errorf, opt);

    case TK_INTERFACE:
      return is_nominal_sub_interface(sub, super, check_cap, errorf, opt);

    case TK_TRAIT:
      return is_nominal_sub_trait(sub, super, check_cap, errorf, opt);

    default:
      pony_assert(0);
      return false;
  }
}

static bool is_x_sub_typeparam(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  // If we want to ignore the cap, we can use the constraint of the type param.
  if(check_cap == CHECK_CAP_IGNORE)
  {
    ast_t* constraint = typeparam_constraint(super);

    if(constraint == NULL)
    {
      if(errorf != NULL)
      {
        ast_error_frame(errorf, sub,
          "%s is not a subtype of %s: the type parameter has no constraint",
          ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
      }

      return false;
    }

    return is_x_sub_x(sub, constraint, CHECK_CAP_IGNORE, errorf, opt);
  }

  // N k <: lowerbound(A k')
  // ---
  // N k <: A k'
  ast_t* super_lower = typeparam_lower(super);

  if(super_lower == NULL)
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "%s is not a subtype of %s: the type parameter has no lower bounds",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    }

    return false;
  }

  bool ok = is_x_sub_x(sub, super_lower, check_cap, errorf, opt);
  ast_free_unattached(super_lower);
  return ok;
}

static bool is_x_sub_arrow(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  // If we want to ignore the cap, we can ignore the left side of the arrow.
  if(check_cap == CHECK_CAP_IGNORE)
    return is_x_sub_x(sub, ast_childidx(super, 1), CHECK_CAP_IGNORE,
      errorf, opt);

  // If checking for subtype while ignoring the cap fails, then there's no way
  // that it will succeed as a subtype when taking the cap into account.
  // Failing fast can avoid expensive ast_dup operations in viewpoint functions.
  if(!is_x_sub_x(sub, ast_childidx(super, 1), CHECK_CAP_IGNORE, errorf,
    opt))
    return false;

  // N k <: lowerbound(T1->T2)
  // ---
  // N k <: T1->T2
  ast_t* super_lower = viewpoint_lower(super, opt);

  if(super_lower == NULL)
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "%s is not a subtype of %s: the supertype has no lower bounds",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    }

    return false;
  }

  bool ok = is_x_sub_x(sub, super_lower, check_cap, errorf, opt);
  ast_free_unattached(super_lower);
  return ok;
}

static bool is_nominal_entity_type(ast_t* sub, ast_t* super, errorframe_t* errorf,
  pass_opt_t* opt)
{
  (void)opt;

  ast_t* sub_def = (ast_t*)ast_data(sub);

  token_id super_id = ast_id(super);
  token_id sub_id = ast_id(sub_def);
  if((super_id == TK_ENTITY_TYPE_CLASS && sub_id == TK_CLASS) ||
     (super_id == TK_ENTITY_TYPE_STRUCT && sub_id == TK_STRUCT) ||
     (super_id == TK_ENTITY_TYPE_PRIMITIVE && sub_id == TK_PRIMITIVE) ||
     (super_id == TK_ENTITY_TYPE_ACTOR && sub_id == TK_ACTOR))
  {
    return true;
  }

  if(errorf != NULL)
  {
    ast_error_frame(errorf, sub,
      "%s is not an entity type of %s",
      ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
  }

  return false;
}

static bool is_nominal_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  // N k <: T
  switch(ast_id(super))
  {
    case TK_UNIONTYPE:
    {
      if(is_bare(sub, opt))
      {
        if(errorf != NULL)
        {
          ast_error_frame(errorf, sub, "a bare type cannot be in a union type");
          return false;
        }
      }

      return is_x_sub_union(sub, super, check_cap, errorf, opt);
    }

    case TK_ISECTTYPE:
    {
      if(is_bare(sub, opt))
      {
        if(errorf != NULL)
        {
          ast_error_frame(errorf, sub, "a bare type cannot be in an "
            "intersection type");
          return false;
        }
      }

      return is_x_sub_isect(sub, super, check_cap, errorf, opt);
    }

    case TK_TUPLETYPE:
      return is_single_sub_tuple(sub, super, check_cap, errorf, opt);

    case TK_NOMINAL:
      return is_nominal_sub_nominal(sub, super, check_cap, errorf, opt);

    case TK_TYPEPARAMREF:
      return is_x_sub_typeparam(sub, super, check_cap, errorf, opt);

    case TK_ARROW:
      return is_x_sub_arrow(sub, super, check_cap, errorf, opt);

    case TK_ENTITY_TYPE_CLASS:
    case TK_ENTITY_TYPE_STRUCT:
    case TK_ENTITY_TYPE_PRIMITIVE:
    case TK_ENTITY_TYPE_ACTOR:
      return is_nominal_entity_type(sub, super, errorf, opt);

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(super);
      if(unfolded == NULL)
        return false;
      bool ok = is_x_sub_x(sub, unfolded, check_cap, errorf, opt);
      ast_free_unattached(unfolded);
      return ok;
    }

    case TK_FUNTYPE:
    case TK_INFERTYPE:
    case TK_ERRORTYPE:
      return false;

    default: {}
  }

  pony_assert(0);
  return false;
}

static bool is_entity_type_sub_x(ast_t* sub, ast_t* super, errorframe_t* errorf,
  pass_opt_t* opt)
{
  // Entity type is the top type of all and it cannot be a subtype of anything
  // besides itself.

  (void)opt;
  // N k <: T
  switch(ast_id(super))
  {
    case TK_ENTITY_TYPE_CLASS:
    case TK_ENTITY_TYPE_STRUCT:
    case TK_ENTITY_TYPE_PRIMITIVE:
    case TK_ENTITY_TYPE_ACTOR:
      return ast_id(sub) == ast_id(super);

    default: {}
  }

  if(errorf != NULL)
  {
    ast_error_frame(errorf, sub,
      "entity %s is not a subtype of %s",
      ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
  }

  return false;
}

static bool is_typeparam_sub_typeparam(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  // k <: k'
  // ---
  // A k <: A k'
  (void)opt;
  ast_t* sub_def = (ast_t*)ast_data(sub);
  ast_t* super_def = (ast_t*)ast_data(super);

  // We know the bounds on the rcap is the same, so we have different
  // subtyping rules here.
  if(check_cap == CHECK_CAP_SUB)
    check_cap = CHECK_CAP_BOUND;

  // Dig through defs if there are multiple layers of directly-bound
  // type params (created through the collect_type_params function).
  sub_def = typeparam_root(sub_def);
  super_def = typeparam_root(super_def);

  if(sub_def == super_def)
    return is_sub_cap_and_eph(sub, super, check_cap, errorf, opt);

  if(errorf != NULL)
  {
    ast_error_frame(errorf, sub,
      "%s is not a subtype of %s: they are different type parameters",
      ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
  }

  return false;
}

static bool is_typeparam_sub_arrow(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  // If we want to ignore the cap, we can ignore the left side of the arrow.
  if(check_cap == CHECK_CAP_IGNORE)
    return is_typeparam_sub_x(sub, ast_childidx(super, 1), CHECK_CAP_IGNORE,
      errorf, opt);

  // If checking for subtype while ignoring the cap fails, then there's no way
  // that it will succeed as a subtype when taking the cap into account.
  // Failing fast can avoid expensive ast_dup operations in viewpoint functions.
  if(!is_typeparam_sub_x(sub, ast_childidx(super, 1), CHECK_CAP_IGNORE, errorf,
    opt))
    return false;

  // forall k' in k . A k' <: lowerbound(T1->T2 {A k |-> A k'})
  // ---
  // A k <: T1->T2
  ast_t* r_sub = viewpoint_reifytypeparam(sub, sub, opt);
  ast_t* r_super = viewpoint_reifytypeparam(super, sub, opt);

  if(r_sub != NULL)
  {
    bool ok = is_x_sub_x(r_sub, r_super, check_cap, errorf, opt);
    ast_free_unattached(r_sub);
    ast_free_unattached(r_super);
    return ok;
  }

  // If there is only a single instantiation, calculate the lower bounds.
  //
  // A k <: lowerbound(T1->T2)
  // ---
  // A k <: T1->T2
  ast_t* super_lower = viewpoint_lower(super, opt);

  if(super_lower == NULL)
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "%s is not a subtype of %s: the supertype has no lower bounds",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    }

    return false;
  }

  bool ok = is_x_sub_x(sub, super_lower, check_cap, errorf, opt);
  ast_free_unattached(super_lower);
  return ok;
}

static bool is_typeparam_base_sub_x(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  switch(ast_id(super))
  {
    case TK_UNIONTYPE:
      return is_x_sub_union(sub, super, check_cap, errorf, opt);

    case TK_ISECTTYPE:
      return is_x_sub_isect(sub, super, check_cap, errorf, opt);

    case TK_TUPLETYPE:
    case TK_NOMINAL:
      return false;

    case TK_TYPEPARAMREF:
      return is_typeparam_sub_typeparam(sub, super, check_cap, errorf, opt);

    case TK_ARROW:
      return is_typeparam_sub_arrow(sub, super, check_cap, errorf, opt);

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(super);
      if(unfolded == NULL)
        return false;
      bool ok = is_x_sub_x(sub, unfolded, check_cap, errorf, opt);
      ast_free_unattached(unfolded);
      return ok;
    }

    case TK_FUNTYPE:
    case TK_INFERTYPE:
    case TK_ERRORTYPE:
      return false;

    default: {}
  }

  pony_assert(0);
  return false;
}

static bool is_typeparam_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  if(is_typeparam_base_sub_x(sub, super, check_cap, NULL, opt))
    return true;

  // If we want to ignore the cap, we can use the constraint of the type param.
  if(check_cap == CHECK_CAP_IGNORE)
  {
    ast_t* constraint = typeparam_constraint(sub);

    if(constraint == NULL)
    {
      if(errorf != NULL)
      {
        ast_error_frame(errorf, sub,
          "%s is not a subtype of %s: the subtype has no constraint",
          ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
      }

      return false;
    }

    return is_x_sub_x(constraint, super, CHECK_CAP_IGNORE, errorf, opt);
  }

  // upperbound(A k) <: T
  // ---
  // A k <: T
  ast_t* sub_upper = typeparam_upper(sub);

  if(sub_upper == NULL)
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "%s is not a subtype of %s: the subtype has no constraint",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    }

    return false;
  }

  bool ok = is_x_sub_x(sub_upper, super, check_cap, errorf, opt);
  ast_free_unattached(sub_upper);
  return ok;
}

// Unfortunately, this typechecking method relies on many assumptions about how
// and when it will be called. This is because we can't nicely check values
// during the type checking phase as we need expressions to be evaluated but the
// expressions aren't guaranteed to be type safe / legal until after typechecking.
// As such we delay checking equality of value arguments until after type checking
// and after evaluation. At the time of writing there are no value constraints
// so provided the value is of the correct type this is fine.
// TODO: ammend this comment after value constraints
static bool is_typevalueparam_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  if (ast_id(super) == TK_VALUEFORMALPARAMREF)
  {
    ast_t* super_underlying_type = ast_type(super);
    pony_assert(super_underlying_type != NULL);

    return is_x_sub_x(sub, super_underlying_type, check_cap, errorf, opt);
  }
  else if(ast_id(super) == TK_VALUEFORMALARG)
  {
    ast_t* sub_value = ast_child(sub);
    ast_t *super_value = ast_child(super);
    ast_t *super_type = ast_type(sub_value);
    ast_t *sub_type = ast_type(super_value);

    // The type of these should be equal so we first check this
    if(!is_eqtype(super_type, sub_type, errorf, opt))
      return false;

    // If the expressions are not the same then we add them to a list of
    // expressions which we which check for equality later.
    /*if (!ast_equa(super_value, sub_value))
    {
      // If we are later than type checking then this will have already been handled
      // TODO: find a nicer way of handling this edge case
      if(opt->program_pass >= PASS_REACH)
        return true;

      ast_t* type = opt->check.frame->type;

      // TODO: what is a better way of getting the memeber?
      ast_t* member = ast_nearest(super_value, TK_FUN);
      if(member == NULL)
        member = ast_nearest(super_value, TK_NEW);

      // If member is still NULL it's because we have no
      // member to attach an equality entry to, this may be
      // the case when a class inherits from a trait. This
      // is fine as the entry will have been attached to another
      // list.
      if(member == NULL)
        return true;

      assert(type != NULL);
      assert(member != NULL);

      const char* type_name = ast_name(ast_child(type));
      const char* member_name = ast_name(ast_childidx(member, 1));

      equality_entry_t* entry = search_equality(opt->check.equality_tab,
                                                type_name, member_name);

      // This condition is a slight optimisation as this method is called
      // from eq_typeargs so all entries would appear twice. This improves
      // compilation time and error messages.
      if(entry == NULL || !ast_equal(sub_value, entry->current->lhs))
        mark_check_equality(opt->check.equality_tab, type_name, member_name,
                            super_value, sub_value);
    }*/

    return true;
  }
  else
  {
    ast_t* sub_underlying_type = (ast_t*)ast_data(sub);
    pony_assert(sub_underlying_type != NULL);

    return is_x_sub_x(sub_underlying_type, super, check_cap, errorf, opt);
  }

  return false;
}

static bool is_arrow_sub_nominal(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  // If we want to ignore the cap, we can ignore the left side of the arrow.
  if(check_cap == CHECK_CAP_IGNORE)
    return is_x_sub_x(ast_childidx(sub, 1), super, CHECK_CAP_IGNORE, errorf,
      opt);

  // If checking for subtype while ignoring the cap fails, then there's no way
  // that it will succeed as a subtype when taking the cap into account.
  // Failing fast can avoid expensive ast_dup operations in viewpoint functions.
  if(!is_x_sub_x(ast_childidx(sub, 1), super, CHECK_CAP_IGNORE, errorf, opt))
    return false;

  // upperbound(T1->T2) <: N k
  // ---
  // T1->T2 <: N k
  ast_t* sub_upper = viewpoint_upper(sub, opt);

  if(sub_upper == NULL)
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "%s is not a subtype of %s: the subtype has no upper bounds",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    }

    return false;
  }

  bool ok = is_x_sub_x(sub_upper, super, check_cap, errorf, opt);
  ast_free_unattached(sub_upper);
  return ok;
}

static bool is_arrow_sub_typeparam(ast_t* sub, ast_t* super,
  check_cap_t check_cap, errorframe_t* errorf, pass_opt_t* opt)
{
  // If we want to ignore the cap, we can ignore the left side of the arrow.
  if(check_cap == CHECK_CAP_IGNORE)
    return is_x_sub_x(ast_childidx(sub, 1), super, CHECK_CAP_IGNORE, errorf,
      opt);

  // If checking for subtype while ignoring the cap fails, then there's no way
  // that it will succeed as a subtype when taking the cap into account.
  // Failing fast can avoid expensive ast_dup operations in viewpoint functions.
  if(!is_x_sub_x(ast_childidx(sub, 1), super, CHECK_CAP_IGNORE, errorf, opt))
    return false;

  // forall k' in k . T1->T2 {A k |-> A k'} <: A k'
  // ---
  // T1->T2 <: A k
  ast_t* r_sub = viewpoint_reifytypeparam(sub, super, opt);
  ast_t* r_super = viewpoint_reifytypeparam(super, super, opt);

  if(r_sub != NULL)
  {
    bool ok = is_x_sub_x(r_sub, r_super, check_cap, errorf, opt);
    ast_free_unattached(r_sub);
    ast_free_unattached(r_super);
    return ok;
  }

  // If there is only a single instantiation, treat as a nominal type.
  return is_arrow_sub_nominal(sub, super, check_cap, errorf, opt);
}

static bool is_arrow_sub_arrow(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  // If we want to ignore the cap, we can ignore the left side of both arrows.
  if(check_cap == CHECK_CAP_IGNORE)
    return is_x_sub_x(ast_childidx(sub, 1), ast_childidx(super, 1),
      CHECK_CAP_IGNORE, errorf, opt);

  // If checking for subtype while ignoring the cap fails, then there's no way
  // that it will succeed as a subtype when taking the cap into account.
  // Failing fast can avoid expensive ast_dup operations in viewpoint functions.
  if(!is_x_sub_x(ast_childidx(sub, 1), ast_childidx(super, 1), CHECK_CAP_IGNORE,
    errorf, opt))
    return false;

  // S = this | A {#read, #send, #share, #any}
  // K = N k | A {iso, trn, ref, val, box, tag} | K->K | (empty)
  // L = S | K
  // T = N k | A k | L->T
  //
  // forall K' in S . K->S->T1 {S |-> K'} <: T2 {S |-> K'}
  // ---
  // K->S->T1 <: T2
  ast_t* r_sub;
  ast_t* r_super;

  if(viewpoint_reifypair(sub, super, &r_sub, &r_super, opt))
  {
    bool ok = is_x_sub_x(r_sub, r_super, check_cap, errorf, opt);
    ast_free_unattached(r_sub);
    ast_free_unattached(r_super);
    return ok;
  }

  // No elements need reification.
  //
  // upperbound(T1->T2) <: lowerbound(T3->T4)
  // ---
  // T1->T2 <: T3->T4
  ast_t* sub_upper = viewpoint_upper(sub, opt);
  ast_t* super_lower = viewpoint_lower(super, opt);
  bool ok = true;

  if(sub_upper == NULL)
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "%s is not a subtype of %s: the subtype has no upper bounds",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    }

    ok = false;
  }

  if(super_lower == NULL)
  {
    if(errorf != NULL)
    {
      ast_error_frame(errorf, sub,
        "%s is not a subtype of %s: the supertype has no lower bounds",
        ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab));
    }

    ok = false;
  }

  if(ok)
    ok = is_x_sub_x(sub_upper, super_lower, check_cap, errorf, opt);

  ast_free_unattached(sub_upper);
  ast_free_unattached(super_lower);
  return ok;
}

static bool is_arrow_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  switch(ast_id(super))
  {
    case TK_UNIONTYPE:
      return is_x_sub_union(sub, super, check_cap, errorf, opt);

    case TK_ISECTTYPE:
      return is_x_sub_isect(sub, super, check_cap, errorf, opt);

    case TK_TUPLETYPE:
      return is_single_sub_tuple(sub, super, check_cap, errorf, opt);

    case TK_NOMINAL:
      return is_arrow_sub_nominal(sub, super, check_cap, errorf, opt);

    case TK_TYPEPARAMREF:
      return is_arrow_sub_typeparam(sub, super, check_cap, errorf, opt);

    case TK_ARROW:
      return is_arrow_sub_arrow(sub, super, check_cap, errorf, opt);

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(super);
      if(unfolded == NULL)
        return false;
      bool ok = is_x_sub_x(sub, unfolded, check_cap, errorf, opt);
      ast_free_unattached(unfolded);
      return ok;
    }

    case TK_FUNTYPE:
    case TK_INFERTYPE:
    case TK_ERRORTYPE:
      return false;

    default: {}
  }

  pony_assert(0);
  return false;
}

static bool is_x_sub_x_impl(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt);

static bool is_x_sub_x(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  pony_assert(sub != NULL);
  pony_assert(super != NULL);

  size_t my_depth = type_assume_depth(TYPE_ASSUME_SUBTYPE);

  // Top-level boundary: drop any cache state from a prior user-level
  // subtype call. Two reasons combine here.
  //
  // 1. Soundness anchor for CACHE_DEPENDS_ON_TOP_LEVEL entries.
  //    Such an entry can only be consulted while the same depth-0
  //    entry that was on the stack at insert time is still on the
  //    stack. Soundness rests on two facts:
  //
  //      a. Conditional entries are only consulted when stack.len
  //         > 0 (gated at the call site below; see also
  //         subtype_cache_lookup's docstring).
  //      b. Every is_x_sub_x entry at my_depth == 0 clears the
  //         cache before any lookup. After the depth-0 frame that
  //         owns a conditional entry pops (stack returns to
  //         empty), the next user-level subtype call must enter at
  //         my_depth == 0 and clears.
  //
  //    Together these guarantee a conditional entry is only ever
  //    consulted under the same depth-0 entry that was active at
  //    insert time. The cross-sibling case — an unguarded outer
  //    pair where is_union_sub_x dispatches to is_x_sub_x per child
  //    and the outer pair did not push an assumption frame, so
  //    each child re-enters at my_depth == 0 — gets a fresh cache
  //    per sibling: the clear wipes the whole map, so even
  //    CACHE_UNCONDITIONAL entries from a prior sibling don't carry
  //    over. No inter-sibling reuse, no contamination.
  //
  // 2. Prevents fingerprint pointer reuse. The fingerprint mixes
  //    ast_data into the structural identity. Across user-level
  //    calls, a freed AST's address can be re-used by a freshly-
  //    allocated AST with a different def. Without the clear, a
  //    stale entry could collide with a same-address-but-different-
  //    def query in a later call.
  //
  // Cost is O(cache_size) — free entries, destroy and re-create the
  // hashmap (see subtype_cache_clear for the exact mechanics). Cache
  // size is bounded by one top-level call's working set, and the
  // clear runs at most once per user-level subtype call. No
  // explicit "exit" hook is needed: stale entries can linger if a
  // thread does no further is_x_sub_x calls, but they cause no
  // correctness issue, and any heap-spilled fingerprint storage is
  // reclaimed at the next clear.
  if(my_depth == 0)
    subtype_cache_clear();

  // Skip the cache when the caller is collecting diagnostics. Two
  // reasons gate this together:
  //   (1) Correctness: a cache hit would short-circuit the recursion
  //       and skip the side-effect of emitting error frames. Callers
  //       passing errorf need the frames, not just the bool answer.
  //   (2) Perf: the errorf-bearing path dominates the typechecker
  //       workload but is absent from the reach pass that drives the
  //       SCC compile-time DoS the cache exists to fix. Bypassing
  //       costs nothing on the path the cache is meant to help.
  const bool cache_lookup_allowed = (errorf == NULL);
  if(cache_lookup_allowed)
  {
    subtype_cache_value_t hit;
    if(subtype_cache_lookup(sub, super, (uint8_t)check_cap, &hit) &&
      (hit.kind == CACHE_UNCONDITIONAL || my_depth > 0))
    {
      // Propagate the cached entry's dependence so the caller's
      // classification reflects it. CACHE_DEPENDS_ON_TOP_LEVEL means
      // the producing descent fired a cycle hit at stack index 0;
      // surface that as a cycle-hit-at-0 in the accumulator.
      if(hit.kind == CACHE_DEPENDS_ON_TOP_LEVEL)
      {
        if(subtype_cache_min_match_idx_get() > 0)
          subtype_cache_min_match_idx_set(0);
      }
      return hit.result;
    }
  }

  // Co-inductive cycle protection at recurrence-prone pairs. Cycles
  // form at:
  //   - TK_NOMINAL pairs (recursive interface back-references and
  //     parameterized types with recursive aliases as type arguments)
  //   - pairs where at least one side is a TK_TYPEALIASREF (recursive
  //     alias unfolds, e.g., None <: A where A is (A | None) recurses
  //     to None <: A through the union dispatch).
  // Compound shapes (union/isect/tuple/arrow) decompose to per-child
  // is_x_sub_x calls in the impl, so any cycle they could form must
  // pass through one of the leaf shapes above; gating only on those
  // catches the cycles without paying the wrapper cost on every
  // structural decomposition. Pushing on every pair would incorrectly
  // treat ordinary structural subtype recursion that happens to
  // revisit the same shape as a cycle.
  bool guard = (ast_id(sub) == TK_TYPEALIASREF)
    || (ast_id(super) == TK_TYPEALIASREF)
    || ((ast_id(sub) == TK_NOMINAL) && (ast_id(super) == TK_NOMINAL));

  bool entered = false;
  if(guard)
  {
    // Recursion-divergence guard for drifting same-def chains
    // (ponylang/ponyc#1216). Recursive generic interfaces such as
    //
    //     interface Iter[A]
    //       fun enum[B](): Iter[(B, A)] => this
    //
    // produce subtype queries where every level shares the same def
    // pair (Iter, Iter) but has strictly larger typeargs at each step
    // (Iter[A] <: Iter[(B, A)], then Iter[(B', A)] <: Iter[(B', (B, A))],
    // ...). is_assumption_match is purely structural, so drifting
    // typeargs never match earlier entries; the structural cycle
    // detection below cannot terminate the chain. Without a divergence
    // bound the recursion grows without limit.
    //
    // When the new query is a (NOMINAL, NOMINAL) pair and the SUBTYPE
    // assume stack already holds SAME_DEF_LIMIT entries with the same
    // (sub_def, super_def), bail with `false` rather than push another
    // frame. Bailing with `false` is the conservative answer: a
    // convergent proof would have closed structurally by this depth;
    // the alternative (returning true coinductively when typeargs
    // differ) is unsound.
    //
    // The four call sites in is_reified_fun_sub_fun (covariant result
    // for TK_NEW and TK_FUN/TK_BE, contravariant type-parameter
    // constraints, contravariant parameter types) are the recursions
    // that drive this divergence on recursive generic interfaces; if
    // those change, revisit this guard.
    //
    // SAME_DEF_LIMIT was inherited from upstream's empirical floor in
    // 0483f9d99: K=2 was refuted by pony_check shapes that need more
    // than one drifting round to converge; K=4 leaves headroom.
    // Lowering K requires re-validating against pony_check
    // (packages/pony_check/) and any other stdlib shape that exercises
    // drifting same-def recursion. Raising K is always safe.
    if((ast_id(sub) == TK_NOMINAL) && (ast_id(super) == TK_NOMINAL))
    {
      const size_t SAME_DEF_LIMIT = 4;
      size_t same_def = type_assume_same_def_count(TYPE_ASSUME_SUBTYPE,
        ast_data(sub), ast_data(super));
      if(same_def >= SAME_DEF_LIMIT)
      {
        if(errorf != NULL)
        {
          ast_error_frame(errorf, sub,
            "%s is not a subtype of %s: recursion through generic "
            "typeargs that grow at each level. The "
            "recursion-divergence guard aborted after %zu same-def "
            "frames (ponylang/ponyc#1216).",
            ast_print_type(sub, opt->strtab), ast_print_type(super, opt->strtab), SAME_DEF_LIMIT);
        }

        // Mark the subtree as poisoned so neither this frame nor any
        // ancestor caches the bailed result. The bailed `false` is a
        // conservative answer; caching it could give wrong answers in
        // another stack context within the same top-level call.
        subtype_cache_subtree_poisoned_set(true);
        return false;
      }
    }

    int matched = type_assume_enter_indexed(TYPE_ASSUME_SUBTYPE, sub, super);
    if(matched >= 0)
    {
      // Cycle base case. Record the matched stack index as a cycle
      // hit so the caller's classification can see how shallow this
      // cycle reaches.
      if(matched < subtype_cache_min_match_idx_get())
        subtype_cache_min_match_idx_set(matched);
      return true;
    }
    entered = true;
  }

  // Save the caller's accumulator state and run the impl with a fresh
  // baseline. The accumulator state the impl finishes with is this
  // frame's own (my_min, my_poisoned), used below for classification.
  int saved_min = subtype_cache_min_match_idx_get();
  bool saved_poisoned = subtype_cache_subtree_poisoned_get();
  subtype_cache_min_match_idx_set(INT_MAX);
  subtype_cache_subtree_poisoned_set(false);

  bool result = is_x_sub_x_impl(sub, super, check_cap, errorf, opt);

  if(entered)
    type_assume_leave(TYPE_ASSUME_SUBTYPE);

  int my_min = subtype_cache_min_match_idx_get();
  bool my_poisoned = subtype_cache_subtree_poisoned_get();

  // Categorize and cache. Errorf-bearing calls skip insertion (same
  // reason as the lookup bypass above). Poisoned subtrees skip
  // insertion (the divergence-guard bail is conservative, not a real
  // subtype refutation). Intermediate min_match_idx (in the open range
  // (0, my_depth)) is intentionally not cached: the entry would depend
  // on an intermediate assumption with no clean invalidation hook. SCC
  // reach-pass workloads collapse entirely to {INT_MAX, 0, >= my_depth};
  // the intermediate case matters only for nested-top-level scenarios
  // and is parked.
  if((errorf == NULL) && !my_poisoned)
  {
    if((my_min == INT_MAX) || (my_min >= (int)my_depth))
    {
      subtype_cache_insert(sub, super, (uint8_t)check_cap, result,
        CACHE_UNCONDITIONAL);
    }
    else if(my_min == 0)
    {
      subtype_cache_insert(sub, super, (uint8_t)check_cap, result,
        CACHE_DEPENDS_ON_TOP_LEVEL);
    }
  }

  // Propagate to caller: take the min of caller's saved and my own,
  // and OR the poison flags so any ancestor that picks up our
  // poisoned subtree also skips caching.
  int new_min = (saved_min < my_min) ? saved_min : my_min;
  subtype_cache_min_match_idx_set(new_min);
  subtype_cache_subtree_poisoned_set(saved_poisoned || my_poisoned);

  return result;
}

static bool is_x_sub_x_impl(ast_t* sub, ast_t* super, check_cap_t check_cap,
  errorframe_t* errorf, pass_opt_t* opt)
{
  if((ast_id(super) == TK_DONTCARETYPE) || (ast_id(sub) == TK_DONTCARETYPE))
    return true;

  // Bool singleton types: only the identical singleton matches as super.
  if(ast_id(super) == TK_BOOL_TRUE || ast_id(super) == TK_BOOL_FALSE)
    return ast_id(sub) == ast_id(super);

  switch(ast_id(sub))
  {
    case TK_UNIONTYPE:
      return is_union_sub_x(sub, super, check_cap, errorf, opt);

    case TK_ISECTTYPE:
      return is_isect_sub_x(sub, super, check_cap, errorf, opt);

    case TK_TUPLETYPE:
      return is_tuple_sub_x(sub, super, check_cap, errorf, opt);

    case TK_NOMINAL:
      return is_nominal_sub_x(sub, super, check_cap, errorf, opt);

    case TK_TYPEPARAMREF:
      return is_typeparam_sub_x(sub, super, check_cap, errorf, opt);

    case TK_VALUEFORMALPARAMREF:
      return is_typevalueparam_sub_x(sub, super, check_cap, errorf, opt);

    case TK_ARROW:
      return is_arrow_sub_x(sub, super, check_cap, errorf, opt);

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(sub);
      if(unfolded == NULL)
        return false;
      bool ok = is_x_sub_x(unfolded, super, check_cap, errorf, opt);
      ast_free_unattached(unfolded);
      return ok;
    }

    case TK_FUNTYPE:
    case TK_INFERTYPE:
    case TK_ERRORTYPE:
      return false;

    case TK_ENTITY_TYPE_CLASS:
    case TK_ENTITY_TYPE_STRUCT:
    case TK_ENTITY_TYPE_PRIMITIVE:
    case TK_ENTITY_TYPE_ACTOR:
      return is_entity_type_sub_x(sub, super, errorf, opt);

    case TK_BOOL_TRUE:
    case TK_BOOL_FALSE:
    {
      if(ast_id(sub) == ast_id(super))
        return true;
      if(is_bool(super))
        return true;
      if(ast_id(super) == TK_UNIONTYPE)
        return is_x_sub_union(sub, super, check_cap, errorf, opt);
      return false;
    }

    default: {}
  }

  return true;

  pony_assert(0);
  return false;
}

bool is_subtype(ast_t* sub, ast_t* super, errorframe_t* errorf,
  pass_opt_t* opt)
{
  return is_x_sub_x(sub, super, CHECK_CAP_SUB, errorf, opt);
}

bool is_subtype_constraint(ast_t* sub, ast_t* super, errorframe_t* errorf,
  pass_opt_t* opt)
{
  return is_x_sub_x(sub, super, CHECK_CAP_EQ, errorf, opt);
}

bool is_subtype_ignore_cap(ast_t* sub, ast_t* super, errorframe_t* errorf,
  pass_opt_t* opt)
{
  return is_x_sub_x(sub, super, CHECK_CAP_IGNORE, errorf, opt);
}

bool is_subtype_fun(ast_t* sub, ast_t* super, errorframe_t* errorf,
  pass_opt_t* opt)
{
  return is_fun_sub_fun(sub, super, errorf, opt);
}

bool is_eqtype(ast_t* a, ast_t* b, errorframe_t* errorf, pass_opt_t* opt)
{
  return is_subtype(a, b, errorf, opt) && is_subtype(b, a, errorf, opt);
}

bool is_sub_provides(ast_t* type, ast_t* provides, errorframe_t* errorf,
  pass_opt_t* opt)
{
  pony_assert(ast_id(type) == TK_NOMINAL);
  return is_nominal_sub_structural(type, provides, errorf, opt);
}

bool is_literal(ast_t* type, const char* name)
{
  if(type == NULL)
    return false;

  if(ast_id(type) == TK_TYPEALIASREF)
  {
    ast_t* unfolded = typealias_unfold(type);

    if(unfolded == NULL)
      return false;

    bool r = is_literal(unfolded, name);
    ast_free_unattached(unfolded);
    return r;
  }

  if(ast_id(type) != TK_NOMINAL)
    return false;

  // Don't have to check the package, since literals are all builtins.
  return !strcmp(ast_name(ast_childidx(type, 1)), name);
}

bool is_value_formal_arg_literal(ast_t* ast)
{
  if (ast == NULL)
    return false;

  token_id id = ast_id(ast);
  return id == TK_TRUE   ||
         id == TK_FALSE  ||
         id == TK_INT    ||
         id == TK_FLOAT  ||
         id == TK_STRING ||
         id == TK_CONSTANT_OBJECT;
}

bool is_pointer(ast_t* type)
{
  return is_literal(type, "Pointer");
}

bool is_nullable_pointer(ast_t* type)
{
  return is_literal(type, "NullablePointer");
}

bool is_c_fixed_sized_array(ast_t* type)
{
  return is_literal(type, "CFixedSizedArray");
}

bool is_none(ast_t* type)
{
  return is_literal(type, "None");
}

bool is_any(ast_t* type)
{
  return is_literal(type, "Any");
}

bool is_env(ast_t* type)
{
  return is_literal(type, "Env");
}

bool is_runtime_options(ast_t* type)
{
  return is_literal(type, "RuntimeOptions");
}

bool is_bool(ast_t* type)
{
  return is_literal(type, "Bool");
}

bool is_float(ast_t* type)
{
  return is_literal(type, "F32") || is_literal(type, "F64");
}

bool is_integer(ast_t* type)
{
  return
    is_literal(type, "I8") ||
    is_literal(type, "I16") ||
    is_literal(type, "I32") ||
    is_literal(type, "I64") ||
    is_literal(type, "I128") ||
    is_literal(type, "ILong") ||
    is_literal(type, "ISize") ||
    is_literal(type, "U8") ||
    is_literal(type, "U16") ||
    is_literal(type, "U32") ||
    is_literal(type, "U64") ||
    is_literal(type, "U128") ||
    is_literal(type, "ULong") ||
    is_literal(type, "USize");
}

bool is_machine_word(ast_t* type)
{
  return is_bool(type) || is_integer(type) || is_float(type);
}

bool is_signed(ast_t* type)
{
  return
    is_literal(type, "I8") ||
    is_literal(type, "I16") ||
    is_literal(type, "I32") ||
    is_literal(type, "I64") ||
    is_literal(type, "I128") ||
    is_literal(type, "ILong") ||
    is_literal(type, "ISize");
}

bool is_constructable(ast_t* type)
{
  if(type == NULL)
    return false;

  switch(ast_id(type))
  {
    case TK_UNIONTYPE:
    case TK_TUPLETYPE:
      return false;

    case TK_ISECTTYPE:
    {
      ast_t* child = ast_child(type);

      while(child != NULL)
      {
        if(is_constructable(child))
          return true;

        child = ast_sibling(child);
      }

      return false;
    }

    case TK_NOMINAL:
    {
      ast_t* def = (ast_t*)ast_data(type);

      switch(ast_id(def))
      {
        case TK_INTERFACE:
        case TK_TRAIT:
        {
          ast_t* members = ast_childidx(def, 4);
          ast_t* member = ast_child(members);

          while(member != NULL)
          {
            if(ast_id(member) == TK_NEW)
              return true;

            member = ast_sibling(member);
          }

          return false;
        }

        case TK_PRIMITIVE:
        case TK_STRUCT:
        case TK_CLASS:
        case TK_ACTOR:
          return true;

        default: {}
      }
      break;
    }

    case TK_TYPEPARAMREF:
      return is_constructable(typeparam_constraint(type));

    case TK_VALUEFORMALPARAMREF:
      return is_constructable(ast_type(type));

    case TK_ARROW:
      return is_constructable(ast_childidx(type, 1));

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(type);
      if(unfolded == NULL)
        return false;
      bool ok = is_constructable(unfolded);
      ast_free_unattached(unfolded);
      return ok;
    }

    default: {}
  }

  pony_assert(0);
  return false;
}

bool is_concrete(ast_t* type)
{
  if(type == NULL)
    return false;

  switch(ast_id(type))
  {
    case TK_UNIONTYPE:
    case TK_TUPLETYPE:
      return false;

    case TK_ISECTTYPE:
    {
      ast_t* child = ast_child(type);

      while(child != NULL)
      {
        if(is_concrete(child))
          return true;

        child = ast_sibling(child);
      }

      return false;
    }

    case TK_NOMINAL:
    {
      ast_t* def = (ast_t*)ast_data(type);

      switch(ast_id(def))
      {
        case TK_INTERFACE:
        case TK_TRAIT:
          return false;

        case TK_PRIMITIVE:
        case TK_STRUCT:
        case TK_CLASS:
        case TK_ACTOR:
          return true;

        default: {}
      }
      break;
    }

    case TK_TYPEPARAMREF:
      return is_constructable(typeparam_constraint(type));

    case TK_VALUEFORMALPARAMREF:
      return is_constructable(ast_type(type));

    case TK_ARROW:
      return is_concrete(ast_childidx(type, 1));

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(type);
      if(unfolded == NULL)
        return false;
      bool ok = is_concrete(unfolded);
      ast_free_unattached(unfolded);
      return ok;
    }

    default: {}
  }

  pony_assert(0);
  return false;
}

bool is_known(ast_t* type)
{
  if(type == NULL)
    return false;

  switch(ast_id(type))
  {
    case TK_ENTITY_TYPE_CLASS:
    case TK_ENTITY_TYPE_STRUCT:
    case TK_ENTITY_TYPE_PRIMITIVE:
    case TK_ENTITY_TYPE_ACTOR:
    case TK_UNIONTYPE:
    case TK_TUPLETYPE:
      return false;

    case TK_ISECTTYPE:
    {
      ast_t* child = ast_child(type);

      while(child != NULL)
      {
        if(is_known(child))
          return true;

        child = ast_sibling(child);
      }

      return false;
    }

    case TK_NOMINAL:
    {
      ast_t* def = (ast_t*)ast_data(type);

      switch(ast_id(def))
      {
        case TK_INTERFACE:
        case TK_TRAIT:
          return false;

        case TK_PRIMITIVE:
        case TK_STRUCT:
        case TK_CLASS:
        case TK_ACTOR:
          return true;

        default: {}
      }
      break;
    }

    case TK_ARROW:
      return is_known(ast_childidx(type, 1));

    case TK_TYPEPARAMREF:
      return is_known(typeparam_constraint(type));

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(type);
      if(unfolded == NULL)
        return false;
      bool ok = is_known(unfolded);
      ast_free_unattached(unfolded);
      return ok;
    }

    default: {}
  }

  pony_assert(0);
  return false;
}

bool is_bare(ast_t* type, pass_opt_t* opt)
{
  // opt must be non-NULL: the TK_NOMINAL case interns "ponyint_bare" into
  // opt->strtab. Lookup paths that deliberately pass opt == NULL to skip
  // access-control checks must pass from == NULL instead (see genmatch.c), which
  // keeps the checks off without starving the subtype machinery of a table.
  pony_assert(opt != NULL);

  if(type == NULL)
    return false;

  switch(ast_id(type))
  {
    case TK_UNIONTYPE:
    case TK_ISECTTYPE:
    case TK_TUPLETYPE:
    {
      ast_t* child = ast_child(type);
      while(child != NULL)
      {
        if(is_bare(child, opt))
          return true;

        child = ast_sibling(child);
      }

      return false;
    }

    case TK_NOMINAL:
    {
      ast_t* def = (ast_t*)ast_data(type);
      return ast_has_annotation(def, "ponyint_bare", opt->strtab);
    }

    case TK_ARROW:
      return is_bare(ast_childidx(type, 1), opt);

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(type);
      if(unfolded == NULL)
        return false;
      bool ok = is_bare(unfolded, opt);
      ast_free_unattached(unfolded);
      return ok;
    }

    case TK_TYPEPARAMREF:
    case TK_VALUEFORMALPARAM:
    case TK_VALUEFORMALPARAMREF:
    case TK_VALUEFORMALARG:
    case TK_FUNTYPE:
    case TK_INFERTYPE:
    case TK_ERRORTYPE:
    case TK_DONTCARETYPE:
      return false;

    default:
    {
      if(is_value_formal_arg_literal(type))
      {
        return false;
      }
      break;
    }
  }

  pony_assert(0);
  return false;
}

bool is_top_type(ast_t* type, bool ignore_cap, pass_opt_t* opt)
{
  if(type == NULL)
    return false;

  switch(ast_id(type))
  {
    case TK_UNIONTYPE:
    case TK_ISECTTYPE:
    {
      ast_t* child = ast_child(type);

      while(child != NULL)
      {
        if(!is_top_type(child, ignore_cap, opt))
          return false;

        child = ast_sibling(child);
      }

      return true;
    }

    case TK_NOMINAL:
    {
      if(!ignore_cap && (ast_id(cap_fetch(type)) != TK_TAG))
        return false;

      // An empty interface is a top type.
      ast_t* def = (ast_t*)ast_data(type);

      if(ast_id(def) != TK_INTERFACE)
        return false;

      ast_t* members = ast_childidx(def, 4);

      if(ast_childcount(members) != 0)
        return false;

      return true;
    }

    case TK_ARROW:
    {
      if(ignore_cap)
        return is_top_type(ast_childidx(type, 1), true, opt);

      ast_t* type_lower = viewpoint_lower(type, opt);

      if(type_lower == NULL)
        return false;

      bool r = is_top_type(type_lower, false, opt);

      ast_free_unattached(type_lower);
      return r;
    }

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(type);
      if(unfolded == NULL)
        return false;
      bool ok = is_top_type(unfolded, ignore_cap, opt);
      ast_free_unattached(unfolded);
      return ok;
    }

    case TK_TUPLETYPE:
    case TK_TYPEPARAMREF:
    case TK_FUNTYPE:
    case TK_INFERTYPE:
    case TK_ERRORTYPE:
    case TK_DONTCARETYPE:
      return false;

    default : {}
  }

  pony_assert(0);
  return false;
}

bool is_entity(ast_t* type, token_id entity)
{
  if(type == NULL)
    return false;

  switch(ast_id(type))
  {
    case TK_TUPLETYPE:
      return false;

    case TK_UNIONTYPE:
    {
      ast_t* child = ast_child(type);

      while(child != NULL)
      {
        if(!is_entity(child, entity))
          return false;

        child = ast_sibling(child);
      }

      return true;
    }

    case TK_ISECTTYPE:
    {
      ast_t* child = ast_child(type);

      while(child != NULL)
      {
        if(is_entity(child, entity))
          return true;

        child = ast_sibling(child);
      }

      return false;
    }

    case TK_NOMINAL:
    {
      ast_t* def = (ast_t*)ast_data(type);
      return ast_id(def) == entity;
    }

    case TK_ARROW:
      return is_entity(ast_childidx(type, 1), entity);

    case TK_TYPEPARAMREF:
      return is_entity(typeparam_constraint(type), entity);

    case TK_TYPEALIASREF:
    {
      ast_t* unfolded = typealias_unfold(type);
      if(unfolded == NULL)
        return false;
      bool ok = is_entity(unfolded, entity);
      ast_free_unattached(unfolded);
      return ok;
    }

    default: {}
  }

  pony_assert(0);
  return false;
}

bool contains_dontcare(ast_t* ast)
{
  switch(ast_id(ast))
  {
    case TK_DONTCARETYPE:
      return true;

    case TK_TUPLETYPE:
    {
      ast_t* child = ast_child(ast);

      while(child != NULL)
      {
        if(contains_dontcare(child))
          return true;

        child = ast_sibling(child);
      }

      return false;
    }

    default: {}
  }

  return false;
}

bool contains_struct(ast_t* type)
{
  switch(ast_id(type))
  {
    case TK_UNIONTYPE:
    case TK_ISECTTYPE:
    {
      ast_t* child = ast_child(type);

      while(child != NULL)
      {
        if(contains_struct(child))
        {
          return true;
        }

        child = ast_sibling(child);
      }

      break;
    }

    case TK_NOMINAL:
    {
      ast_t* def = (ast_t*)ast_data(type);
      if(ast_id(def) == TK_STRUCT)
      {
        return true;
      }

      break;
    }

    default:
      break;
  }

  return false;
}

bool contains_entity_type(ast_t* type)
{
  switch(ast_id(type))
  {
    case TK_UNIONTYPE:
    case TK_ISECTTYPE:
    {
      ast_t* child = ast_child(type);

      while(child != NULL)
      {
        if(contains_entity_type(child))
        {
          return true;
        }

        child = ast_sibling(child);
      }

      break;
    }

    case TK_ENTITY_TYPE_CLASS:
    case TK_ENTITY_TYPE_STRUCT:
    case TK_ENTITY_TYPE_PRIMITIVE:
    case TK_ENTITY_TYPE_ACTOR:
      return true;

    default:
      break;
  }

  return false;
}

bool type_is_only_cap(ast_t* type, token_id cap)
{
  switch(ast_id(type))
  {
    case TK_UNIONTYPE:
    case TK_ISECTTYPE:
    {
      ast_t* child = ast_child(type);

      while(child != NULL)
      {
        if(!type_is_only_cap(child, cap))
        {
          return false;
        }

        child = ast_sibling(child);
      }

      break;
    }

    case TK_NOMINAL:
      if(ast_id(ast_childidx(type, 3)) != cap)
      {
        return false;
      }
      break;

    default:
      pony_assert(false);
      break;
  }

  return true;
}

ast_t* remove_entity_types(ast_t* type)
{
  switch(ast_id(type))
  {
    case TK_UNIONTYPE:
    case TK_ISECTTYPE:
    {
      ast_t* child = ast_child(type);

      ast_t* new_chain = ast_from(type, ast_id(type));

      while(child != NULL)
      {
        ast_t* ret_type = remove_entity_types(child);
        if(ret_type != NULL)
        {
          ast_append(new_chain, ret_type);
        }

        child = ast_sibling(child);
      }

      size_t child_count = ast_childcount(new_chain);
      if(child_count == 0)
      {
        ast_free_unattached(new_chain);
        return NULL;
      }
      else if(child_count == 1)
      {
        ast_t* only_one_type = ast_dup(ast_child(new_chain));
        ast_free_unattached(new_chain);
        return only_one_type;
      }
      else
      {
        return new_chain;
      }

      break;
    }

    case TK_ENTITY_TYPE_CLASS:
    case TK_ENTITY_TYPE_STRUCT:
    case TK_ENTITY_TYPE_PRIMITIVE:
    case TK_ENTITY_TYPE_ACTOR:
      return NULL;

    default:
      break;
  }

  return ast_dup(type);
}


bool is_pointer_referenced_object(ast_t* type)
{
  if(is_machine_word(type))
  {
    return false;
  }
  else if(is_pointer(type) || is_nullable_pointer(type))
  {
    return true;
  }

  switch(ast_id(type))
  {
    case TK_UNIONTYPE:
    case TK_ISECTTYPE:
      return true;

    case TK_NOMINAL:
    {
      ast_t* def = (ast_t*)ast_data(type);
      token_id underlying = ast_id(def);
      if(underlying == TK_STRUCT || underlying == TK_CLASS || underlying == TK_ACTOR ||
         underlying == TK_TRAIT || underlying == TK_INTERFACE)
      {
        return true;
      }

      break;
    }

    default:
      break;
  }

  return false;
}


bool is_allowed_pointer_conversion(ast_t* l_type, ast_t* r_type, pass_opt_t* opt, errorframe_t* frame)
{
  if(ast_id(r_type) == TK_TYPEALIASREF)
  {
    ast_t* unfolded = typealias_unfold(r_type);
    if(unfolded == NULL)
    {
      return false;
    }
    
    r_type = unfolded;
  }

  if(ast_id(l_type) == TK_TYPEALIASREF)
  {
    ast_t* unfolded = typealias_unfold(l_type);
    if(unfolded == NULL)
    {
      return false;
    }

    l_type = unfolded;
  }

  if(is_bare(l_type, opt))
  {
    // Implicit pointer to lambda conversion
    if(is_pointer(r_type))
    {
      ast_t* pointer_typeargs = ast_childidx(r_type, 2);
      ast_t* pointer_typearg = ast_child(pointer_typeargs);

      if(is_none(pointer_typearg) || is_eqtype(pointer_typearg, l_type, frame, opt))
      {
        return true;
      }
    }
  }
  else if(is_pointer(l_type))
  {
    // Implicit lambda to pointer conversion
    if(is_bare(r_type, opt))
    {
      ast_t* pointer_typeargs = ast_childidx(l_type, 2);
      ast_t* pointer_typearg = ast_child(pointer_typeargs);

      if(is_none(pointer_typearg) || is_eqtype(pointer_typearg, r_type, frame, opt))
      {
        return true;
      }
    }
    // Implicit struct/class/actor to pointer conversion

    ast_t* r_def = (ast_t*)ast_data(r_type);

    if(ast_id(r_type) == TK_NOMINAL &&
       (ast_id(r_def) == TK_STRUCT || ast_id(r_def) == TK_CLASS || ast_id(r_def) == TK_ACTOR))
    {
      ast_t* pointer_elem_type = ast_child(ast_childidx(l_type, 2));

      if(is_none(pointer_elem_type) || is_eqtype(pointer_elem_type, r_type, frame, opt))
      {
        return true;
      }
    }
  }

  // Implicit pointer to struct/class/actor conversion

  ast_t* l_def = (ast_t*)ast_data(l_type);

  if(ast_id(l_type) == TK_NOMINAL &&
    (ast_id(l_def) == TK_STRUCT || ast_id(l_def) == TK_CLASS || ast_id(l_def) == TK_ACTOR))
  {
    if(is_pointer(r_type))
    {
      ast_t* pointer_elem_type = ast_child(ast_childidx(r_type, 2));

      if(is_none(pointer_elem_type) || is_eqtype(pointer_elem_type, l_type, frame, opt))
      {
        return true;
      }
    }
  }

  return false;
}