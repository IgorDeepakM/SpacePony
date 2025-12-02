#include "ctfe_runner.h"
#include "ctfe_value_struct.h"
#include "ctfe_exception.h"

#include "../ast/ast.h"
#include "../type/subtype.h"
#include "ponyassert.h"
#include "../type/lookup.h"
#include "../type/assemble.h"

#include <thread>


using namespace std;

CtfeRunner::CtfeRunner(pass_opt_t* opt):
  m_max_recursion_depth{opt->ctfe_max_recursion},
  m_current_recursion_depth{0},
  m_max_duration_exceeded{false},
  m_stop_thread{false}
{
  CtfeAstType::initialize(opt);
}


CtfeRunner::~CtfeRunner()
{
  for(auto& elem : m_allocated)
  {
    if(elem.is_struct())
    {
      if(elem.is_pointer())
      {
        elem.get_pointer().delete_array_pointer();
      }
      else
      {
        delete elem.get_struct_ref();
      }
    }
  }

  for(auto& elem : m_cached_ast)
  {
    ast_free_unattached(elem.second);
  }
}


ast_t* CtfeRunner::get_reified_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast_pos,
  ast_t* recv_type, ast_t* typeargs, const char* method_name)
{
  uint64_t type_hash = CtfeAstType::ast_hash(recv_type);
  uint64_t hash = ponyint_hash_str(method_name) ^ type_hash;
  if(typeargs != nullptr)
  {
    hash ^= CtfeAstType::ast_hash(typeargs);
  }

  auto it = m_cached_ast.find(hash);
  if(it == m_cached_ast.end())
  {
    deferred_reification_t* looked_up = lookup_try(opt, ast_pos, recv_type, method_name, true);
    if(looked_up == nullptr)
    {
      ast_error_frame(errors, ast_pos,
        "CTFE: could not find the method '%s'", method_name);
      throw CtfeFailToEvaluateException();
    }

    ast_t* looked_up_ast = looked_up->ast;
    ast_t* reified_lookedup_ast = nullptr;

    if(typeargs != nullptr)
    {
      ast_t* typeparams = ast_childidx(looked_up_ast, 2);
      deferred_reify_add_method_typeparams(looked_up, typeparams, typeargs, opt);
    }

    if(looked_up->method_typeargs != nullptr || looked_up->type_typeargs != nullptr ||
        looked_up->thistype != nullptr)
    {
      reified_lookedup_ast = deferred_reify(looked_up, looked_up_ast, opt);
    }
    else
    {
      reified_lookedup_ast = ast_dup(looked_up_ast);
    }

    deferred_reify_free(looked_up);

    m_cached_ast.insert({hash, reified_lookedup_ast});
    return reified_lookedup_ast;
  }
  else
  {
    return it->second;
  }
}


ast_t* CtfeRunner::get_builtin_type(pass_opt_t* opt, ast_t* ast_pos, const char* type_name)
{
  uint64_t hash = ponyint_hash_str(type_name);
  auto it = m_cached_ast.find(hash);
  if(it == m_cached_ast.end())
  {
    ast_t* type_ast = type_builtin(opt, ast_pos, "Bool");
    m_cached_ast.insert({hash, type_ast});

    return type_ast;
  }
  else
  {
    return it->second;
  }
}


ast_t* CtfeRunner::cache_and_get_built_type(ast_t* ast_type)
{
  uint64_t hash = CtfeAstType::ast_hash(ast_type);
  auto it = m_cached_ast.find(hash);
  if(it == m_cached_ast.end())
  {
    m_cached_ast.insert({ hash, ast_type });
    return ast_type;
  }
  else
  {
    ast_free_unattached(ast_type);
    return it->second;
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

  while(member != nullptr)
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
          CtfeValueStruct* embed_s = new CtfeValueStruct(embed_type);
          embed_val = CtfeValue(embed_s, embed_type);
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


CtfeValue CtfeRunner::call_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast_pos,
  ast_t* res_type, const char* method_name, ast_t* recv_type, CtfeValue& recv_val,
  const vector<CtfeValue>& args, ast_t* typeargs)
{
  CtfeValue result;
  bool ret = CtfeValue::run_method(opt, errors, ast_pos, res_type, recv_val, args,
    method_name, result, *this);
  if(ret)
  {
    return result;
  }

  pony_assert(recv_type != nullptr);

  // If the underlying type is an interface it is a possible lamda
  ast_t* recv_underlying_type = (ast_t*)ast_data(recv_type);
  if(recv_underlying_type != nullptr)
  {
    switch(ast_id(recv_underlying_type))
    {
      case TK_INTERFACE:
        recv_type = recv_val.get_type_ast();
        break;
      default:
        break;
    }
  }

  bool failed = false;

  ast_t* looked_up_ast = get_reified_method(opt, errors, ast_pos, recv_type, typeargs, method_name);

  m_frames.push_frame();

  if(!recv_val.is_empty())
  {
    m_frames.new_value("this", recv_val);
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

  CtfeValue return_value;

  ast_t* seq = ast_childidx(looked_up_ast, 6);
  if(ast_id(seq) != TK_NONE && ast_id(ast_child(seq)) != TK_COMPILE_INTRINSIC)
  {
    return_value = evaluate(opt, errors, seq);
    if(return_value.get_control_flow_modifier() == CtfeValue::ControlFlowModifier::Return)
    {
      return_value.clear_control_flow_modifier();
    }
  }
  else
  {
    if(ast_id(seq) == TK_NONE)
    {
      // This means that we haven't found the proper virtual method
      pony_assert(false);
    }
    else if(ast_id(ast_child(seq)) == TK_COMPILE_INTRINSIC)
    {
      ast_error_frame(errors, ast_pos,
        "Unsupported compiler intrinsic function '%s", method_name);
    }
    failed = true;
  }

  m_frames.pop_frame();

  if(failed)
  {
    throw CtfeFailToEvaluateException();
  }

  return return_value;
}


CtfeValue CtfeRunner::evaluate_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast)
{
  AST_GET_CHILDREN(ast, receiver, positional_args, named_args, question);

  // named arguments have already been converted to positional
  pony_assert(ast_id(named_args) == TK_NONE);

  vector<CtfeValue> args;
  CtfeValue rec_val;
  ast_t* recv_type = nullptr;
  const char* method_name = nullptr;
  CtfeValue return_value;
  ast_t* typeargs = nullptr;

  switch(ast_id(receiver))
  {
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
          rec_val = CtfeValue(CtfeValuePointer(recv_type), recv_type);
        }
        else
        {
          CtfeValueStruct* s = new CtfeValueStruct(recv_type);
          rec_val = CtfeValue(s, recv_type);
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
      else if(ast_id(underlying_type) == TK_PRIMITIVE)
      {
        rec_val = CtfeValue(recv_type);
      }
      else if(ast_id(underlying_type) == TK_ACTOR)
      {
        ast_error_frame(errors, ast,
          "Calling actor behaviours is not supported in CTFE.");
        throw CtfeFailToEvaluateException();
      }
      else
      {
        pony_assert(false);
      }
      break;
    }
    case TK_FUNREF:
    case TK_FUNCHAIN:
    {
      if(ast_id(ast_childidx(receiver, 1)) == TK_TYPEARGS)
      {
        typeargs = ast_childidx(receiver, 1);
        receiver = ast_child(receiver);
      }

      ast_t* rec_expr = ast_child(receiver);
      rec_val = evaluate(opt, errors, rec_expr);
      method_name = ast_name(ast_sibling(rec_expr));
      recv_type = ast_type(rec_expr);
      break;
    }
    default:
      ast_error_frame(errors, ast,
              "Unsupported function call receiver, '%s'", ast_get_print(receiver));
      throw CtfeFailToEvaluateException();
  }

  ast_t* argument = ast_child(positional_args);
  while(argument != nullptr)
  {
    CtfeValue evaluated_arg = evaluate(opt, errors, argument);

    args.push_back(evaluated_arg);
    argument = ast_sibling(argument);
  }

  pony_assert(method_name != nullptr);

  ast_t* result_type = ast_type(ast);

  m_current_recursion_depth++;
  if(m_current_recursion_depth >= m_max_recursion_depth)
  {
    ast_error_frame(errors, ast,
      "compile time expression evaluation reached maximum method recursion depth of %lu.",
      m_current_recursion_depth);
    throw CtfeFailToEvaluateException();
  }

  return_value = call_method(opt, errors, ast, result_type, method_name, recv_type, rec_val, args,
    typeargs);

  m_current_recursion_depth--;

  if(is_machine_word(result_type) || ::is_pointer(result_type))
  {
    return return_value;
  }
  else
  {
    switch(ast_id(receiver))
    {
      case TK_NEWREF:
      case TK_NEWBEREF:
      case TK_FUNCHAIN:
        return rec_val;
      default:
        return return_value;
    }
  }
}

CtfeValue CtfeRunner::evaluate(pass_opt_t* opt, errorframe_t* errors, ast_t* expression)
{
  // When debugging you might want to comment this out because it is likely to time out
  // during a debugging session.
  if(m_max_duration_exceeded)
  {
    ast_error_frame(errors, expression,
      "compile time expression evaluation exceeded the maximum duration of %lu seconds",
      opt->ctfe_max_duration);
    throw CtfeFailToEvaluateException();
  }

  switch(ast_id(expression))
  {
    case TK_NONE:
    case TK_DONTCARE:
    case TK_DONTCAREREF:
      return CtfeValue(ast_type(expression));
    case TK_TRUE:
    case TK_FALSE:
    {
      ast_t* bool_type = ast_type(expression);
      CtfeValue ret;
      bool val = ast_id(expression) == TK_TRUE ? true : false;
      if(bool_type != nullptr)
      {
        ret = CtfeValue(CtfeValueBool(val), bool_type);
      }
      else
      {
        bool_type = get_builtin_type(opt, expression, "Bool");
        ret = CtfeValue(CtfeValueBool(val), bool_type);
      }

      return ret;
    }

    // Literal cases where we can return the value
    case TK_INT:
      return CtfeValue(*ast_int(expression), ast_type(expression));

    case TK_FLOAT:
    {
      ast_t* type = ast_type(expression);
      if(::is_literal(type, "F32"))
      {
        return CtfeValue(CtfeValueFloat<float>(ast_float(expression)), type);
      }
      else if(::is_literal(type, "F64"))
      {
        return CtfeValue(CtfeValueFloat<double>(ast_float(expression)), type);
      }
      else
      {
        pony_assert(false);
      }

      break;
    }
    case TK_STRING:
    {
      ast_t* string_type = ast_type(expression);
      ast_t* underlying_string_type = (ast_t*)ast_data(string_type);
      ast_t* underlying_members = ast_childidx(underlying_string_type, 4);
      ast_t* size_var = ast_child(underlying_members);
      ast_t* size_var_type = ast_childidx(size_var, 1);
      ast_t* alloc_var = ast_sibling(size_var);
      ast_t* alloc_var_type = ast_childidx(alloc_var, 1);
      ast_t* ptr_var = ast_sibling(alloc_var);
      ast_t* ptr_var_type = ast_childidx(ptr_var, 1);
      CtfeValueStruct* s = new CtfeValueStruct(string_type);

      size_t len = ast_name_len(expression);
      s->new_value("_size", CtfeValue(len, size_var_type));
      s->new_value("_alloc", CtfeValue(len + 1, alloc_var_type));
      s->new_value("_ptr", CtfeValue(CtfeValuePointer(
          reinterpret_cast<uint8_t*>(const_cast<char*>(ast_name(expression))),
          len + 1,
          ptr_var_type),
        ptr_var_type));

      CtfeValue ret = CtfeValue(s, string_type);
      add_allocated_reference(ret);

      return ret;
    }

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
      return evaluate(opt, errors, ast_child(expression));

    case TK_CALL:
      return evaluate_method(opt, errors, expression);

    case TK_SEQ:
    {
      CtfeValue evaluated;
      for(ast_t* p = ast_child(expression); p != nullptr; p = ast_sibling(p))
      {
        evaluated = evaluate(opt, errors, p);

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
      CtfeValue condition_evaluated = evaluate(opt, errors, condition);

      pony_assert(condition_evaluated.is_bool());

      m_frames.push_frame();
      CtfeValue ret = condition_evaluated.get_bool().get_value() ?
            evaluate(opt, errors, then_branch):
            evaluate(opt, errors, else_branch);
      m_frames.pop_frame();

      return ret;
    }

    case TK_RETURN:
    {
      AST_GET_CHILDREN(expression, return_expr);
      CtfeValue ret = evaluate(opt, errors, return_expr);
      ret.set_control_flow_modifier(CtfeValue::ControlFlowModifier::Return);
      return ret;
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
    case TK_MATCH_CAPTURE:
    {
      const char* var_name = ast_name(ast_child(expression));
      CtfeValue found_value;
      if (!m_frames.get_value(var_name, found_value))
      {
        ast_error_frame(errors, expression,
          "Symbol '%s' was not found because it was not previously created", var_name);
        throw CtfeFailToEvaluateException();
      }
      return found_value;
    }

    case TK_FVARREF:
    case TK_EMBEDREF:
    case TK_FLETREF:
    {
      AST_GET_CHILDREN(expression, receiver, id);
      CtfeValue evaluated_receiver = evaluate(opt, errors, receiver);

      if(evaluated_receiver.is_none())
      {
        ast_error_frame(errors, expression,
          "Cannot access variables in 'this' outside the comptime expression scope.");
        throw CtfeFailToEvaluateException();
      }

      const char* var_name = ast_name(id);

      CtfeValue found_value;
      if(evaluated_receiver.is_struct())
      {
        CtfeValueStruct* s = evaluated_receiver.get_struct_ref();
        if (!s->get_value(var_name, found_value))
        {
          ast_error_frame(errors, expression,
            "Symbol '%s' was not found in struct, this should not happen", var_name);
          throw CtfeFailToEvaluateException();
        }
      }
      else
      {
        ast_error_frame(errors, expression,
          "Unsupported field variable type");
        throw CtfeFailToEvaluateException();
      }

      return found_value;
    }

    case TK_ASSIGN:
    {
      AST_GET_CHILDREN(expression, left, right);

      CtfeValue left_value = evaluate(opt, errors, left);
      CtfeValue right_value = evaluate(opt, errors, right);

      left_side_assign(opt, errors, left, right_value);

      return left_value;
    }

    case TK_WHILE:
    {
      AST_GET_CHILDREN(expression, cond, thenbody, elsebody);

      CtfeValue evaluated_cond = evaluate(opt, errors, cond);

      pony_assert(evaluated_cond.is_bool());

      // the condition didn't hold on the first iteration so we evaluate the
      // else
      if(evaluated_cond.get_bool().get_value() == false)
      {
        return evaluate(opt, errors, elsebody);
      }

      // the condition held so evaluate the while returning the file iteration
      // result as the evaluated result
      CtfeValue evaluated_while;
      while(evaluated_cond.get_bool().get_value() == true)
      {
        m_frames.push_frame();
        evaluated_while = evaluate(opt, errors, thenbody);
        m_frames.pop_frame();

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

        evaluated_cond = evaluate(opt, errors, cond);
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
      CtfeValue ret = evaluate(opt, errors, break_expr);
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
    case TK_TRY_NO_CHECK:
    {
      AST_GET_CHILDREN(expression, trybody, elsebody);
      CtfeValue evaluated_try = evaluate(opt, errors, trybody);
      if(evaluated_try.get_control_flow_modifier() == CtfeValue::ControlFlowModifier::Error)
      {
        evaluated_try = evaluate(opt, errors, elsebody);
      }

      return evaluated_try;
    }

    case TK_REPEAT:
    {
      AST_GET_CHILDREN(expression, repeat_body, cond, elsebody);

      CtfeValue evaluated_cond = evaluate(opt, errors, cond);

      CtfeValue repeat_value;
      bool last_was_from_continue = false;

      do
      {
        last_was_from_continue = false;

        m_frames.push_frame();
        repeat_value = evaluate(opt, errors, repeat_body);
        m_frames.pop_frame();

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
        else if(modifier == CtfeValue::ControlFlowModifier::Continue)
        {
          repeat_value.clear_control_flow_modifier();
          last_was_from_continue = true;
        }

        evaluated_cond = evaluate(opt, errors, cond);

      }while(evaluated_cond.get_bool().get_value() == false);

      if(last_was_from_continue)
      {
        m_frames.push_frame();
        repeat_value = evaluate(opt, errors, elsebody);
        m_frames.pop_frame();
      }

      return repeat_value;
    }

    case TK_TUPLE:
    {
      ast_t* elem = ast_child(expression);

      // First go through and check how many members
      size_t tuple_size = 0;
      while(elem != nullptr)
      {
        tuple_size++;
        elem = ast_sibling(elem);
      }

      CtfeValueTuple tuple = CtfeValueTuple(tuple_size);

      elem = ast_child(expression);
      size_t i = 0;
      while(elem != nullptr)
      {
        CtfeValue evaluated_elem = evaluate(opt, errors, elem);

        if(!tuple.update_value(i, evaluated_elem))
        {
          ast_error_frame(errors, expression,
            "Creating tuple failed, tuple index out of range");
          throw CtfeFailToEvaluateException();
        }

        i++;
        elem = ast_sibling(elem);
      }

      return CtfeValue(tuple, ast_type(expression));
    }

    case TK_TUPLEELEMREF:
    {
      AST_GET_CHILDREN(expression, receiver, nr);

      CtfeValue evaluated_rec = evaluate(opt, errors, receiver);

      pony_assert(evaluated_rec.is_tuple());

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

    case TK_IFTYPE_SET:
    {
      AST_GET_CHILDREN(expression, iftype, else_clause);
      CtfeValue ret = evaluate(opt, errors, iftype);
      if(ret.is_empty())
      {
        if(!ret.get_bool().get_value())
        {
          m_frames.push_frame();
          ret = evaluate(opt, errors, else_clause);
          m_frames.pop_frame();
        }
        else
        {
          pony_assert(false);
        }
      }

      return ret;
    }

    case TK_IFTYPE:
    {
      AST_GET_CHILDREN(expression, left, right, then);

      CtfeValue ret;
      if(is_subtype_constraint(left, right, nullptr, opt))
      {
        m_frames.push_frame();
        ret = evaluate(opt, errors, then);
        m_frames.pop_frame();
      }
      else
      {
        // Empty CtfeValue indicates false
        ret = CtfeValue();
      }

      return ret;
    }

    case TK_FFICALL:
      return handle_ffi_call(opt, errors, expression);

    case TK_RECOVER:
    case TK_CONSUME:
    {
      CtfeValue ret = evaluate(opt, errors, ast_childidx(expression, 1));

      ast_t* new_type = ast_type(expression);

      // Special case for unions. Since the type of a union value is stored as the
      // selected type as the ast type, we must cherry pick the correct ast type in the
      // union type.
      if(CtfeAstType::is_union_type(new_type))
      {
        ast_t* u_type = ast_child(new_type);
        bool found = false;
        while(u_type != NULL)
        {
          if(is_subtype_ignore_cap(u_type, ret.get_type_ast(), nullptr, opt))
          {
            ret.set_type_ast(u_type);
            found = true;
            break;
          }

          u_type = ast_sibling(u_type);
        }

        if(!found)
        {
          pony_assert(false);
        }
      }
      else
      {
        ret.set_type_ast(new_type);
      }

      return ret;
    }

    case TK_MATCH:
    {
      AST_GET_CHILDREN(expression, matchref, cases, elsebody);
      CtfeValue match = evaluate(opt, errors, matchref);

      CtfeValue ret;

      bool matched = false;

      // Match cases by exhaustion
      ast_t* match_type = match.get_type_ast();
      ast_t* the_case = ast_child(cases);
      while(the_case != nullptr && !matched)
      {
        AST_GET_CHILDREN(the_case, pattern, guard, body);

        // First test that match is a subtype of the pattern
        ast_t* pattern_type = ast_type(pattern);

        bool is_subtype = false;
        if(match.is_tuple())
        {
          pony_assert(ast_id(pattern_type) == TK_TUPLETYPE);

          is_subtype = match.get_tuple().is_subtype(pattern_type, opt);
        }
        else
        {
          is_subtype = is_subtype_ignore_cap(match_type, pattern_type, nullptr, opt);
        }


        if(is_subtype)
        {
          m_frames.push_frame();

          if(match.is_tuple())
          {
            pony_assert(ast_id(pattern) == TK_TUPLE);

            ast_t* pattern_elem_seq = ast_child(pattern);

            bool element_match = true;
            CtfeValueTuple& match_tuple = match.get_tuple();
            for(size_t i = 0; i < match_tuple.size(); i++)
            {
              CtfeValue match_val;
              if(!match_tuple.get_value(i, match_val))
              {
                pony_assert(false);
              }

              ast_t* pattern_elem = ast_child(pattern_elem_seq);

              if(!match_eq_element(opt, errors, expression, match_val, pattern_elem, the_case))
              {
                element_match = false;
                break;
              }

              pattern_elem_seq = ast_sibling(pattern_elem_seq);
            }

            matched = element_match;
          }
          else
          {
            matched = match_eq_element(opt, errors, expression, match, pattern, the_case);
          }

          // if the pattern match proceed to check the guard
          if(matched)
          {
            if(ast_id(guard) != TK_NONE)
            {
              CtfeValue eval_guard = evaluate(opt, errors, guard);

              pony_assert(eval_guard.is_bool());

              if(eval_guard.get_bool().get_value())
              {
                ret = evaluate(opt, errors, body);
              }
              else
              {
                matched = false;
              }
            }
            else
            {
              ret = evaluate(opt, errors, body);
            }
          }

          m_frames.pop_frame();
        }

        the_case = ast_sibling(the_case);
      }

      if(!matched)
      {
        m_frames.push_frame();
        ret = evaluate(opt, errors, elsebody);
        m_frames.pop_frame();
      }

      return ret;
    }

    case TK_IS:
    {
      AST_GET_CHILDREN(expression, recv, that);
      CtfeValue recv_value = evaluate(opt, errors, recv);
      CtfeValue that_value = evaluate(opt, errors, that);

      bool is_result = is_operator(opt, errors, expression, recv_value, that_value);

      return CtfeValue(CtfeValueBool(is_result), ast_type(expression));
    }

    case TK_COMPTIME:
      // Comptime inside another comptime, no problem we just evaulate that as well
      return evaluate(opt, errors, ast_child(expression));

    default:
      ast_error_frame(errors, expression,
        "The CTFE runner does not support the '%s' expression",
        ast_get_print(expression));
      throw CtfeFailToEvaluateException();
  }

  throw CtfeFailToEvaluateException();
}


void CtfeRunner::left_side_assign(pass_opt_t* opt, errorframe_t* errors, ast_t* left,
  CtfeValue& right_val)
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
      CtfeValue evaluated_receiver = evaluate(opt, errors, receiver);

      const char* var_name = ast_name(id);

      if(evaluated_receiver.is_struct())
      {
        CtfeValueStruct* s = evaluated_receiver.get_struct_ref();
        if (!s->update_value(var_name, right_val))
        {
          ast_error_frame(errors, left,
            "Could not update symbol '%s' was not found in struct, this "
            "should not happen", var_name);
          throw CtfeFailToEvaluateException();
        }
      }
      else
      {
          ast_error_frame(errors, left,
            "Unsupported field variable type");
          throw CtfeFailToEvaluateException();
      }

      break;
    }
    case TK_TUPLE:
    {
      pony_assert(right_val.is_tuple());

      ast_t* seq_elem = ast_child(left);
      size_t i = 0;
      while(seq_elem != nullptr)
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

        left_side_assign(opt, errors, elem, right_tuple_value);

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
}


bool CtfeRunner::match_eq_element(pass_opt_t* opt, errorframe_t* errors, ast_t* ast_pos,
  CtfeValue &match, ast_t* pattern, ast_t* the_case)
{
  if(ast_id(pattern) == TK_DONTCAREREF)
  {
    return true;
  }
  else if(ast_id(pattern) == TK_MATCH_CAPTURE)
  {
    // Test whether the value is captured under a new name
    //map_value(opt, pattern, match, false);

    const char* var_name = ast_name(ast_child(pattern));

    if(!m_frames.new_value(var_name, match))
    {
      ast_error_frame(errors, ast_pos,
        "Parameter with name %s was already found in this case expression frame",
        var_name);
      throw CtfeFailToEvaluateException();
    }

    return true;
  }
  else
  {
    CtfeValue evaluated_pattern = evaluate(opt, errors, pattern);

    vector<CtfeValue> args = { evaluated_pattern };

    ast_t* res_type = get_builtin_type(opt, the_case, "Bool");

    CtfeValue equal = call_method(opt, errors, ast_pos, res_type, stringtab("eq"),
      match.get_type_ast(), match, args, nullptr);

    pony_assert(equal.is_bool());

    return equal.get_bool().get_value();
  }
}


bool CtfeRunner::is_operator(pass_opt_t* opt, errorframe_t* errors, ast_t* ast_pos,
  CtfeValue& left, const CtfeValue& right)
{
  ast_t* left_type = left.get_type_ast();
  ast_t* right_type = right.get_type_ast();

  bool is_sub = is_subtype(left_type, right_type, nullptr, opt);

  if(is_sub)
  {
    if(CtfeAstType::is_nominal(left_type))
    {
      if(CtfeAstType::is_primitive(left_type))
      {
        if(CtfeAstType::is_machine_word(left_type))
        {
          vector<CtfeValue> args = { right };

          ast_t* bool_type_ast = type_builtin(opt, ast_pos, "Bool");
          CtfeValue equal = call_method(opt, errors, ast_pos, bool_type_ast, stringtab("eq"),
            left_type, left, args, nullptr);

          return equal.get_bool().get_value();
        }

        // If we end up here that means it is a primitive and not machine word
        // and the type comparison is the only thing we need and we can fall through
        // to the last return.
      }
      else if(CtfeAstType::is_struct(left_type))
      {
        return left.get_struct_ref() == right.get_struct_ref();
      }
      else
      {
        pony_assert(false);
      }
    }
    else if(CtfeAstType::is_tuple(left_type))
    {
      CtfeValueTuple left_tuple = left.get_tuple();
      CtfeValueTuple right_tuple = right.get_tuple();

      if(left_tuple.size() != right_tuple.size())
      {
        return false;
      }

      for(size_t i = 0; i < left_tuple.size(); i++)
      {
        CtfeValue left_val;
        if(!left_tuple.get_value(i, left_val))
        {
          pony_assert(false);
        }

        CtfeValue right_val;
        if(!right_tuple.get_value(i, right_val))
        {
          pony_assert(false);
        }

        if(!is_operator(opt, errors, ast_pos, left_val, right_val))
        {
          return false;
        }
      }
    }
    else
    {
      pony_assert(false);
    }
  }

  return is_sub;
}


bool CtfeRunner::contains_valueparamref(ast_t* ast)
{
  while(ast != NULL)
  {
    if(ast_id(ast) == TK_VALUEFORMALPARAMREF ||
       contains_valueparamref(ast_child(ast)))
    {
      return true;
    }
    ast = ast_sibling(ast);
  }
  return false;
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

  if(contains_valueparamref(ast))
  {
    return true;
  }

  // Insert an empty this reference so that we can at least call functions inside the
  // current object. However it is not possible to access any variables.
  m_frames.new_value("this", CtfeValue());

  // We can't evaluate expressions which still have references to value
  // parameters so we simply stop, indicating no error yet.
  //if(contains_valueparamref(expression))
  //  return true;

  // evaluate the expression passing NULL as 'this' as we aren't
  // evaluating a method on an object
  errorframe_t errors = nullptr;
  bool failed = false;

  // Start a thread with a condition variable that just sits and waits until a timeout
  auto thread = std::thread([&]()
  {
    auto locked = unique_lock<mutex>(m_mutex);

    while(!m_stop_thread)
    {
      auto result = m_terminate.wait_until(locked,
        chrono::system_clock::now() + chrono::seconds(opt->ctfe_max_duration));

      if(result == cv_status::timeout) {
        m_max_duration_exceeded = true;
      }
    }
  });

  try
  {
    CtfeValue evaluated = evaluate(opt, &errors, ast);
    ast_t* new_literal = evaluated.create_ast_literal_node(opt, &errors, ast);
    if(new_literal != nullptr)
    {
      ast_replace(astp, new_literal);
    }
    else
    {
      failed = true;
    }
  }
  catch(const CtfeException& ex)
  {
    failed = true;
  }

  {
    auto locked = unique_lock<mutex>(m_mutex);
    m_stop_thread = true;
  }
  m_terminate.notify_one();
  if(thread.joinable())
  {
    thread.join();
  }

  if(failed)
  {
    if(ast_type(ast) != nullptr)
    {
      ast_settype(ast, ast_from(ast_type(ast), TK_ERRORTYPE));
    }
    else
    {
      ast_settype(ast, ast_blank(TK_ERRORTYPE));
    }
    errorframe_t frame = nullptr;
    ast_error_frame(&frame, ast, "could not evaluate compile time expression");
    errorframe_append(&frame, &errors);
    errorframe_report(&frame, opt->check.errors);
    opt->check.evaluation_error = true;
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

  return !failed;
}
