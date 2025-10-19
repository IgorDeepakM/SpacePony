#include "ctfe_runner.h"

#include "ponyassert.h"

#include <cstring>


using namespace std;

CtfeValue CtfeRunner::handle_ffi_call(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
    int depth)
{
  AST_GET_CHILDREN(ast, name, return_typeargs, args, namedargs, question);

  vector<CtfeValue> evaluated_args;

  ast_t* argument = ast_child(args);
  while(argument != NULL)
  {
    CtfeValue evaluated_arg = evaluate(opt, errors, argument, depth + 1);

    evaluated_args.push_back(evaluated_arg);
    argument = ast_sibling(argument);
  }

  string ffi_name = ast_name(name);
  if(ffi_name == "@memcmp" || ffi_name == "@memmove"|| ffi_name == "@memcpy")
  {
    return handle_ffi_ptr_ptr_size(opt, errors, ast, ffi_name, evaluated_args);
  }
  else if(string(ast_name(name)) == "@memset")
  {
    void* ptr = NULL;
    int ch = 0;
    size_t size = 0;

    CtfeValue arg1 = evaluated_args[0];

    if(arg1.is_pointer())
    {
      ptr = arg1.get_pointer().get_cpointer();
    }
    else
    {
      ast_error_frame(errors, ast,
        "Incompatible argument 1 in CTFE calling @memset");
      throw CtfeFailToEvaluateException();
    }

    CtfeValue arg2 = evaluated_args[1];

    if(arg2.is_typed_int())
    {
      ch = arg2.to_uint64();
    }
    else
    {
      ast_error_frame(errors, ast,
        "Incompatible argument 2 in CTFE calling @memset");
      throw CtfeFailToEvaluateException();
    }

    CtfeValue arg3 = evaluated_args[2];

    if(arg3.is_typed_int())
    {
      size = arg3.to_uint64();
    }
    else
    {
      ast_error_frame(errors, ast,
        "Incompatible argument 3 in CTFE calling @memset");
      throw CtfeFailToEvaluateException();
    }

    void* ret = memset(ptr, ch, size);
    ast_t* ret_type = ast_type(ast);
    return CtfeValue(CtfeValuePointer(ret, ret_type), ret_type);
  }

  ast_error_frame(errors, ast,
        "CTFE unsupported FFI call calling %s", ffi_name.c_str());
  throw CtfeFailToEvaluateException();
}


CtfeValue CtfeRunner::handle_ffi_ptr_ptr_size(pass_opt_t* opt, errorframe_t* errors,
  ast_t* ast, const string& ffi_name, const vector<CtfeValue>& evaluated_args)
{
  void* ptr1 = NULL;
  void* ptr2 = NULL;
  size_t size = 0;

  CtfeValue arg1 = evaluated_args[0];

  if(arg1.is_pointer())
  {
    ptr1 = arg1.get_pointer().get_cpointer();
  }
  else
  {
    ast_error_frame(errors, ast,
      "Incompatible argument 1 in CTFE calling %s", ffi_name.c_str());
    throw CtfeFailToEvaluateException();
  }

  CtfeValue arg2 = evaluated_args[1];

  if(arg2.is_pointer())
  {
    ptr2 = arg2.get_pointer().get_cpointer();
  }
  else
  {
    ast_error_frame(errors, ast,
      "Incompatible argument 2 in CTFE calling %s", ffi_name.c_str());
    throw CtfeFailToEvaluateException();
  }

  CtfeValue arg3 = evaluated_args[2];

  if(arg3.is_typed_int())
  {
    size = arg3.to_uint64();
  }
  else
  {
    ast_error_frame(errors, ast,
      "Incompatible argument 3 in CTFE calling %s", ffi_name.c_str());
    throw CtfeFailToEvaluateException();
  }

  if(ffi_name == "@memcmp")
  {
    int ret = memcmp(ptr1, ptr2, size);
    const string ret_type_name = ast_name(ast_childidx(ast_type(ast), 1));
    ast_t* ret_type = ast_type(ast);
    return CtfeValue(CtfeValueIntLiteral(ret), ret_type);
  }
  else if(ffi_name == "@memmove")
  {
    void* ret = memmove(ptr1, ptr2, size);
    ast_t* ret_type = ast_type(ast);
    return CtfeValue(CtfeValuePointer(ret, ret_type), ret_type);
  }
  else if(ffi_name == "@memcpy")
  {
    void* ret = memcpy(ptr1, ptr2, size);
    ast_t* ret_type = ast_type(ast);
    return CtfeValue(CtfeValuePointer(ret, ret_type), ret_type);
  }


  pony_assert(false);

  return CtfeValue();
}
