#include "ctfe_runner.h"
#include "ctfe_value_aggregate.h"
#include "../ast/astbuild.h"
#include "../type/subtype.h"
#include "ponyassert.h"
#include "../type/lookup.h"


using namespace std;

CtfeRunner::CtfeRunner(pass_opt_t* opt)
{
  CtfeValue::initialize(opt);
}


CtfeRunner::~CtfeRunner()
{
  for (auto elem : m_allocated)
  {
    switch(elem.get_type())
    {
      case CtfeValue::Type::AggRef:
        delete elem.get_agg_ref();
        break;
      default:
        break;
    }
  }
}

void CtfeRunner::add_allocated_reference(CtfeValue& ref)
{
  m_allocated.push_back(ref);
}

CtfeValue CtfeRunner::evaluate_method(pass_opt_t* opt, errorframe_t* errors,
  ast_t* ast, int depth)
{
  AST_GET_CHILDREN(ast, receiver, positional_args, named_args, question);

  // named arguments have already been converted to positional
  pony_assert(ast_id(named_args) == TK_NONE);

  vector<CtfeValue> args;
  CtfeValue rec_val;
  ast_t* recv_type = NULL;
  const char* method_name = NULL;
  CtfeValue return_value = CtfeValue(CtfeValue::Type::ComptimeError);

  switch(ast_id(receiver))
  {
    case TK_DOT:
    {
      ast_t* lh = ast_child(receiver);
      rec_val = evaluate(opt, errors, lh, depth + 1);
      if(rec_val.has_comptime_error())
      {
        return rec_val;
      }
      ast_t* rh = ast_childidx(receiver, 1);
      method_name = ast_name(rh);
      recv_type = ast_type(lh);

      break;
    }
    case TK_NEWREF:
    {
      AST_GET_CHILDREN(receiver, typeref, new_name);
      recv_type = ast_type(typeref);
      method_name = ast_name(new_name);
      break;
    }
    case TK_FUNREF:
    {
      AST_GET_CHILDREN(receiver, rec_type, pos_args);
      rec_val = evaluate(opt, errors, rec_type, depth + 1);
      if(rec_val.has_comptime_error())
      {
        return rec_val;
      }
      method_name = ast_name(ast_sibling(rec_type));
      recv_type = ast_type(rec_type);
      break;
    }
    default:
      ast_error_frame(errors, ast,
              "Unsupported function call receiver");
      return CtfeValue(CtfeValue::Type::ComptimeError);
  }

  if(!rec_val.is_none())
  {
    args.push_back(rec_val);
  }

  ast_t* argument = ast_child(positional_args);
  while(argument != NULL)
  {
    CtfeValue evaluated_arg = evaluate(opt, errors, argument, depth + 1);
    if(evaluated_arg.has_comptime_error())
    {
      return evaluated_arg;
    }
    //if(eval_error(evaluated_arg))
    //  return evaluated_arg;

    args.push_back(evaluated_arg);
    argument = ast_sibling(argument);
  }

  if(method_name != NULL)
  {
    CtfeValue result;
    bool ret = CtfeValue::run_method(opt, errors, ast, args, method_name, result);
    if(ret)
    {
      return result;
    }

    if(recv_type != NULL)
    {
      deferred_reification_t* looked_up = lookup(opt, ast, recv_type, method_name);
      if(looked_up != NULL)
      {
        m_frames.push_frame();

        if(!rec_val.is_none())
        {
          m_frames.new_value("this", rec_val);
        }

        ast_t* params = ast_childidx(looked_up->ast, 3);
        if(ast_id(params) != TK_NONE)
        {
          ast_t* curr_param = ast_child(params);

          // We need to begin with the second arguement because the first is
          // the "class" pointer. Only if there is a such pointer.
          auto it = args.cbegin();
          if(!rec_val.is_none())
          {
            ++it;
          }
          for (;it != args.end(); ++it)
          {
            const CtfeValue arg = *it;
            const char* param_name = ast_name(ast_child(curr_param));
            m_frames.new_value(param_name, arg);
            curr_param = ast_sibling(curr_param);
          }
        }

        // we don't know if this method has been visited before.
        if(ast_passes_subtree(&looked_up->ast, opt, PASS_EXPR))
        {
          ast_t* seq = ast_childidx(looked_up->ast, 6);
          if(ast_id(ast_child(seq)) != TK_COMPILE_INTRINSIC)
          {
            return_value = evaluate(opt, errors, seq, depth + 1);
            if(return_value.has_comptime_error())
            {
              return return_value;
            }
            else if(return_value.get_control_flow_modifier() == CtfeValue::ControlFlowModifier::Return)
            {
              return_value.clear_control_flow_modifier();
            }
          }
          else
          {
            ast_error_frame(errors, ast,
              "Unsupported compiler intrinsic function");
          }
        }

        m_frames.pop_frame();

        deferred_reify_free(looked_up);
      }
    }
  }

  return return_value;
}

CtfeValue CtfeRunner::evaluate(pass_opt_t* opt, errorframe_t* errors, ast_t* expression, int depth)
{
  /*if(depth >= opt->evaluation_depth)
  {
    ast_error_frame(errors, expression,
      "compile-time expression evaluation depth exceeds maximum of %d",
       opt->evaluation_depth);
    return NULL;
  }*/

  switch(ast_id(expression))
  {
    case TK_NONE:
      return CtfeValue();
    case TK_TRUE:
      return CtfeValue(CtfeValueBool(true));
    case TK_FALSE:
      return CtfeValue(CtfeValueBool(false));
    // Literal cases where we can return the value
    case TK_INT:
    {
      CtfeValue val = CtfeValue(CtfeValueIntLiteral(*ast_int(expression)));
      ast_t* val_type = ast_type(expression);
      if(ast_id(val_type) != TK_LITERAL)
      {
        val = CtfeValue(val.get_int_literal(), ast_name(ast_childidx(val_type, 1)));
      }

      return val;
    }

    case TK_THIS:
    {
      CtfeValue this_ref;
      if(!m_frames.get_value("this", this_ref))
      {
        ast_error_frame(errors, expression,
          "\"this\" reference was not previously not defined");
        return CtfeValue(CtfeValue::Type::ComptimeError);
      }
      return this_ref;
    }

    case TK_PARAMREF:
    {
      CtfeValue var_ref;
      const char* var_name = ast_name(ast_child(expression));
      if(!m_frames.get_value(var_name, var_ref))
      {
        ast_error_frame(errors, expression,
          "Parameter with name %s was not found in this frame", var_name);
        return CtfeValue(CtfeValue::Type::ComptimeError);
      }

      return var_ref;
    }

    case TK_CALL:
      return evaluate_method(opt, errors, expression, depth);

    case TK_SEQ:
    {
      CtfeValue evaluated;
      for(ast_t* p = ast_child(expression); p != NULL; p = ast_sibling(p))
      {
        evaluated = evaluate(opt, errors, p, depth + 1);
        if(evaluated.has_comptime_error())
        {
          break;
        }

        if(evaluated.get_control_flow_modifier() != CtfeValue::ControlFlowModifier::None)
        {
          break;
        }
      }
      return evaluated;
    }

    case TK_IF:
    {
      AST_GET_CHILDREN(expression, condition, then_branch, else_branch);
      CtfeValue condition_evaluated = evaluate(opt, errors, condition, depth + 1);
      if(condition_evaluated.has_comptime_error())
      {
        return condition_evaluated;
      }

      //if(eval_error(condition_evaluated))
      //  return condition_evaluated;

      pony_assert(condition_evaluated.get_type() == CtfeValue::Type::Bool);

      return condition_evaluated.get_bool().get_value() ?
            evaluate(opt, errors, then_branch, depth + 1):
            evaluate(opt, errors, else_branch, depth + 1);
    }

    case TK_RETURN:
    {
      AST_GET_CHILDREN(expression, return_expr);
      CtfeValue ret = evaluate(opt, errors, return_expr, depth + 1);
      if(ret.has_comptime_error())
      {
        return ret;
      }
      ret.set_control_flow_modifier(CtfeValue::ControlFlowModifier::Return);
      return ret;
    }

    case TK_ASSIGN:
    {
      AST_GET_CHILDREN(expression, right, left);

      const char* var_name = NULL;
      bool new_value = false;

      switch(ast_id(right))
      {
        case TK_VAR:
        case TK_LET:
        {
          var_name = ast_name(ast_child(right));
          if(!m_frames.new_value(var_name, CtfeValue()))
          {
            ast_error_frame(errors, expression,
              "Symbol %s was already previously created in this frame", var_name);
            return CtfeValue(CtfeValue::Type::ComptimeError);
          }
          new_value = true;
          break;
        }
        case TK_VARREF:
        case TK_LETREF:
        {
          var_name = ast_name(ast_child(right));
          break;
        }
        default:
          break;
      }

      CtfeValue ret = evaluate(opt, errors, left, depth + 1);
      if(ret.has_comptime_error())
      {
        return ret;
      }

      CtfeValue old_value;
      if(!m_frames.update_value(var_name, ret, old_value))
      {
        ast_error_frame(errors, expression,
          "Symbol %s was not found or previously set", var_name);
        return CtfeValue(CtfeValue::Type::ComptimeError);
      }
      return new_value ? CtfeValue() : old_value;
    }

    case TK_VARREF:
    case TK_LETREF:
    {
      ast_t* id = ast_child(expression);
      const char* var_name = ast_name(id);
      CtfeValue ret;
      if(!m_frames.get_value(var_name, ret))
      {
        ast_error_frame(errors, expression,
          "Symbol %s was not found or previously set", var_name);
        return CtfeValue(CtfeValue::Type::ComptimeError);
      }
      return ret;
    }

    case TK_WHILE:
    {
      AST_GET_CHILDREN(expression, cond, thenbody, elsebody);
      CtfeValue evaluated_cond = evaluate(opt, errors, cond, depth + 1);
      if(evaluated_cond.has_comptime_error())
      {
        return evaluated_cond;
      }

      pony_assert(evaluated_cond.get_type() == CtfeValue::Type::Bool);

      // the condition didn't hold on the first iteration so we evaluate the
      // else
      if(evaluated_cond.get_bool().get_value() == false)
      {
        return evaluate(opt, errors, elsebody, depth + 1);
      }

      // the condition held so evaluate the while returning the file iteration
      // result as the evaluated result
      CtfeValue evaluated_while;
      while(evaluated_cond.get_bool().get_value() == true)
      {
        evaluated_while = evaluate(opt, errors, thenbody, depth + 1);
        if(evaluated_while.has_comptime_error())
        {
          break;
        }

        CtfeValue::ControlFlowModifier modifier = evaluated_while.get_control_flow_modifier();
        if(modifier == CtfeValue::ControlFlowModifier::Return ||
           modifier == CtfeValue::ControlFlowModifier::Error)
        {
          break;
        }
        else if(modifier == CtfeValue::ControlFlowModifier::Break)
        {
          evaluated_while.clear_control_flow_modifier();
          break;
        }
        else if(modifier == CtfeValue::ControlFlowModifier::Continue)
        {
          evaluated_while.clear_control_flow_modifier();
        }

        evaluated_cond = evaluate(opt, errors, cond, depth + 1);
        if(evaluated_cond.has_comptime_error())
        {
          return evaluated_cond;
        }
      }

      return evaluated_while;
    }

    case TK_CONTINUE:
    {
      CtfeValue ret;
      ret.set_control_flow_modifier(CtfeValue::ControlFlowModifier::Continue);
      return ret;
    }

    case TK_BREAK:
    {
      AST_GET_CHILDREN(expression, break_expr);
      CtfeValue ret = evaluate(opt, errors, break_expr, depth + 1);
      if(ret.has_comptime_error())
      {
        return ret;
      }
      ret.set_control_flow_modifier(CtfeValue::ControlFlowModifier::Break);
      return ret;
    }

    case TK_ERROR:
    {
      CtfeValue ret;
      ret.set_control_flow_modifier(CtfeValue::ControlFlowModifier::Error);
      return ret;
    }

    case TK_TRY:
    {
      AST_GET_CHILDREN(expression, trybody, elsebody);
      CtfeValue evaluated_try = evaluate(opt, errors, trybody, depth + 1);
      if(!evaluated_try.has_comptime_error() &&
         evaluated_try.get_control_flow_modifier() == CtfeValue::ControlFlowModifier::Error)
      {
        evaluated_try = evaluate(opt, errors, elsebody, depth + 1);
      }

      return evaluated_try;
    }

    case TK_REPEAT:
    {
      AST_GET_CHILDREN(expression, repeat_body, cond, elsebody);
      CtfeValue evaluated_cond = evaluate(opt, errors, cond, depth + 1);

      CtfeValue repeat_value;

      do
      {
        repeat_value = evaluate(opt, errors, repeat_body, depth + 1);

        CtfeValue::ControlFlowModifier modifier = repeat_value.get_control_flow_modifier();
        if(modifier == CtfeValue::ControlFlowModifier::Return ||
           modifier == CtfeValue::ControlFlowModifier::Error)
        {
          break;
        }
        else if(modifier == CtfeValue::ControlFlowModifier::Break)
        {
          repeat_value.clear_control_flow_modifier();
          break;
        }

        evaluated_cond = evaluate(opt, errors, cond, depth + 1);
        if(evaluated_cond.has_comptime_error())
        {
          return evaluated_cond;
        }

      }while(evaluated_cond.get_bool().get_value() == false);

      if(repeat_value.get_control_flow_modifier() == CtfeValue::ControlFlowModifier::Continue)
      {

        repeat_value = evaluate(opt, errors, elsebody, depth + 1);
      }

      return repeat_value;
    }

    default:
      ast_error_frame(errors, expression,
        "expression interpeter does not support this expression");
      return CtfeValue(CtfeValue::Type::ComptimeError);
  }

  return CtfeValue(CtfeValue::Type::ComptimeError);
}


bool CtfeRunner::run(pass_opt_t* opt, ast_t** astp)
{
  ast_t* ast = *astp;
  /*ast_t* cached = search_cache(ast);
  if(cached != NULL)
  {
    ast_replace(astp, cached);
    return true;
  }*/

  //ast_setconstant(ast);
  //ast_t* expression = ast;

  ast_t* expression = ast_child(ast);

  // Insert a this reference for good measure
  CtfeValue base_this = CtfeValue(new CtfeValueAggregate);
  add_allocated_reference(base_this);
  m_frames.new_value("this", base_this);

  // We can't evaluate expressions which still have references to value
  // parameters so we simply stop, indicating no error yet.
  //if(contains_valueparamref(expression))
  //  return true;

  // evaluate the expression passing NULL as 'this' as we aren't
  // evaluating a method on an object
  errorframe_t errors = NULL;
  CtfeValue evaluated = evaluate(opt, &errors, expression, 0);

  // We may not have errored in a couple of ways, NULL, is some error that
  // occured as we could not evaluate the expression
  if(evaluated.has_comptime_error())
  {
    ast_settype(ast, ast_from(ast_type(expression), TK_ERRORTYPE));
    errorframe_t frame = NULL;
    ast_error_frame(&frame, ast, "could not evaluate compile time expression");
    errorframe_append(&frame, &errors);
    errorframe_report(&frame, opt->check.errors);
    //opt->check.evaluation_error = true;
    return false;
  }

  // TK_ERROR means we encountered some error expression and it hasn't been
  // resolved. We will error out on this case, using this to provide static
  // assertions.
  /*if(eval_error(evaluated) && ast_id(evaluated) == TK_ERROR)
  {
    ast_settype(ast, ast_from(ast_type(expression), TK_ERRORTYPE));
    ast_error(opt->check.errors, expression,
              "unresolved error occurred during evaluation");
    ast_error_continue(opt->check.errors, evaluated,
              "error originated from here");
    opt->check.evaluation_error = true;
    return false;
  }

  ast_setconstant(evaluated);

  // Our result may contain a valueparamref, e.g. a lookup of a variable
  // assigned to a value paramter
  if(contains_valueparamref(evaluated))
    return true;

  ast_t* type = ast_type(evaluated);
  // See if we can recover the expression to val capability
  if(!is_type_literal(type))
  {
    type = recover_type(type, TK_VAL);
    if(type == NULL)
    {
      ast_error(opt->check.errors, expression,
        "can't recover compile-time object to val capability");
      opt->check.evaluation_error = true;
      return false;
    }
  }
  ast_settype(ast, type);

  // cache the result of evaluation so that we don't evaluate the same
  // expressions again and then rewrite the AST
  cache_ast(ast, evaluated);
  ast_replace(astp, evaluated);*/

  ast_t* new_literal = evaluated.create_ast_literal_node(opt, ast);
  ast_replace(astp, new_literal);

  return true;
}
