#include "ctfe_runner.h"

#include "ponyassert.h"
#include "../ast/lexint.h"

#include <cstring>
#include <bit>


using namespace std;

CtfeValue CtfeRunner::handle_ffi_call(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
    int depth)
{
  AST_GET_CHILDREN(ast, name, return_typeargs, args);

  vector<CtfeValue> evaluated_args;
  ast_t* ffi_decl = (ast_t*)ast_data(ast);
  ast_t* ffi_decl_return_typargs = ast_childidx(ffi_decl, 1);
  ast_t* return_type = ast_child(ffi_decl_return_typargs);

  ast_t* argument = ast_child(args);
  while(argument != NULL)
  {
    CtfeValue evaluated_arg = evaluate(opt, errors, argument, depth + 1);

    evaluated_args.push_back(evaluated_arg);
    argument = ast_sibling(argument);
  }

  string ffi_name = ast_name(name);

  if(ffi_name.rfind("@llvm.", 0) == 0)
  {
    return handle_llvm_ffi(opt, errors, ast, return_type, ffi_name, evaluated_args);
  }
  if(ffi_name == "@memcmp" || ffi_name == "@memmove"|| ffi_name == "@memcpy")
  {
    return handle_ffi_ptr_ptr_size(opt, errors, ast, return_type, ffi_name, evaluated_args);
  }
  else if(string(ast_name(name)) == "@memset")
  {
    void* ptr = nullptr;
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
    return CtfeValue(CtfeValuePointer(ret, return_type), return_type);
  }

  ast_error_frame(errors, ast,
        "CTFE unsupported FFI call calling %s", ffi_name.c_str());
  throw CtfeFailToEvaluateException();
}


CtfeValue CtfeRunner::handle_ffi_ptr_ptr_size(pass_opt_t* opt, errorframe_t* errors,
  ast_t* ast, ast_t* return_type, const string& ffi_name, const vector<CtfeValue>& evaluated_args)
{
  void* ptr1 = nullptr;
  void* ptr2 = nullptr;
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
    return CtfeValue(CtfeValueIntLiteral(ret), return_type);
  }
  else if(ffi_name == "@memmove")
  {
    void* ret = memmove(ptr1, ptr2, size);
    return CtfeValue(CtfeValuePointer(ret, return_type), return_type);
  }
  else if(ffi_name == "@memcpy")
  {
    void* ret = memcpy(ptr1, ptr2, size);
    return CtfeValue(CtfeValuePointer(ret, return_type), return_type);
  }

  pony_assert(false);

  return CtfeValue();
}


CtfeValue CtfeRunner::handle_llvm_ffi(pass_opt_t* opt, errorframe_t* errors,
  ast_t* ast, ast_t* return_type, const string& ffi_name, const vector<CtfeValue>& evaluated_args)
{

  if(ffi_name.rfind("@llvm.ctlz.", 0) == 0)
  {
    uint64_t src = 0;
    //bool is_zero_undef = false; not needed

    CtfeValue arg1 = evaluated_args[0];

    if(arg1.is_typed_int())
    {
      src = arg1.to_uint64();
    }
    else
    {
      ast_error_frame(errors, ast,
        "Incompatible argument 1 in CTFE calling %s", ffi_name.c_str());
      throw CtfeFailToEvaluateException();
    }

    if(ffi_name == "@llvm.ctlz.i64")
    {
      int ret = count_leading_zeros(src);
      return CtfeValue(CtfeValueIntLiteral(ret), return_type);
    }
    else if(ffi_name == "@llvm.ctlz.i32")
    {
      int ret = count_leading_zeros(src << 32);
      return CtfeValue(CtfeValueIntLiteral(ret > 32 ? 32 : ret), return_type);
    }
    else if(ffi_name == "@llvm.ctlz.i16")
    {
      int ret = count_leading_zeros(src << (32 + 16));
      return CtfeValue(CtfeValueIntLiteral(ret > 16 ? 16 : ret), return_type);
    }
    else if(ffi_name == "@llvm.ctlz.i8")
    {
      int ret = count_leading_zeros(src << (32 + 16 + 8));
      return CtfeValue(CtfeValueIntLiteral(ret > 8 ? 8 : ret), return_type);
    }
  }

  ast_error_frame(errors, ast,
        "CTFE unsupported LLVM FFI call calling %s", ffi_name.c_str());
  throw CtfeFailToEvaluateException();
}
