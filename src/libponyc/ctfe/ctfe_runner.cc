#include "ctfe_runner.h"
#include "ctfe_value_struct.h"

#include "../ast/astbuild.h"
#include "../type/subtype.h"
#include "ponyassert.h"
#include "../type/lookup.h"


using namespace std;

CtfeRunner::CtfeRunner(pass_opt_t* opt)
{
  CtfeValueTypeRef::initialize(opt);
}


CtfeRunner::~CtfeRunner()
{
  for (auto& elem : m_allocated)
  {
    switch(elem.get_type())
    {
      case CtfeValue::Type::StructRef:
        delete elem.get_struct_ref();
        break;
      case CtfeValue::Type::Pointer:
        elem.get_pointer().delete_array_pointer();
        break;
      default:
        break;
    }
  }
}

void CtfeRunner::add_allocated_reference(const CtfeValue& ref)
{
  m_allocated.push_back(ref);
}

bool CtfeRunner::populate_struct_members(pass_opt_t* opt, errorframe_t* errors,
  CtfeValueStruct* s, ast_t* members)
{
  ast_t* member = ast_child(members);

  while(member != NULL)
  {
    switch(ast_id(member))
    {
      case TK_FVAR:
      case TK_FLET:
      {
        const char* var_name = ast_name(ast_child(member));
        if(!s->new_value(var_name, CtfeValue()))
        {
          return false;
        }
        break;
      }
      case TK_EMBED:
      {
        ast_t* embed_type = ast_type(member);
        ast_t* underlying_type = (ast_t*)ast_data(embed_type);
        CtfeValue embed_val;

        if(ast_id(underlying_type) == TK_STRUCT ||
           ast_id(underlying_type) == TK_CLASS)
        {
          CtfeValueStruct* embed_s = new CtfeValueStruct;
          embed_val = CtfeValue(embed_s);
          add_allocated_reference(embed_val);

          ast_t* embed_members = ast_childidx(underlying_type, 4);

          bool r = populate_struct_members(opt, errors, embed_s, embed_members);
          if(!r)
          {
            return false;
          }
        }
        else
        {
          return false;
        }

        const char* var_name = ast_name(ast_child(member));
        if(!s->new_value(var_name, embed_val))
        {
          return false;
        }

        break;
      }
      default:
        break;
    }

    member = ast_sibling(member);
  }

  return true;
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
  CtfeValue return_value;
  ast_t* typeargs = NULL;

  switch(ast_id(receiver))
  {
    case TK_DOT:
    {
      ast_t* lh = ast_child(receiver);
      rec_val = evaluate(opt, errors, lh, depth + 1);
      ast_t* rh = ast_childidx(receiver, 1);
      method_name = ast_name(rh);
      recv_type = ast_type(lh);

      break;
    }
    case TK_NEWREF:
    case TK_NEWBEREF:
    {
      if(ast_id(ast_childidx(receiver, 1)) == TK_TYPEARGS)
      {
        typeargs = ast_childidx(receiver, 1);
        receiver = ast_child(receiver);
      }

      AST_GET_CHILDREN(receiver, typeref, new_name);
      recv_type = ast_type(typeref);
      method_name = ast_name(new_name);

      ast_t* underlying_type = (ast_t*)ast_data(recv_type);
      if(ast_id(underlying_type) == TK_STRUCT ||
         ast_id(underlying_type) == TK_CLASS)
      {
        if(::is_pointer(recv_type))
        {
          //Get the type for the pointer
          ast_t* pointer_type = ast_child(ast_childidx(recv_type, 2));
          CtfeValueTypeRef typeref = CtfeValueTypeRef(pointer_type);
          rec_val = CtfeValuePointer(typeref);
        }
        else
        {
          CtfeValueStruct* s = new CtfeValueStruct;
          rec_val = CtfeValue(s);
          add_allocated_reference(rec_val);

          ast_t* members = ast_childidx(underlying_type, 4);
          bool r = populate_struct_members(opt, errors, s, members);
          if(!r)
          {
            ast_error_frame(errors, ast,
                "Failed to populate struct/class members");
            throw CtfeFailToEvaluateException();
          }
        }
      }
      break;
    }
    case TK_FUNREF:
    {
      if(ast_id(ast_childidx(receiver, 1)) == TK_TYPEARGS)
      {
        typeargs = ast_childidx(receiver, 1);
        receiver = ast_child(receiver);
      }

      AST_GET_CHILDREN(receiver, rec_type, pos_args);
      rec_val = evaluate(opt, errors, rec_type, depth + 1);
      method_name = ast_name(ast_sibling(rec_type));
      recv_type = ast_type(rec_type);
      break;
    }
    default:
      ast_error_frame(errors, ast,
              "Unsupported function call receiver, '%s'", ast_get_print(receiver));
      throw CtfeFailToEvaluateException();
  }

  ast_t* argument = ast_child(positional_args);
  while(argument != NULL)
  {
    CtfeValue evaluated_arg = evaluate(opt, errors, argument, depth + 1);

    args.push_back(evaluated_arg);
    argument = ast_sibling(argument);
  }

  if(method_name != NULL)
  {
    CtfeValue result;
    bool ret = CtfeValue::run_method(opt, errors, ast, rec_val, args, method_name, result, *this);
    if(ret)
    {
      return result;
    }

    if(recv_type != NULL)
    {
      deferred_reification_t* looked_up = lookup(opt, ast, recv_type, method_name);
      if(looked_up != NULL)
      {
        // we don't know if this method has been visited before.
        if(!ast_passes_subtree(&looked_up->ast, opt, PASS_EXPR))
        {
          throw CtfeFailToEvaluateException();
        }

        ast_t* looked_up_ast = looked_up->ast;
        ast_t* reified_lookedup_ast = NULL;

        if(typeargs != NULL)
        {
          ast_t* typeparams = ast_childidx(looked_up_ast, 2);
          deferred_reify_add_method_typeparams(looked_up, typeparams, typeargs, opt);
        }

        if(looked_up->method_typeargs != NULL || looked_up->type_typeargs != NULL ||
           looked_up->thistype != NULL)
        {
          reified_lookedup_ast = deferred_reify(looked_up, looked_up_ast, opt);
          looked_up_ast = reified_lookedup_ast;
        }

        m_frames.push_frame();

        if(!rec_val.is_none())
        {
          m_frames.new_value("this", rec_val);
        }

        ast_t* params = ast_childidx(looked_up_ast, 3);
        if(ast_id(params) != TK_NONE)
        {
          ast_t* curr_param = ast_child(params);
          for (const auto& arg: args)
          {
            const char* param_name = ast_name(ast_child(curr_param));
            m_frames.new_value(param_name, arg);
            curr_param = ast_sibling(curr_param);
          }
        }

        ast_t* seq = ast_childidx(looked_up_ast, 6);
        if(ast_id(ast_child(seq)) != TK_COMPILE_INTRINSIC)
        {
          return_value = evaluate(opt, errors, seq, depth + 1);
          if(return_value.get_control_flow_modifier() == CtfeValue::ControlFlowModifier::Return)
          {
            return_value.clear_control_flow_modifier();
          }
        }
        else
        {
          ast_error_frame(errors, ast,
            "Unsupported compiler intrinsic function");
          throw CtfeFailToEvaluateException();
        }

        m_frames.pop_frame();

        if(reified_lookedup_ast != NULL)
        {
          ast_free_unattached(reified_lookedup_ast);
        }
        deferred_reify_free(looked_up);
      }
    }
  }

  switch(ast_id(receiver))
  {
    case TK_NEWREF:
      return rec_val;
    default:
      return return_value;
  }
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
    case TK_DONTCARE:
    case TK_DONTCAREREF:
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
    case TK_STRING:
      return CtfeValue(CtfeValueStringLiteral(ast_name(expression)));
    case TK_THIS:
    {
      CtfeValue this_ref;
      if(!m_frames.get_value("this", this_ref))
      {
        ast_error_frame(errors, expression,
          "'this' reference was not previously not defined");
        throw CtfeFailToEvaluateException();
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
        throw CtfeFailToEvaluateException();
      }

      return var_ref;
    }

    case TK_VALUEFORMALARG:
      return evaluate(opt, errors, ast_child(expression), depth);

    case TK_CALL:
      return evaluate_method(opt, errors, expression, depth);

    case TK_SEQ:
    {
      CtfeValue evaluated;
      for(ast_t* p = ast_child(expression); p != NULL; p = ast_sibling(p))
      {
        evaluated = evaluate(opt, errors, p, depth + 1);

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

      pony_assert(condition_evaluated.get_type() == CtfeValue::Type::Bool);

      return condition_evaluated.get_bool().get_value() ?
            evaluate(opt, errors, then_branch, depth + 1):
            evaluate(opt, errors, else_branch, depth + 1);
    }

    case TK_RETURN:
    {
      AST_GET_CHILDREN(expression, return_expr);
      return evaluate(opt, errors, return_expr, depth + 1);
    }

    case TK_VAR:
    case TK_LET:
    {
      const char* var_name = ast_name(ast_child(expression));
      if(!m_frames.new_value(var_name, CtfeValue()))
      {
        ast_error_frame(errors, expression,
          "CTFE runner tried to create symbol %s twice in the same frame", var_name);
        throw CtfeFailToEvaluateException();
      }

      return CtfeValue();
    }

    case TK_VARREF:
    case TK_LETREF:
    {
      const char* var_name = ast_name(ast_child(expression));
      CtfeValue found_value;
      if (!m_frames.get_value(var_name, found_value))
      {
        ast_error_frame(errors, expression,
          "Symbol %s was not found because it was not previously created", var_name);
        throw CtfeFailToEvaluateException();
      }
      return found_value;
    }

    case TK_FVARREF:
    case TK_EMBEDREF:
    case TK_FLETREF:
    {
      AST_GET_CHILDREN(expression, receiver, id);
      CtfeValue evaluated_receiver = evaluate(opt, errors, receiver, depth + 1);

      if(evaluated_receiver.is_none())
      {
        ast_error_frame(errors, expression,
          "Cannot access variables in 'this' the comptime expression frame.");
        throw CtfeFailToEvaluateException();
      }

      const char* var_name = ast_name(id);

      CtfeValue found_value;
      switch(evaluated_receiver.get_type())
      {
        case CtfeValue::Type::StructRef:
        {
          CtfeValueStruct* s = evaluated_receiver.get_struct_ref();
          if (!s->get_value(var_name, found_value))
          {
            ast_error_frame(errors, expression,
              "Symbol '%s' was not found in struct, this should not happen", var_name);
            throw CtfeFailToEvaluateException();
          }
          break;
        }
        default:
          ast_error_frame(errors, expression,
            "Unsupported field variable type");
          throw CtfeFailToEvaluateException();
      }

      return found_value;
    }

    case TK_ASSIGN:
    {
      AST_GET_CHILDREN(expression, left, right);

      const char* var_name = NULL;
      bool new_value = false;

      CtfeValue left_value = evaluate(opt, errors, left, depth + 1);
      CtfeValue right_value = evaluate(opt, errors, right, depth + 1);

      CtfeValue ret = left_side_assign(opt, errors, left, right_value, depth + 1);

      return left_value;
    }

    case TK_WHILE:
    {
      AST_GET_CHILDREN(expression, cond, thenbody, elsebody);

      CtfeValue evaluated_cond = evaluate(opt, errors, cond, depth + 1);

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
      if(evaluated_try.get_control_flow_modifier() == CtfeValue::ControlFlowModifier::Error)
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

      }while(evaluated_cond.get_bool().get_value() == false);

      if(repeat_value.get_control_flow_modifier() == CtfeValue::ControlFlowModifier::Continue)
      {

        repeat_value = evaluate(opt, errors, elsebody, depth + 1);
      }

      return repeat_value;
    }

    case TK_TUPLE:
    {
      ast_t* elem = ast_child(expression);

      // First go through and check how many members
      size_t tuple_size = 0;
      while(elem != NULL)
      {
        tuple_size++;
        elem = ast_sibling(elem);
      }

      CtfeValueTuple tuple = CtfeValueTuple(tuple_size);

      elem = ast_child(expression);
      size_t i = 0;
      while(elem != NULL)
      {
        CtfeValue evaluated_elem = evaluate(opt, errors, elem, depth + 1);

        if(!tuple.update_value(i, evaluated_elem))
        {
          ast_error_frame(errors, expression,
            "Creating tuple failed, tuple index out of range");
          throw CtfeFailToEvaluateException();
        }

        i++;
        elem = ast_sibling(elem);
      }

      return CtfeValue(tuple);
    }

    case TK_TUPLEELEMREF:
    {
      AST_GET_CHILDREN(expression, receiver, nr);

      CtfeValue evaluated_rec = evaluate(opt, errors, receiver, depth + 1);

      pony_assert(evaluated_rec.get_type() == CtfeValue::Type::Tuple);

      lexint_t* lexpos = ast_int(nr);
      size_t tuple_pos = (size_t)lexpos->low;

      CtfeValue ret;
      CtfeValueTuple& tuple = evaluated_rec.get_tuple();
      if(!tuple.get_value(tuple_pos, ret))
      {
        ast_error_frame(errors, expression,
          "Accessing tuple value failed, tuple index out of range");
        throw CtfeFailToEvaluateException();
      }


      return ret;
    }

    default:
      ast_error_frame(errors, expression,
        "The CTFE runner does not support the '%s' expression",
        ast_get_print(expression));
      throw CtfeFailToEvaluateException();
  }

  throw CtfeFailToEvaluateException();
}


CtfeValue CtfeRunner::left_side_assign(pass_opt_t* opt, errorframe_t* errors,
  ast_t* left, CtfeValue& right_val, int depth)
{
  switch(ast_id(left))
  {
    case TK_VAR:
    case TK_LET:
    case TK_VARREF:
    case TK_LETREF:
    {
      const char* var_name = ast_name(ast_child(left));
      if (!m_frames.update_value(var_name, right_val))
      {
        ast_error_frame(errors, left,
          "Symbol %s was not found or previously set", var_name);
        throw CtfeFailToEvaluateException();
      }
      break;
    }

    case TK_FVARREF:
    case TK_EMBEDREF:
    case TK_FLETREF:
    {
      AST_GET_CHILDREN(left, receiver, id);
      CtfeValue evaluated_receiver = evaluate(opt, errors, receiver, depth + 1);

      const char* var_name = ast_name(id);

      switch(evaluated_receiver.get_type())
      {
        case CtfeValue::Type::StructRef:
        {
          CtfeValueStruct* s = evaluated_receiver.get_struct_ref();
          if (!s->update_value(var_name, right_val))
          {
            ast_error_frame(errors, left,
              "Could not update symbol '%s' was not found in struct, this "
              "should not happen", var_name);
            throw CtfeFailToEvaluateException();
          }
          break;
        }
        default:
          ast_error_frame(errors, left,
            "Unsupported field variable type");
          throw CtfeFailToEvaluateException();
      }
      break;
    }
    case TK_TUPLE:
    {
      pony_assert(right_val.get_type() == CtfeValue::Type::Tuple);

      ast_t* seq_elem = ast_child(left);
      size_t i = 0;
      while(seq_elem != NULL)
      {
        ast_t* elem = ast_child(seq_elem);

        CtfeValue right_tuple_value;
        CtfeValueTuple& tuple = right_val.get_tuple();
        if(!tuple.get_value(i, right_tuple_value))
        {
          ast_error_frame(errors, left,
            "Accessing tuple value failed, tuple index out of range");
          throw CtfeFailToEvaluateException();
        }

        CtfeValue ret = left_side_assign(opt, errors, elem, right_tuple_value, depth + 1);

        i++;
        seq_elem = ast_sibling(seq_elem);
      }
      break;
    }
    case TK_DONTCAREREF:
      break;
    default:
      ast_error_frame(errors, left,
        "Unsupported assignment receiver type");
      throw CtfeFailToEvaluateException();
  }

  return CtfeValue();
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

  // Insert an empty this reference so that we can at least call functions inside the
  // current object. However it is not possible to access any variables.
  m_frames.new_value("this", CtfeValue());

  // We can't evaluate expressions which still have references to value
  // parameters so we simply stop, indicating no error yet.
  //if(contains_valueparamref(expression))
  //  return true;

  // evaluate the expression passing NULL as 'this' as we aren't
  // evaluating a method on an object
  errorframe_t errors = NULL;

  try
  {
    CtfeValue evaluated = evaluate(opt, &errors, expression, 0);
    ast_t* new_literal = evaluated.create_ast_literal_node(opt, &errors, ast);
    if(new_literal != NULL)
    {
      ast_replace(astp, new_literal);
      return true;
    }
  }
  catch(const CtfeException& ex)
  {
    if(ast_type(expression) != NULL)
    {
      ast_settype(ast, ast_from(ast_type(expression), TK_ERRORTYPE));
    }
    else
    {
      ast_settype(ast, ast_blank(TK_ERRORTYPE));
    }
    errorframe_t frame = NULL;
    ast_error_frame(&frame, ast, "could not evaluate compile time expression");
    errorframe_append(&frame, &errors);
    errorframe_report(&frame, opt->check.errors);
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

  return false;
}
