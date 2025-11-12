#include "ctfe_value.h"
#include "ctfe_value_struct.h"
#include "ctfe_value_typed_int_run_method.h"
#include "ctfe_value_typed_float_run_method.h"
#include "ctfe_exception.h"

#include "ponyassert.h"
#include "../pass/pass.h"
#include "../ast/lexer.h"

#include <algorithm>


using namespace std;


CtfeValue::CtfeValue():
  m_type{nullptr},
  m_ctrlFlow{ControlFlowModifier::None}
{

}


CtfeValue::CtfeValue(ast_t* type):
  m_type{type},
  m_ctrlFlow{ControlFlowModifier::None}
{

}


CtfeValue::CtfeValue(const CtfeValueIntLiteral& val, ast_t* type):
  m_type{type},
  m_ctrlFlow{ControlFlowModifier::None}
{
  if(ast_id(type) != TK_LITERAL)
  {
    convert_from_int_literal_to_type(val, type);
  }
  else
  {
    pony_assert(false);
  }
}


CtfeValue::CtfeValue(CtfeValueStruct* ref, ast_t* type):
  m_type{type},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{ref}
{

}


CtfeValue::CtfeValue(const CtfeValueBool& val, ast_t* type):
  m_type{type},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{val}
{

}


CtfeValue::CtfeValue(const CtfeValueTuple& val, ast_t* type):
  m_type{type},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{val}
{

}


CtfeValue::CtfeValue(const CtfeValuePointer& p, ast_t* type):
  m_type{type},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{p}
{

}


void CtfeValue::set_type_ast(ast_t* new_type)
{
  m_type = new_type;
}


void CtfeValue::convert_from_int_literal_to_type(const CtfeValueIntLiteral& val,
  ast_t* type)
{
  const string pony_type = CtfeAstType::get_type_name(type);

  if(pony_type == "I8")
  {
    m_val = CtfeValueTypedInt<int8_t>(val);
  }
  else if(pony_type == "U8")
  {
    m_val = CtfeValueTypedInt<uint8_t>(val);
  }
  else if(pony_type == "I16")
  {
    m_val = CtfeValueTypedInt<int16_t>(val);
  }
  else if(pony_type == "U16")
  {
    m_val = CtfeValueTypedInt<uint16_t>(val);
  }
  else if(pony_type == "I32")
  {
    m_val = CtfeValueTypedInt<int32_t>(val);
  }
  else if(pony_type == "U32")
  {
    m_val = CtfeValueTypedInt<uint32_t>(val);
  }
  else if(pony_type == "I64")
  {
    m_val = CtfeValueTypedInt<int64_t>(val);
  }
  else if(pony_type == "U64")
  {
    m_val = CtfeValueTypedInt<uint64_t>(val);
  }
   else if(pony_type == "I128")
  {
    m_val = CtfeValueTypedInt<CtfeI128Type>(val);
  }
  else if(pony_type == "U128")
  {
    m_val = CtfeValueTypedInt<CtfeU128Type>(val);
  }
  else if(pony_type == "ILong")
  {
    if(get_long_size() == 4)
    {
      m_val = CtfeValueTypedInt<int32_t>(val);
    }
    else if(get_long_size() == 8)
    {
      m_val = CtfeValueTypedInt<int64_t>(val);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(pony_type == "ULong")
  {
    if(get_long_size() == 4)
    {
      m_val = CtfeValueTypedInt<uint32_t>(val);
    }
    else if(get_long_size() == 8)
    {
      m_val = CtfeValueTypedInt<uint64_t>(val);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(pony_type == "ISize")
  {
    if(get_size_size() == 4)
    {
      m_val = CtfeValueTypedInt<int32_t>(val);
    }
    else if(get_size_size() == 8)
    {
      m_val = CtfeValueTypedInt<int64_t>(val);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(pony_type == "USize")
  {
     if(get_size_size() == 4)
    {
      m_val = CtfeValueTypedInt<uint32_t>(val);
    }
    else if(get_size_size() == 8)
    {
      m_val = CtfeValueTypedInt<uint64_t>(val);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(pony_type == "F32")
  {
    m_val = CtfeValueTypedFloat<float>(val);
  }
  else if(pony_type == "F64")
  {
    m_val = CtfeValueTypedFloat<double>(val);
  }
}


ast_t* CtfeValue::create_ast_literal_node(pass_opt_t* opt, errorframe_t* errors,
  ast_t* from)
{
  ast_t* new_node = NULL;

  if(CtfeAstType::is_tuple(m_type))
  {
    new_node = get_tuple().create_ast_literal_node(opt, errors, from);
  }
  else if(CtfeAstType::is_nominal(m_type))
  {
    const string pony_type = CtfeAstType::get_type_name(m_type);

    if(pony_type == "Bool")
    {
      new_node = get<CtfeValueBool>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "I8")
    {
      new_node = get<CtfeValueTypedInt<int8_t>>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "U8")
    {
      new_node =  get<CtfeValueTypedInt<uint8_t>>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "I16")
    {
      new_node =  get<CtfeValueTypedInt<int16_t>>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "U16")
    {
      new_node =  get<CtfeValueTypedInt<uint16_t>>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "I32")
    {
      new_node =  get<CtfeValueTypedInt<int32_t>>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "U32")
    {
      new_node =  get<CtfeValueTypedInt<uint32_t>>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "I64")
    {
      new_node =  get<CtfeValueTypedInt<int64_t>>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "U64")
    {
      new_node =  get<CtfeValueTypedInt<uint64_t>>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "I128")
    {
      new_node =  get<CtfeValueTypedInt<CtfeI128Type>>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "U128")
    {
      new_node = get<CtfeValueTypedInt<CtfeU128Type>>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "ILong")
    {
      if(get_long_size() == 4)
      {
        new_node = get<CtfeValueTypedInt<int32_t>>(m_val).create_ast_literal_node();
      }
      else if(get_long_size() == 8)
      {
        new_node = get<CtfeValueTypedInt<int64_t>>(m_val).create_ast_literal_node();
      }
      else
      {
        pony_assert(false);
      }
    }
    else if(pony_type == "ULong")
    {
      if(get_long_size() == 4)
      {
        new_node = get<CtfeValueTypedInt<uint32_t>>(m_val).create_ast_literal_node();
      }
      else if(get_long_size() == 8)
      {
        new_node = get<CtfeValueTypedInt<uint64_t>>(m_val).create_ast_literal_node();
      }
      else
      {
        pony_assert(false);
      }
    }
    else if(pony_type == "ISize")
    {
      if(get_size_size() == 4)
      {
        new_node = get<CtfeValueTypedInt<int32_t>>(m_val).create_ast_literal_node();
      }
      else if(get_size_size() == 8)
      {
        new_node = get<CtfeValueTypedInt<int64_t>>(m_val).create_ast_literal_node();
      }
      else
      {
        pony_assert(false);
      }
    }
    else if(pony_type == "USize")
    {
      if(get_size_size() == 4)
      {
        new_node = get<CtfeValueTypedInt<uint32_t>>(m_val).create_ast_literal_node();
      }
      else if(get_size_size() == 8)
      {
        new_node = get<CtfeValueTypedInt<uint64_t>>(m_val).create_ast_literal_node();
      }
      else
      {
        pony_assert(false);
      }
    }
    else if(pony_type == "F32")
    {
      new_node =  get<CtfeValueTypedFloat<float>>(m_val).create_ast_literal_node();
    }
    else if(pony_type == "F64")
    {
      new_node = get<CtfeValueTypedFloat<double>>(m_val).create_ast_literal_node();
    }
    else if(CtfeAstType::is_struct(m_type))
    {
      if(CtfeAstType::get_type_name(m_type) == "String")
      {
        CtfeValueStruct* s = get_struct_ref();
        new_node = ast_blank(TK_STRING);
        CtfeValue string;
        s->get_value("_ptr", string);
        ast_set_name(new_node, reinterpret_cast<const char*>(string.get_pointer().get_cpointer()));
      }
      else
      {
        CtfeValueStruct* s = get_struct_ref();
        new_node = s->create_ast_literal_node(opt, errors, from);
        // We don't take the type from CtfeValue but rather CtfeValueStruct type
        return new_node;
      }
    }
    else
    {
      ast_error_frame(errors, from,
        "CTFE: creating constant object from unknown nominal type");
    }
  }
  else
  {
    ast_error_frame(errors, from,
      "CTFE: cannot create a constant object of type '%s'", lexer_print(ast_id(m_type)));
  }

  if(new_node != nullptr)
  {
    ast_settype(new_node, ast_dup(m_type));
  }

  return new_node;
}


bool CtfeValue::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, CtfeRunner &ctfeRunner)
{
  // A null ast type could indicate that we are trying operate on the
  // host this that doesn't exist.
  if(recv.is_empty())
  {
    return false;
  }

  if(recv.is_machine_word())
  {
    const string type_name = recv.get_type_name();

    if(type_name == "Bool")
    {
      return CtfeValueBool::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "I8")
    {
      return CtfeValueTypedInt<int8_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "U8")
    {
      return CtfeValueTypedInt<uint8_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "I16")
    {
      return CtfeValueTypedInt<int16_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "U16")
    {
      return CtfeValueTypedInt<uint16_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "I32")
    {
      return CtfeValueTypedInt<int32_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "U32")
    {
      return CtfeValueTypedInt<uint32_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "I64")
    {
      return CtfeValueTypedInt<int64_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "U64")
    {
      return CtfeValueTypedInt<uint64_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "I128")
    {
      return CtfeValueTypedInt<CtfeI128Type>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "U128")
    {
      return CtfeValueTypedInt<CtfeU128Type>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "ILong")
    {
        if(get_long_size() == 4)
        {
          return CtfeValueTypedInt<int32_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
        }
        else if(get_long_size() == 8)
        {
          return CtfeValueTypedInt<int64_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
        }
        else
        {
          pony_assert(false);
        }
    }
    else if(type_name == "ULong")
    {
      if(get_long_size() == 4)
      {
        return CtfeValueTypedInt<uint32_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
      }
      else if(get_long_size() == 8)
      {
        return CtfeValueTypedInt<uint64_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
      }
      else
      {
        pony_assert(false);
      }
    }
    else if(type_name == "ISize")
    {
      if(get_size_size() == 4)
      {
        return CtfeValueTypedInt<int32_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
      }
      else if(get_size_size() == 8)
      {
        return CtfeValueTypedInt<int64_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
      }
      else
      {
        pony_assert(false);
      }
    }
    else if(type_name == "USize")
    {
      if(get_size_size() == 4)
      {
        return CtfeValueTypedInt<uint32_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
      }
      else if(get_size_size() == 8)
      {
        return CtfeValueTypedInt<uint64_t>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
      }
      else
      {
        pony_assert(false);
      }
    }
    else if(type_name == "F32")
    {
      return CtfeValueTypedFloat<float>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
    else if(type_name == "F64")
    {
      return CtfeValueTypedFloat<double>::run_method(opt, errors, ast, res_type, recv, args, method_name, result);
    }
  }
  else if(CtfeAstType::is_pointer(recv.get_type_ast()))
  {
    return CtfeValuePointer::run_method(opt, errors, ast, res_type, recv, args, method_name, result, ctfeRunner);
  }

  return false;
}


uint64_t CtfeValue::to_uint64() const
{
  const string type_name = CtfeAstType::get_type_name(m_type);

  if(type_name == "I8")
  {
    return get<CtfeValueTypedInt<int8_t>>(m_val).to_uint64();
  }
  else if(type_name == "U8")
  {
    return get<CtfeValueTypedInt<uint8_t>>(m_val).to_uint64();
  }
  else if(type_name == "I16")
  {
    return get<CtfeValueTypedInt<int16_t>>(m_val).to_uint64();
  }
  else if(type_name == "U16")
  {
    return get<CtfeValueTypedInt<uint16_t>>(m_val).to_uint64();
  }
  else if(type_name == "I32")
  {
    return get<CtfeValueTypedInt<int32_t>>(m_val).to_uint64();
  }
  else if(type_name == "U32")
  {
    return get<CtfeValueTypedInt<uint32_t>>(m_val).to_uint64();
  }
  else if(type_name == "I64")
  {
    return get<CtfeValueTypedInt<int64_t>>(m_val).to_uint64();
  }
  else if(type_name == "U64")
  {
    return get<CtfeValueTypedInt<uint64_t>>(m_val).to_uint64();
  }
  else if(type_name == "I128")
  {
    return get<CtfeValueTypedInt<CtfeI128Type>>(m_val).to_uint64();
  }
  else if(type_name == "U128")
  {
    return get<CtfeValueTypedInt<CtfeU128Type>>(m_val).to_uint64();
  }
  else if(type_name == "ILong")
  {
    if(get_long_size() == 4)
    {
      return get<CtfeValueTypedInt<int32_t>>(m_val).to_uint64();
    }
    else if(get_long_size() == 8)
    {
      return get<CtfeValueTypedInt<int64_t>>(m_val).to_uint64();
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(type_name == "ULong")
  {
    if(get_long_size() == 4)
    {
      return get<CtfeValueTypedInt<uint32_t>>(m_val).to_uint64();
    }
    else if(get_long_size() == 8)
    {
      return get<CtfeValueTypedInt<uint64_t>>(m_val).to_uint64();
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(type_name == "ISize")
  {
    if(get_size_size() == 4)
    {
      return get<CtfeValueTypedInt<int32_t>>(m_val).to_uint64();
    }
    else if(get_size_size() == 8)
    {
      return get<CtfeValueTypedInt<int64_t>>(m_val).to_uint64();
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(type_name == "USize")
  {
    if(get_size_size() == 4)
    {
      return get<CtfeValueTypedInt<uint32_t>>(m_val).to_uint64();
    }
    else if(get_size_size() == 8)
    {
      return get<CtfeValueTypedInt<uint64_t>>(m_val).to_uint64();
    }
    else
    {
      pony_assert(false);
    }
  }

  pony_assert(false);

  return 0;
}


void CtfeValue::write_to_memory(uint8_t* ptr) const
{
  const string type_name = CtfeAstType::get_type_name(m_type);

  if(type_name == "Bool")
  {
    get<CtfeValueBool>(m_val).write_to_memory(ptr);
  }
  else if(type_name == "I8")
  {
    get<CtfeValueTypedInt<int8_t>>(m_val).write_to_memory(ptr);
  }
  else if(type_name == "U8")
  {
    get<CtfeValueTypedInt<uint8_t>>(m_val).write_to_memory(ptr);
  }
  else if(type_name == "I16")
  {
    get<CtfeValueTypedInt<int16_t>>(m_val).write_to_memory(ptr);
  }
  else if(type_name == "U16")
  {
    get<CtfeValueTypedInt<uint16_t>>(m_val).write_to_memory(ptr);
  }
  else if(type_name == "I32")
  {
    get<CtfeValueTypedInt<int32_t>>(m_val).write_to_memory(ptr);
  }
  else if(type_name == "U32")
  {
    get<CtfeValueTypedInt<uint32_t>>(m_val).write_to_memory(ptr);
  }
  else if(type_name == "I64")
  {
    get<CtfeValueTypedInt<int64_t>>(m_val).write_to_memory(ptr);
  }
  else if(type_name == "U64")
  {
    get<CtfeValueTypedInt<uint64_t>>(m_val).write_to_memory(ptr);
  }
  else if(type_name == "I128")
  {
    get<CtfeValueTypedInt<CtfeI128Type>>(m_val).write_to_memory(ptr);
  }
  else if(type_name == "U128")
  {
    get<CtfeValueTypedInt<CtfeU128Type>>(m_val).write_to_memory(ptr);
  }
  else if(type_name == "ILong")
  {
    if(get_long_size() == 4)
    {
      get<CtfeValueTypedInt<int32_t>>(m_val).write_to_memory(ptr);
    }
    else if(get_long_size() == 8)
    {
      get<CtfeValueTypedInt<int64_t>>(m_val).write_to_memory(ptr);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(type_name == "ULong")
  {
    if(get_long_size() == 4)
    {
      get<CtfeValueTypedInt<uint32_t>>(m_val).write_to_memory(ptr);
    }
    else if(get_long_size() == 8)
    {
      get<CtfeValueTypedInt<uint64_t>>(m_val).write_to_memory(ptr);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(type_name == "ISize")
  {
    if(get_size_size() == 4)
    {
      get<CtfeValueTypedInt<int32_t>>(m_val).write_to_memory(ptr);
    }
    else if(get_size_size() == 8)
    {
      get<CtfeValueTypedInt<int64_t>>(m_val).write_to_memory(ptr);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(type_name == "USize")
  {
    if(get_size_size() == 4)
    {
      get<CtfeValueTypedInt<uint32_t>>(m_val).write_to_memory(ptr);
    }
    else if(get_size_size() == 8)
    {
      get<CtfeValueTypedInt<uint64_t>>(m_val).write_to_memory(ptr);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if (type_name == "F32")
  {
    get<CtfeValueTypedFloat<float>>(m_val).write_to_memory(ptr);
  }
  else if (type_name == "F64")
  {
    get<CtfeValueTypedFloat<double>>(m_val).write_to_memory(ptr);
  }
  else if(CtfeAstType::is_struct(m_type) || CtfeAstType::is_interface(m_type))
  {
    get_struct_ref()->write_to_memory(ptr);
  }
  else
  {
    pony_assert(false);
  }
}


CtfeValue CtfeValue::read_from_memory(ast_t* type, uint8_t* ptr)
{
  const string type_name = CtfeAstType::get_type_name(type);

  if(type_name == "Bool")
  {
    return CtfeValue(CtfeValueBool::read_from_memory(ptr), type);
  }
  else if(type_name == "I8")
  {
    return CtfeValue(CtfeValueTypedInt<int8_t>::read_from_memory(ptr), type);
  }
  else if(type_name == "U8")
  {
    return CtfeValue(CtfeValueTypedInt<uint8_t>::read_from_memory(ptr), type);
  }
  else if(type_name == "I16")
  {
    return CtfeValue(CtfeValueTypedInt<int16_t>::read_from_memory(ptr), type);
  }
  else if(type_name == "U16")
  {
    return CtfeValue(CtfeValueTypedInt<uint16_t>::read_from_memory(ptr), type);
  }
  else if(type_name == "I32")
  {
    return CtfeValue(CtfeValueTypedInt<int32_t>::read_from_memory(ptr), type);
  }
  else if(type_name == "U32")
  {
    return CtfeValue(CtfeValueTypedInt<uint32_t>::read_from_memory(ptr), type);
  }
  else if(type_name == "I64")
  {
    return CtfeValue(CtfeValueTypedInt<int64_t>::read_from_memory(ptr), type);
  }
  else if(type_name == "U64")
  {
    return CtfeValue(CtfeValueTypedInt<uint64_t>::read_from_memory(ptr), type);
  }
  else if(type_name == "I128")
  {
    return CtfeValue(CtfeValueTypedInt<CtfeI128Type>::read_from_memory(ptr), type);
  }
  else if(type_name == "U128")
  {
    return CtfeValue(CtfeValueTypedInt<CtfeU128Type>::read_from_memory(ptr), type);
  }
  else if(type_name == "ILong")
  {
    if(get_long_size() == 4)
    {
      return CtfeValue(CtfeValueTypedInt<int32_t>::read_from_memory(ptr), type);
    }
    else if(get_long_size() == 8)
    {
      return CtfeValue(CtfeValueTypedInt<int64_t>::read_from_memory(ptr), type);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(type_name == "ULong")
  {
    if(get_long_size() == 4)
    {
      return CtfeValue(CtfeValueTypedInt<uint32_t>::read_from_memory(ptr), type);
    }
    else if(get_long_size() == 8)
    {
      return CtfeValue(CtfeValueTypedInt<uint64_t>::read_from_memory(ptr), type);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(type_name == "ISize")
  {
    if(get_size_size() == 4)
    {
      return CtfeValue(CtfeValueTypedInt<int32_t>::read_from_memory(ptr), type);
    }
    else if(get_size_size() == 8)
    {
      return CtfeValue(CtfeValueTypedInt<int64_t>::read_from_memory(ptr), type);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(type_name == "USize")
  {
    if(get_size_size() == 4)
    {
      return CtfeValue(CtfeValueTypedInt<uint32_t>::read_from_memory(ptr), type);
    }
    else if(get_size_size() == 8)
    {
      return CtfeValue(CtfeValueTypedInt<uint64_t>::read_from_memory(ptr), type);
    }
    else
    {
      pony_assert(false);
    }
  }
  else if (type_name == "F32")
  {
    return CtfeValue(CtfeValueTypedFloat<float>::read_from_memory(ptr), type);
  }
  else if (type_name == "F64")
  {
    return CtfeValue(CtfeValueTypedFloat<double>::read_from_memory(ptr), type);
  }
  else if(CtfeAstType::is_struct(type) || CtfeAstType::is_interface(type))
  {
    CtfeValueStruct* s = CtfeValueStruct::read_from_memory(ptr);
    if(s != nullptr)
    {
      // When restored from memory, the type information is lost as it is only a pointer.
      // This can be restored from the CtfeValueStruct itself.
      return CtfeValue(s, s->get_type_ast());
    }
    else
    {
      return CtfeValue();
    }
  }
  else
  {
    pony_assert(false);
  }

  return CtfeValue();
}


bool CtfeValue::operator==(const CtfeValue& b) const
{
  return eq(b).get_value();
}


bool CtfeValue::operator!=(const CtfeValue& b) const
{
  return !eq(b).get_value();
}


CtfeValueBool CtfeValue::eq(const CtfeValue& b) const
{
  if(CtfeAstType::ast_hash(m_type) != CtfeAstType::ast_hash(b.m_type))
  {
    return CtfeValueBool(false);
  }

  CtfeValueBool ret;

  if(CtfeAstType::is_nominal(m_type))
  {
    const string pony_type = CtfeAstType::get_type_name(m_type);

    if(pony_type == "Bool")
    {
      return CtfeValueBool(
        get<CtfeValueBool>(m_val).get_value() == get<CtfeValueBool>(b.m_val).get_value());
    }
    else if(pony_type == "I8")
    {
      return get<CtfeValueTypedInt<int8_t>>(m_val).eq(get<CtfeValueTypedInt<int8_t>>(b.m_val));
    }
    else if(pony_type == "U8")
    {
      return get<CtfeValueTypedInt<uint8_t>>(m_val).eq(get<CtfeValueTypedInt<uint8_t>>(b.m_val));
    }
    else if(pony_type == "I16")
    {
      return get<CtfeValueTypedInt<int16_t>>(m_val).eq(get<CtfeValueTypedInt<int16_t>>(b.m_val));
    }
    else if(pony_type == "U16")
    {
      return get<CtfeValueTypedInt<uint16_t>>(m_val).eq(get<CtfeValueTypedInt<uint16_t>>(b.m_val));
    }
    else if(pony_type == "I32")
    {
      return get<CtfeValueTypedInt<int32_t>>(m_val).eq(get<CtfeValueTypedInt<int32_t>>(b.m_val));
    }
    else if(pony_type == "U32")
    {
      return get<CtfeValueTypedInt<uint32_t>>(m_val).eq(get<CtfeValueTypedInt<uint32_t>>(b.m_val));
    }
    else if(pony_type == "I64")
    {
      return get<CtfeValueTypedInt<int64_t>>(m_val).eq(get<CtfeValueTypedInt<int64_t>>(b.m_val));
    }
    else if(pony_type == "U64")
    {
      return get<CtfeValueTypedInt<uint64_t>>(m_val).eq(get<CtfeValueTypedInt<uint64_t>>(b.m_val));
    }
    else if(pony_type == "I128")
    {
      return get<CtfeValueTypedInt<CtfeI128Type>>(m_val).eq(get<CtfeValueTypedInt<CtfeI128Type>>(b.m_val));
    }
    else if(pony_type == "U128")
    {
      return get<CtfeValueTypedInt<CtfeU128Type>>(m_val).eq(get<CtfeValueTypedInt<CtfeU128Type>>(b.m_val));
    }
    else if(pony_type == "ILong")
    {
      if(get_long_size() == 4)
      {
        return get<CtfeValueTypedInt<int32_t>>(m_val).eq(get<CtfeValueTypedInt<int32_t>>(b.m_val));
      }
      else if(get_long_size() == 8)
      {
        return get<CtfeValueTypedInt<int64_t>>(m_val).eq(get<CtfeValueTypedInt<int64_t>>(b.m_val));
      }
      else
      {
        pony_assert(false);
      }
    }
    else if(pony_type == "ULong")
    {
      if(get_long_size() == 4)
      {
        return get<CtfeValueTypedInt<uint32_t>>(m_val).eq(get<CtfeValueTypedInt<uint32_t>>(b.m_val));
      }
      else if(get_long_size() == 8)
      {
        return get<CtfeValueTypedInt<uint64_t>>(m_val).eq(get<CtfeValueTypedInt<uint64_t>>(b.m_val));
      }
      else
      {
        pony_assert(false);
      }
    }
    else if(pony_type == "ISize")
    {
      if(get_size_size() == 4)
      {
        return get<CtfeValueTypedInt<int32_t>>(m_val).eq(get<CtfeValueTypedInt<int32_t>>(b.m_val));
      }
      else if(get_size_size() == 8)
      {
        return get<CtfeValueTypedInt<int64_t>>(m_val).eq(get<CtfeValueTypedInt<int64_t>>(b.m_val));
      }
      else
      {
        pony_assert(false);
      }
    }
    else if(pony_type == "USize")
    {
      if(get_size_size() == 4)
      {
        return get<CtfeValueTypedInt<uint32_t>>(m_val).eq(get<CtfeValueTypedInt<uint32_t>>(b.m_val));
      }
      else if(get_size_size() == 8)
      {
        return get<CtfeValueTypedInt<uint64_t>>(m_val).eq(get<CtfeValueTypedInt<uint64_t>>(b.m_val));
      }
      else
      {
        pony_assert(false);
      }
    }
    else if (pony_type == "F32")
    {
      return get<CtfeValueTypedFloat<float>>(m_val).eq(get<CtfeValueTypedFloat<float>>(b.m_val));
    }
    else if (pony_type == "F64")
    {
      return get<CtfeValueTypedFloat<double>>(m_val).eq(get<CtfeValueTypedFloat<double>>(b.m_val));
    }
  }

  pony_assert(false);

  return CtfeValueBool(false);
}
