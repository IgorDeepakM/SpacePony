#include "ctfe_comp_time_primitive.h"
#include "ctfe_value.h"
#include "ctfe_value_struct.h"
#include "ctfe_exception.h"
#include "ctfe_runner.h"

#include "ponyassert.h"
#include "../type/assemble.h"

#include <iostream>
#include <fstream>


using namespace std;

filesystem::path CtfeCompTimePrimitive::get_prepend_path(pass_opt_t* opt, const CtfeValue& arg)
{
  filesystem::path path = "";
  string loc_name = CtfeAstType::get_type_name(arg.get_type_ast());
  if(loc_name == "CompTimeWokringDirectory")
  {
    path = ".";
  }
  else if(loc_name == "CompTimeOutputDirectory")
  {
    path = opt->output;
  }
  else if(loc_name == "CompTimeTempDirectory")
  {
    path = filesystem::temp_directory_path();
  }

  return path;
}


bool CtfeCompTimePrimitive::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, CtfeRunner& ctfeRunner)
{
  if(args.size() == 2)
  { 
    if(method_name == "load_file")
    {
      filesystem::path path = get_prepend_path(opt, args[0]);

      CtfeValueStruct* str = args[1].get_struct_ref();
      CtfeValue str_size;
      if(!str->get_value("_size", str_size))
      {
        pony_assert(false);
      }

      CtfeValue str_ptr;
      if(!str->get_value("_ptr", str_ptr))
      {
        pony_assert(false);
      }

      string spath(reinterpret_cast<char*>(str_ptr.get_pointer().get_cpointer()), str_size.to_uint64());
      filesystem::path file_name = spath;
      path /= file_name;

      std::ifstream f(path, std::ios::in | std::ios::binary);

      size_t sz = 0;
      try
      {
        sz = std::filesystem::file_size(path);
      }
      catch(const filesystem::filesystem_error& e)
      {
        ast_error_frame(errors, ast,
          "Error during comptime loading file, '%s'", e.what());
        throw CtfeFailToEvaluateException();
      }

      ast_t* underlying_res_type = (ast_t*)ast_data(res_type);
      ast_t* members = ast_childidx(underlying_res_type, 4);
      ast_t* ptr_member = ast_childidx(members, 2);
      ast_t* ptr_member_type = ast_childidx(ptr_member, 1);

      ast_t* ptr_type = ast_dup(ptr_member_type);
      ast_t* ptr_first_typearg = ast_child(ast_childidx(ptr_type, 2));
      ast_replace(&ptr_first_typearg, type_builtin(opt, ast, "U8"));

      ptr_type = ctfeRunner.cache_and_get_built_type(ptr_type);

      CtfeValuePointer ar_ptr(sz, ptr_type, ctfeRunner);

      f.read(reinterpret_cast<char*>(ar_ptr.get_cpointer()), sz);
      f.close();

      ast_t* size_var = ast_child(members);
      ast_t* size_var_type = ast_childidx(size_var, 1);
      ast_t* alloc_var = ast_sibling(size_var);
      ast_t* alloc_var_type = ast_childidx(alloc_var, 1);

      CtfeValueStruct* new_array = new CtfeValueStruct(res_type);
      new_array->new_value("_size", CtfeValue(sz, size_var_type));
      new_array->new_value("_alloc", CtfeValue(sz, alloc_var_type));
      new_array->new_value("_ptr", CtfeValue(ar_ptr, ptr_type));
      result = CtfeValue(new_array, res_type);
      return true;
    }
  }
  else if(args.size() == 3)
  {
    if(method_name == "save_file")
    {
      filesystem::path path = get_prepend_path(opt, args[1]);

      CtfeValueStruct* arr = args[0].get_struct_ref();
      CtfeValue arr_size;
      if(!arr->get_value("_size", arr_size))
      {
        pony_assert(false);
      }

      CtfeValue arr_ptr;
      if(!arr->get_value("_ptr", arr_ptr))
      {
        pony_assert(false);
      }

      CtfeValueStruct* str = args[2].get_struct_ref();
      CtfeValue str_size;
      if(!str->get_value("_size", str_size))
      {
        pony_assert(false);
      }

      CtfeValue str_ptr;
      if(!str->get_value("_ptr", str_ptr))
      {
        pony_assert(false);
      }

      string spath(reinterpret_cast<char*>(str_ptr.get_pointer().get_cpointer()), str_size.to_uint64());
      filesystem::path file_name = spath;
      path /= file_name;

      std::ofstream f(path, std::ios::out | std::ios::binary);

      size_t write_len = arr_size.to_uint64();
      
      try
      {
        f.write(reinterpret_cast<const char*>(arr_ptr.get_pointer().get_cpointer()), write_len);
        f.close();
      }
      catch(const filesystem::filesystem_error& e)
      {
        ast_error_frame(errors, ast,
          "Error during comptime saving file, '%s'", e.what());
        throw CtfeFailToEvaluateException();
      }

      return true;
    }
  }

  return false;
}