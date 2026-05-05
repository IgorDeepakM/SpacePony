#include "reify.h"
#include "subtype.h"
#include "typealias.h"
#include "viewpoint.h"
#include "assemble.h"
#include "alias.h"
#include "../ast/token.h"
#include "../ast/astbuild.h"
#include "../../libponyrt/gc/serialise.h"
#include "../../libponyrt/mem/pool.h"
#include "ponyassert.h"
#include "../expr/literal.h"
#include "../pass/expr.h"
#include "typeparam.h"
#include <string.h>


static ast_t* find_typearg(pass_opt_t* opt, ast_t* ast, ast_t* typeparams, ast_t* typeargs,
  ast_t** found_typeparam)
{
  ast_t* ref_def = NULL;
  switch(ast_id(ast))
  {
    case TK_REFERENCE:
      ref_def = ast_get(ast, ast_name(ast_child(ast)), NULL);
      if(ref_def == NULL)
        return NULL;

      // Follow the ast_data chain to the root for TK_TYPEPARAM nodes,
      // same as the TK_TYPEPARAMREF case below.
      if(ast_id(ref_def) == TK_TYPEPARAM || ast_id(ref_def) == TK_VALUEFORMALPARAM)
      {
        while((ast_t*)ast_data(ref_def) != ref_def)
          ref_def = (ast_t*)ast_data(ref_def);
      }

      break;

    case TK_VALUEFORMALPARAMREF:
    {
      ref_def = (ast_t*)ast_data(ast);
      if(ref_def == NULL)
      {
        ref_def = ast_get(ast, ast_name(ast_child(ast)), NULL);
      }
      
      if(ref_def == NULL)
      {
        return NULL;
      }

      break;
    }

    case TK_TYPEPARAMREF:
      ref_def = (ast_t*)ast_data(ast);
      pony_assert(ref_def != NULL);
      // Follow the ast_data chain to the root (self-referential node set
      // during the scope pass). A single-level follow is insufficient when
      // type parameters are copied multiple times, e.g. a lambda inside an
      // object literal inside a generic method.
      while((ast_t*)ast_data(ref_def) != ref_def)
        ref_def = (ast_t*)ast_data(ref_def);
      break;

    default:
      pony_assert(0);
  }

  // Iterate pairwise through the typeparams and typeargs,
  // until we find the one corresponding to this ref
  ast_t* typeparam = ast_child(typeparams);
  ast_t* typearg = ast_child(typeargs);

  while((typeparam != NULL) && (typearg != NULL))
  {
    if(ast_id(typeparam) == TK_TYPEPARAM)
    {
      ast_t* param_def = (ast_t*)ast_data(typeparam);
      pony_assert(param_def != NULL);
      while((ast_t*)ast_data(param_def) != param_def)
        param_def = (ast_t*)ast_data(param_def);

      if(ref_def == param_def)
      {
        if(found_typeparam != NULL)
        {
          *found_typeparam = param_def;
        }
        return typearg;
      }

      if(ast_id(ast) == TK_TYPEPARAMREF)
      {
        AST_GET_CHILDREN(param_def, param_name, param_constraint);
        AST_GET_CHILDREN(ref_def, ref_name, ref_constraint);

        if(ast_name(ref_name) == ast_name(param_name))
        {
          if((ast_id(param_constraint) == TK_TYPEPARAMREF) ||
            is_subtype(ref_constraint, param_constraint, NULL, opt))
          {
            if(found_typeparam != NULL)
            {
              *found_typeparam = param_def;
            }
            return typearg;
          }
        }
      }
    }
    else if(ast_id(typeparam) == TK_VALUEFORMALPARAM)
    {
      if(ref_def == typeparam)
      {
        if(found_typeparam != NULL)
        {
          *found_typeparam = typeparam;
        }
        return typearg;
      }

      if(ast_id(ast) == TK_VALUEFORMALPARAMREF)
      {
        AST_GET_CHILDREN(typeparam, param_name, param_constraint);
        AST_GET_CHILDREN(ref_def, ref_name);

        if(ast_name(ref_name) == ast_name(param_name))
        {
          if(found_typeparam != NULL)
          {
            *found_typeparam = typeparam;
          }
          return typearg;
        }
      }
    }

    // Not the type variable we are looking for, move on to next
    typeparam = ast_sibling(typeparam);
    typearg = ast_sibling(typearg);
  }

  return NULL;
}

static void reify_typeparamref(pass_opt_t* opt, ast_t** astp, ast_t* typeparams, ast_t* typeargs)
{
  ast_t* ast = *astp;
  pony_assert(ast_id(ast) == TK_TYPEPARAMREF);

  ast_t* typearg = find_typearg(opt, ast, typeparams, typeargs, NULL);
  if (typearg == NULL)
    return;

  // Keep ephemerality.
  switch(ast_id(ast_childidx(ast, 2)))
  {
    case TK_EPHEMERAL:
    {
      ast_t* new_typearg = consume_type(typearg, TK_NONE, true);
      // Will be NULL when instantiation produces double ephemerals A^^
      // or equivalent.
      //
      // This might result from perfectly sound code,
      // but probably implies a signature is too weak
      // (e.g. returning T instead of T^).
      //
      // It could be avoided if we require a constraint on the parameter
      // for this case.
      if (new_typearg != NULL)
      {
        typearg = new_typearg;
      }
      break;
    }

    case TK_NONE:
      break;

    case TK_ALIASED:
      typearg = alias(typearg);
      break;

    default:
      pony_assert(0);
  }

  ast_t* annotation = ast_dup(ast_annotation(ast));
  ast_replace(astp, typearg);

  // We need to apply the byval annotation again as it was
  // replaced away.
  if(annotation != NULL)
  {
    ast_setannotation(*astp, annotation);
  }
}

static void reify_valueformalparamref(pass_opt_t* opt, ast_t** astp, ast_t* typeparams, ast_t* typeargs)
{
  ast_t* ast = *astp;
  pony_assert(ast_id(ast) == TK_VALUEFORMALPARAMREF);

  ast_t* found_typeparam = NULL;
  ast_t* typearg = find_typearg(opt, ast, typeparams, typeargs, &found_typeparam);
  if(typearg == NULL)
    return;

  if(ast_id(typearg) == TK_VALUEFORMALARG)
  {
    ast_t* lit = ast_child(typearg);
    ast_t* typeparam_type = ast_childidx(found_typeparam, 1);

    ast_t* literal_type = ast_type(lit);
    if(literal_type == NULL)
    {
      // Why do we need to run the pass_expr and coerce_literals here?
      // When reifying and replacing the TK_VALUEFORMALPARAMREF with
      // a TK_VALUEFORMALARG the literals might not have been processed
      // yet becuase it originates from something later on in the AST tree

      // Here we make literals out of the expression
      if(ast_visit(&lit, NULL, pass_expr, opt, PASS_EXPR) != AST_OK)
      {
        pony_assert(false);
        return;
      }

      // Let's coerce the literal
      if(!coerce_literals(&lit, typeparam_type, opt))
      {
        pony_assert(false);
        return;
      }
    }

    ast_setdata(typearg, found_typeparam);
    ast_settype(typearg, typeparam_type);
  }
  ast_replace(astp, typearg);
}

static void reify_arrow(ast_t** astp)
{
  ast_t* ast = *astp;
  pony_assert(ast_id(ast) == TK_ARROW);
  AST_GET_CHILDREN(ast, left, right);

  ast_t* r_left = left;
  ast_t* r_right = right;

  if(ast_id(left) == TK_ARROW)
  {
    AST_GET_CHILDREN(left, l_left, l_right);
    r_left = l_left;
    r_right = viewpoint_type(l_right, right);
  }

  ast_t* r_type = viewpoint_type(r_left, r_right);
  ast_replace(astp, r_type);
}

static void reify_reference(pass_opt_t* opt, ast_t** astp, ast_t* typeparams, ast_t* typeargs)
{
  ast_t* ast = *astp;
  pony_assert(ast_id(ast) == TK_REFERENCE);

  ast_t* found_typeparam = NULL;
  ast_t* typearg = find_typearg(opt, ast, typeparams, typeargs, &found_typeparam);
  if (typearg == NULL)
    return;

  if(ast_id(found_typeparam) == TK_VALUEFORMALPARAM)
  {
    if(ast_id(typearg) == TK_VALUEFORMALARG)
    {
      ast_t* type = ast_childidx(found_typeparam, 1);
      if(ast_id(type) == TK_TYPEPARAMREF)
      {
        type = find_typearg(opt, type, typeparams, typeargs, &found_typeparam);
      }
      ast_settype(typearg, type);
    }
    ast_replace(astp, typearg);
  }
  else
  {
    ast_setid(ast, TK_TYPEREF);
    ast_add(ast, ast_from(ast, TK_NONE));    // 1st child: package reference
    ast_append(ast, ast_from(ast, TK_NONE)); // 3rd child: type args
    ast_settype(ast, typearg);
  }
}

bool reify_defaults(ast_t* typeparams, ast_t* typeargs, bool errors,
  pass_opt_t* opt)
{
  pony_assert(
    (ast_id(typeparams) == TK_TYPEPARAMS) ||
    (ast_id(typeparams) == TK_NONE)
    );
  pony_assert(
    (ast_id(typeargs) == TK_TYPEARGS) ||
    (ast_id(typeargs) == TK_NONE)
    );

  size_t param_count = ast_childcount(typeparams);
  size_t arg_count = ast_childcount(typeargs);

  if(param_count == arg_count)
    return true;

  if(param_count < arg_count)
  {
    if(errors)
    {
      ast_error(opt->check.errors, typeargs, "too many type arguments");
      ast_error_continue(opt->check.errors, typeparams, "definition is here");
    }

    return false;
  }

  // Pick up default type arguments if they exist.
  ast_setid(typeargs, TK_TYPEARGS);
  ast_t* typeparam = ast_childidx(typeparams, arg_count);

  while(typeparam != NULL)
  {
    ast_t* defarg = ast_childidx(typeparam, 2);

    if(ast_id(defarg) == TK_NONE)
      break;

    // We might have to visit the literals for the default arguments because
    // they might not have been processed yet in the AST tree. This depends on
    // where they are defined in the code.
    if(ast_id(defarg) == TK_VALUEFORMALARG && ast_type(ast_child(defarg)) == NULL &&
       opt->program_pass == PASS_EXPR)
    {
      ast_t* lit_child = ast_child(defarg);
      if(ast_visit(&lit_child, NULL, pass_expr, opt, PASS_EXPR) != AST_OK)
      {
        return false;
      }
    }

    ast_append(typeargs, defarg);

    typeparam = ast_sibling(typeparam);
  }

  if(typeparam != NULL)
  {
    if(errors)
    {
      ast_error(opt->check.errors, typeargs, "not enough type arguments");
      ast_error_continue(opt->check.errors, typeparams, "definition is here");
    }

    return false;
  }

  return true;
}

static void reify_ast(ast_t** astp, ast_t* typeparams, ast_t* typeargs, pass_opt_t* opt)
{
  ast_t* ast = *astp;
  ast_t* child = ast_child(ast);

  while(child != NULL)
  {
    reify_ast(&child, typeparams, typeargs, opt);
    child = ast_sibling(child);
  }

  ast_t* type = ast_type(ast);

  if(type != NULL)
    reify_ast(&type, typeparams, typeargs, opt);

  switch(ast_id(ast))
  {
    case TK_TYPEPARAMREF:
      reify_typeparamref(opt, astp, typeparams, typeargs);
      break;

    case TK_VALUEFORMALPARAMREF:
      reify_valueformalparamref(opt, astp, typeparams, typeargs);
      break;

    case TK_ARROW:
      reify_arrow(astp);
      break;

    case TK_REFERENCE:
      reify_reference(opt, astp, typeparams, typeargs);
      break;

    default: {}
  }
}

ast_t* reify(ast_t* ast, ast_t* typeparams, ast_t* typeargs, pass_opt_t* opt,
  bool duplicate)
{
  (void)opt;
  pony_assert(
    (ast_id(typeparams) == TK_TYPEPARAMS) ||
    (ast_id(typeparams) == TK_NONE)
  );
  pony_assert(
    (ast_id(typeargs) == TK_TYPEARGS) ||
    (ast_id(typeargs) == TK_NONE)
  );

  ast_t* r_ast;
  if(duplicate)
    r_ast = ast_dup(ast);
  else
    r_ast = ast;

  reify_ast(&r_ast, typeparams, typeargs, opt);
  return r_ast;
}

ast_t* reify_method_def(ast_t* ast, ast_t* typeparams, ast_t* typeargs,
  pass_opt_t* opt)
{
  switch(ast_id(ast))
  {
    case TK_FUN:
    case TK_BE:
    case TK_NEW:
      break;

    default:
      pony_assert(false);
  }

  // Do not duplicate the body and docstring.
  bool dup_child[9] = {true, true, true, true, true, true, false, false, true};
  ast_t* r_ast = ast_dup_partial(ast, dup_child, true, true, true);
  return reify(r_ast, typeparams, typeargs, opt, false);
}

deferred_reification_t* deferred_reify_new(ast_t* ast, ast_t* typeparams,
  ast_t* typeargs, ast_t* thistype)
{
  pony_assert(((typeparams != NULL) && (typeargs != NULL)) ||
    ((typeparams == NULL) && (typeargs == NULL)));

  deferred_reification_t* deferred = POOL_ALLOC(deferred_reification_t);

  deferred->ast = ast;
  deferred->type_typeparams = ast_dup(typeparams);
  deferred->type_typeargs = ast_dup(typeargs);
  deferred->thistype = ast_dup(thistype);
  deferred->method_typeparams = NULL;
  deferred->method_typeargs = NULL;

  return deferred;
}

void deferred_reify_add_method_typeparams(deferred_reification_t* deferred,
  ast_t* typeparams, ast_t* typeargs, pass_opt_t* opt)
{
  pony_assert((deferred->method_typeparams == NULL) &&
    (deferred->method_typeargs == NULL));

  pony_assert(((typeparams != NULL) && (typeargs != NULL)) ||
    ((typeparams == NULL) && (typeargs == NULL)));

  if(typeparams == NULL)
    return;

  ast_t* r_typeparams = ast_dup(typeparams);
  deferred->method_typeargs = ast_dup(typeargs);

  // Must replace `this` before typeparam reification.
  if(deferred->thistype != NULL)
    r_typeparams = viewpoint_replacethis(r_typeparams, deferred->thistype,
      false);

  if(deferred->type_typeparams != NULL)
    r_typeparams = reify(r_typeparams, deferred->type_typeparams,
      deferred->type_typeargs, opt, false);

  deferred->method_typeparams = r_typeparams;
}

ast_t* deferred_reify(deferred_reification_t* deferred, ast_t* ast,
  pass_opt_t* opt)
{
  ast_t* r_ast = ast_dup(ast);

  // Must replace `this` before typeparam reification.
  if(deferred->thistype != NULL)
    r_ast = viewpoint_replacethis(r_ast, deferred->thistype, false);

  if(deferred->type_typeparams != NULL)
    r_ast = reify(r_ast, deferred->type_typeparams, deferred->type_typeargs,
      opt, false);

  if(deferred->method_typeparams != NULL)
    r_ast = reify(r_ast, deferred->method_typeparams, deferred->method_typeargs,
      opt, false);

  return r_ast;
}

ast_t* deferred_reify_method_def(deferred_reification_t* deferred, ast_t* ast,
  pass_opt_t* opt)
{
  switch(ast_id(ast))
  {
    case TK_FUN:
    case TK_BE:
    case TK_NEW:
      break;

    default:
      pony_assert(false);
  }

  // Do not duplicate the body and docstring.
  bool dup_child[9] = {true, true, true, true, true, true, false, false, true};
  ast_t* r_ast = ast_dup_partial(ast, dup_child, true, true, true);

  // Must replace `this` before typeparam reification.
  if(deferred->thistype != NULL)
    r_ast = viewpoint_replacethis(r_ast, deferred->thistype, false);

  if(deferred->type_typeparams != NULL)
    r_ast = reify(r_ast, deferred->type_typeparams, deferred->type_typeargs,
      opt, false);

  if(deferred->method_typeparams != NULL)
    r_ast = reify(r_ast, deferred->method_typeparams, deferred->method_typeargs,
      opt, false);

  return r_ast;
}

deferred_reification_t* deferred_reify_dup(deferred_reification_t* deferred)
{
  if(deferred == NULL)
    return NULL;

  deferred_reification_t* copy = POOL_ALLOC(deferred_reification_t);

  copy->ast = deferred->ast;
  copy->type_typeparams = ast_dup(deferred->type_typeparams);
  copy->type_typeargs = ast_dup(deferred->type_typeargs);
  copy->method_typeparams = ast_dup(deferred->method_typeparams);
  copy->method_typeargs = ast_dup(deferred->method_typeargs);
  copy->thistype = ast_dup(deferred->thistype);

  return copy;
}

void deferred_reify_free(deferred_reification_t* deferred)
{
  if(deferred != NULL)
  {
    ast_free_unattached(deferred->type_typeparams);
    ast_free_unattached(deferred->type_typeargs);
    ast_free_unattached(deferred->method_typeparams);
    ast_free_unattached(deferred->method_typeargs);
    ast_free_unattached(deferred->thistype);
    POOL_FREE(deferred_reification_t, deferred);
  }
}

static bool compatible_argument(ast_t* typeparam, ast_t* typearg)
{
  bool is_value_argument = ast_id(typearg) == TK_VALUEFORMALARG ||
    (ast_id(typearg) == TK_VALUEFORMALPARAMREF);

  token_id typeparam_id = ast_id(typeparam);

  return (typeparam_id == TK_VALUEFORMALPARAM && is_value_argument) ||
         (typeparam_id == TK_TYPEPARAM && !is_value_argument);
}

bool check_constraints(ast_t* orig, ast_t* typeparams, ast_t* typeargs,
  bool report_errors, pass_opt_t* opt)
{
  ast_t* typeparam = ast_child(typeparams);
  ast_t* typearg = ast_child(typeargs);

  while(typeparam != NULL)
  {
    if (is_bare(typearg))
    {
      if(report_errors)
      {
        ast_error(opt->check.errors, typearg,
          "a bare type cannot be used as a type argument");
      }

      return false;
    }

    switch(ast_id(typearg))
    {
      case TK_NOMINAL:
      {
        ast_t* def = (ast_t*)ast_data(typearg);

        if(ast_id(def) == TK_STRUCT && !ast_has_annotation(typeparam, "allowstruct"))
        {
          if(report_errors)
          {
            ast_error(opt->check.errors, typearg,
              "a struct cannot be used as a type argument");
          }

          return false;
        }
        break;
      }

      case TK_TYPEPARAMREF:
      {
        ast_t* def = (ast_t*)ast_data(typearg);

        if(def == typeparam)
        {
          typeparam = ast_sibling(typeparam);
          typearg = ast_sibling(typearg);
          continue;
        }
        break;
      }

      case TK_TYPEALIASREF:
      {
        // Unfold to check if the underlying type is a struct.
        ast_t* unfolded = typealias_unfold(typearg);

        if(unfolded != NULL)
        {
          if(ast_id(unfolded) == TK_NOMINAL)
          {
            ast_t* def = (ast_t*)ast_data(unfolded);

            if((def != NULL) && (ast_id(def) == TK_STRUCT) &&
               !ast_has_annotation(typeparam, "allowstruct"))
            {
              ast_free_unattached(unfolded);

              if(report_errors)
              {
                ast_error(opt->check.errors, typearg,
                  "a struct cannot be used as a type argument");
              }

              return false;
            }
          }

          ast_free_unattached(unfolded);
        }
        break;
      }

      default: {}
    }

    if (!compatible_argument(typeparam, typearg))
    {
      token_id typeparam_id = ast_id(typeparam);
      bool is_typeparam = typeparam_id == TK_TYPEPARAM;
      ast_error(opt->check.errors, orig,
        "invalid parameterisation");
      ast_error_continue(opt->check.errors, typeparam,
        "expected %s argument", is_typeparam ? "type" : "value");
      ast_error_continue(opt->check.errors, typearg,
        "provided %s argument", is_typeparam ? "value" : "type");
      return false;
    }

    // Reify the constraint.
    ast_t* constraint = ast_childidx(typeparam, 1);
    ast_t* r_constraint = reify(constraint, typeparams, typeargs, opt,
      true);

    // A bound type must be a subtype of the constraint.
    errorframe_t info = NULL;
    errorframe_t* infop = (report_errors ? &info : NULL);

    // A bound type must be a subtype of the constraint.
    if(ast_id(typearg) == TK_VALUEFORMALARG)
    {
      ast_t* literal = ast_child(typearg);

      if (!coerce_literals(&literal, r_constraint, opt))
        return false;

      ast_t* value_type = ast_type(literal);
      if (is_typecheck_error(value_type))
        return false;

      if (!is_subtype(value_type, r_constraint, report_errors ? &info : NULL, opt))
      {
        if (report_errors)
        {
          ast_error(opt->check.errors, orig,
            "value argument type is outside its constraint");
          ast_error_continue(opt->check.errors, literal,
            "argument type: %s", ast_print_type(ast_type(literal)));
          ast_error_continue(opt->check.errors, typeparam,
            "constraint: %s", ast_print_type(r_constraint));
        }

        ast_free_unattached(r_constraint);
        return false;
      }
    }
    else if(!is_subtype_constraint(typearg, r_constraint, infop, opt))
    {
      if(report_errors)
      {
        errorframe_t frame = NULL;
        ast_error_frame(&frame, orig,
          "type argument is outside its constraint");
        ast_error_frame(&frame, typearg,
          "argument: %s", ast_print_type(typearg));
        ast_error_frame(&frame, typeparam,
          "constraint: %s", ast_print_type(r_constraint));
        errorframe_append(&frame, &info);
        errorframe_report(&frame, opt->check.errors);
      }

      ast_free_unattached(r_constraint);
      return false;
    }

    ast_free_unattached(r_constraint);

    // A constructable constraint can only be fulfilled by a concrete typearg.
    if (is_constructable(constraint) && ast_id(typearg) != TK_VALUEFORMALARG &&
       !is_concrete(typearg))
    {
      if (report_errors)
      {
        ast_error(opt->check.errors, orig, "a constructable constraint can "
          "only be fulfilled by a concrete type argument");
        ast_error_continue(opt->check.errors, typearg, "argument: %s",
          ast_print_type(typearg));
        ast_error_continue(opt->check.errors, typeparam, "constraint: %s",
          ast_print_type(constraint));
      }

      return false;
    }

    typeparam = ast_sibling(typeparam);
    typearg = ast_sibling(typearg);
  }

  pony_assert(typeparam == NULL);
  pony_assert(typearg == NULL);
  return true;
}

static void deferred_reification_serialise_trace(pony_ctx_t* ctx, void* object)
{
  deferred_reification_t* d = (deferred_reification_t*)object;

  pony_traceknown(ctx, d->ast, ast_pony_type(), PONY_TRACE_MUTABLE);

  if(d->type_typeparams != NULL)
    pony_traceknown(ctx, d->type_typeparams, ast_pony_type(),
      PONY_TRACE_MUTABLE);

  if(d->type_typeargs != NULL)
    pony_traceknown(ctx, d->type_typeargs, ast_pony_type(),
      PONY_TRACE_MUTABLE);

  if(d->method_typeparams != NULL)
    pony_traceknown(ctx, d->method_typeparams, ast_pony_type(),
      PONY_TRACE_MUTABLE);

  if(d->method_typeargs != NULL)
    pony_traceknown(ctx, d->method_typeargs, ast_pony_type(),
      PONY_TRACE_MUTABLE);

  if(d->thistype != NULL)
    pony_traceknown(ctx, d->thistype, ast_pony_type(),
      PONY_TRACE_MUTABLE);
}

static void deferred_reification_serialise(pony_ctx_t* ctx, void* object,
  void* buf, size_t offset, int mutability)
{
  (void)mutability;

  deferred_reification_t* d = (deferred_reification_t*)object;
  deferred_reification_t* dst =
    (deferred_reification_t*)((uintptr_t)buf + offset);

  dst->ast = (ast_t*)pony_serialise_offset(ctx, d->ast);
  dst->type_typeparams = (ast_t*)pony_serialise_offset(ctx, d->type_typeparams);
  dst->type_typeargs = (ast_t*)pony_serialise_offset(ctx, d->type_typeargs);
  dst->method_typeparams = (ast_t*)pony_serialise_offset(ctx,
    d->method_typeparams);
  dst->method_typeargs = (ast_t*)pony_serialise_offset(ctx, d->method_typeargs);
  dst->thistype = (ast_t*)pony_serialise_offset(ctx, d->thistype);
}

static void deferred_reification_deserialise(pony_ctx_t* ctx, void* object)
{
  deferred_reification_t* d = (deferred_reification_t*)object;

  d->ast = (ast_t*)pony_deserialise_offset(ctx, ast_pony_type(),
    (uintptr_t)d->ast);
  d->type_typeparams = (ast_t*)pony_deserialise_offset(ctx, ast_pony_type(),
    (uintptr_t)d->type_typeparams);
  d->type_typeargs = (ast_t*)pony_deserialise_offset(ctx, ast_pony_type(),
    (uintptr_t)d->type_typeargs);
  d->method_typeparams = (ast_t*)pony_deserialise_offset(ctx, ast_pony_type(),
    (uintptr_t)d->method_typeparams);
  d->method_typeargs = (ast_t*)pony_deserialise_offset(ctx, ast_pony_type(),
    (uintptr_t)d->method_typeargs);
  d->thistype = (ast_t*)pony_deserialise_offset(ctx, ast_pony_type(),
    (uintptr_t)d->thistype);
}

static pony_type_t deferred_reification_pony =
{
  0,
  sizeof(deferred_reification_t),
  0,
  0,
  0,
  NULL,
#if defined(USE_RUNTIME_TRACING)
  NULL,
  NULL,
#endif
  NULL,
  deferred_reification_serialise_trace,
  deferred_reification_serialise,
  deferred_reification_deserialise,
  NULL,
  NULL,
  NULL,
  NULL,
  0,
  0,
  NULL,
  NULL,
  NULL
};

pony_type_t* deferred_reification_pony_type()
{
  return &deferred_reification_pony;
}
