#include "ctfe_value.h"
#include "ctfe_value_struct.h"
#include "ctfe_value_typed_int_run_method.h"
#include "ctfe_exception.h"

#include "ponyassert.h"
#include "../pass/pass.h"
#include "../type/assemble.h"


using namespace std;

CtfeValue::CtfeValue():
  m_type{Type::None},
  m_ctrlFlow{ControlFlowModifier::None}
{

}


CtfeValue::~CtfeValue()
{

}


CtfeValue::CtfeValue(Type type):
  m_type{type},
  m_ctrlFlow{ControlFlowModifier::None}
{

}


CtfeValue::CtfeValue(const CtfeValue& val):
  m_type{val.m_type},
  m_ctrlFlow{val.m_ctrlFlow},
  m_val{val.m_val}
{

}


CtfeValue::CtfeValue(const CtfeValueIntLiteral& val):
  m_type{Type::IntLiteral},
  m_ctrlFlow{ControlFlowModifier::None}
{
  m_val = val;
}


CtfeValue::CtfeValue(const CtfeValueIntLiteral& val, const std::string& pony_type):
  m_type{Type::IntLiteral},
  m_ctrlFlow{ControlFlowModifier::None}
{
  convert_from_int_literal_to_type(val, pony_type);
}


CtfeValue::CtfeValue(const CtfeValue& val, const std::string& pony_type):
  m_type{Type::None},
  m_ctrlFlow{ControlFlowModifier::None}
{
  switch(val.m_type)
  {
    case Type::IntLiteral:
      convert_from_int_literal_to_type(m_val, pony_type);
      break;
    default:
      break;
  }
}


CtfeValue::CtfeValue(CtfeValueStruct* ref):
  m_type{Type::StructRef},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{ref}
{

}


CtfeValue::CtfeValue(const CtfeValueBool& val):
  m_type{Type::Bool},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{val}
{

}


CtfeValue::CtfeValue(const CtfeValueTuple& val):
  m_type{Type::Tuple},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{val}
{

}


CtfeValue::CtfeValue(const CtfeValueStringLiteral& str):
  m_type{Type::StringLiteral},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{str}
{

}


CtfeValue::CtfeValue(const CtfeValuePointer& p):
  m_type{Type::Pointer},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{p}
{

}


CtfeValue::CtfeValue(const CtfeValueTypeRef& t):
  m_type{Type::Pointer},
  m_ctrlFlow{ControlFlowModifier::None},
  m_val{t}
{

}


CtfeValue& CtfeValue::operator=(const CtfeValue& val)
{
  m_type = val.m_type;
  m_ctrlFlow = val.m_ctrlFlow;
  m_val = val.m_val;

  return *this;
}


void CtfeValue::convert_from_int_literal_to_type(const CtfeValueIntLiteral& val,
  const std::string& pony_type)
{
  if(pony_type == "I8")
  {
    m_val = CtfeValueTypedInt<int8_t>(val);
    m_type = Type::TypedIntI8;
  }
  else if(pony_type == "U8")
  {
    m_val = CtfeValueTypedInt<uint8_t>(val);
    m_type = Type::TypedIntU8;
  }
  else if(pony_type == "I16")
  {
    m_val = CtfeValueTypedInt<int16_t>(val);
    m_type = Type::TypedIntI16;
  }
  else if(pony_type == "U16")
  {
    m_val = CtfeValueTypedInt<uint16_t>(val);
    m_type = Type::TypedIntU16;
  }
  else if(pony_type == "I32")
  {
    m_val = CtfeValueTypedInt<int32_t>(val);
    m_type = Type::TypedIntI32;
  }
  else if(pony_type == "U32")
  {
    m_val = CtfeValueTypedInt<uint32_t>(val);
    m_type = Type::TypedIntU32;
  }
  else if(pony_type == "I64")
  {
    m_val = CtfeValueTypedInt<int64_t>(val);
    m_type = Type::TypedIntI64;
  }
  else if(pony_type == "U64")
  {
    m_val = CtfeValueTypedInt<uint64_t>(val);
    m_type = Type::TypedIntU64;
  }
   else if(pony_type == "I128")
  {
    m_val = CtfeValueTypedInt<CtfeI128Type>(val);
    m_type = Type::TypedIntI128;
  }
  else if(pony_type == "U128")
  {
    m_val = CtfeValueTypedInt<CtfeU128Type>(val);
    m_type = Type::TypedIntU128;
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
    m_type = Type::TypedIntILong;
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
    m_type = Type::TypedIntULong;
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
    m_type = Type::TypedIntISize;
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
    m_type = Type::TypedIntUSize;
  }
}


ast_t* CtfeValue::create_ast_literal_node(pass_opt_t* opt, errorframe_t* errors,
  ast_t* from)
{
  ast_t* new_node = NULL;

  switch(m_type)
  {
    case Type::Bool:
      return get<CtfeValueBool>(m_val).create_ast_literal_node(opt, from);
    case Type::IntLiteral:
      return get<CtfeValueIntLiteral>(m_val).create_ast_literal_node();
    case Type::TypedIntI8:
      new_node = get<CtfeValueTypedInt<int8_t>>(m_val).create_ast_literal_node();
      break;
    case Type::TypedIntU8:
      new_node =  get<CtfeValueTypedInt<uint8_t>>(m_val).create_ast_literal_node();
      break;
    case Type::TypedIntI16:
      new_node =  get<CtfeValueTypedInt<int16_t>>(m_val).create_ast_literal_node();
      break;
    case Type::TypedIntU16:
      new_node =  get<CtfeValueTypedInt<uint16_t>>(m_val).create_ast_literal_node();
      break;
    case Type::TypedIntI32:
      new_node =  get<CtfeValueTypedInt<int32_t>>(m_val).create_ast_literal_node();
      break;
    case Type::TypedIntU32:
      new_node =  get<CtfeValueTypedInt<uint32_t>>(m_val).create_ast_literal_node();
      break;
    case Type::TypedIntI64:
      new_node =  get<CtfeValueTypedInt<int64_t>>(m_val).create_ast_literal_node();
      break;
    case Type::TypedIntU64:
      new_node =  get<CtfeValueTypedInt<uint64_t>>(m_val).create_ast_literal_node();
      break;
    case Type::TypedIntI128:
      new_node =  get<CtfeValueTypedInt<CtfeI128Type>>(m_val).create_ast_literal_node();
      break;
    case Type::TypedIntU128:
      new_node =  get<CtfeValueTypedInt<CtfeU128Type>>(m_val).create_ast_literal_node();
      break;
    case Type::TypedIntILong:
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
      break;
    case Type::TypedIntULong:
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
      break;
    case Type::TypedIntISize:
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
      break;
    case Type::TypedIntUSize:
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
      break;
    case Type::StringLiteral:
      new_node = get<CtfeValueStringLiteral>(m_val).create_ast_literal_node(opt, from);
      break;
    case Type::StructRef:
      ast_error_frame(errors, from,
              "It is currently not possible to create a literal from "
              "a class or struct reference.");
      return NULL;
    default:
      return NULL;
  }

  if(is_typed_int())
  {
    ast_t* type = type_builtin(opt, from, get_pony_type_name().c_str());
    ast_settype(new_node, type);
  }

  return new_node;
}


bool CtfeValue::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, CtfeRunner &ctfeRunner)
{
  switch(recv.get_type())
  {
    case CtfeValue::Type::Bool:
      return CtfeValueBool::run_method(opt, errors, ast, recv, args, method_name, result);
    case CtfeValue::Type::IntLiteral:
      return CtfeValueIntLiteral::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::TypedIntI8:
      return CtfeValueTypedInt<int8_t>::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::TypedIntU8:
      return CtfeValueTypedInt<uint8_t>::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::TypedIntI16:
      return CtfeValueTypedInt<int16_t>::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::TypedIntU16:
      return CtfeValueTypedInt<uint16_t>::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::TypedIntI32:
      return CtfeValueTypedInt<int32_t>::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::TypedIntU32:
      return CtfeValueTypedInt<uint32_t>::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::TypedIntI64:
      return CtfeValueTypedInt<int64_t>::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::TypedIntU64:
      return CtfeValueTypedInt<uint64_t>::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::TypedIntI128:
      return CtfeValueTypedInt<CtfeI128Type>::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::TypedIntU128:
      return CtfeValueTypedInt<CtfeU128Type>::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::TypedIntILong:
      if(get_long_size() == 4)
      {
        return CtfeValueTypedInt<int32_t>::run_method(opt, errors, ast, recv, args, method_name, result);
      }
      else if(get_long_size() == 8)
      {
        return CtfeValueTypedInt<int64_t>::run_method(opt, errors, ast, recv, args, method_name, result);
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntULong:
      if(get_long_size() == 4)
      {
        return CtfeValueTypedInt<uint32_t>::run_method(opt, errors, ast, recv, args, method_name, result);
      }
      else if(get_long_size() == 8)
      {
        return CtfeValueTypedInt<uint64_t>::run_method(opt, errors, ast, recv, args, method_name, result);
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntISize:
      if(get_size_size() == 4)
      {
        return CtfeValueTypedInt<int32_t>::run_method(opt, errors, ast, recv, args, method_name, result);
      }
      else if(get_size_size() == 8)
      {
        return CtfeValueTypedInt<int64_t>::run_method(opt, errors, ast, recv, args, method_name, result);
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntUSize:
      if(get_size_size() == 4)
      {
        return CtfeValueTypedInt<uint32_t>::run_method(opt, errors, ast, recv, args, method_name, result);
      }
      else if(get_size_size() == 8)
      {
        return CtfeValueTypedInt<uint64_t>::run_method(opt, errors, ast, recv, args, method_name, result);
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::StringLiteral:
      return CtfeValueStringLiteral::run_method(opt, errors, ast, recv, args, method_name, result);
    case Type::Pointer:
      return CtfeValuePointer::run_method(opt, errors, ast, recv, args, method_name, result, ctfeRunner);
    default:
      break;
  }

  return false;
}


uint64_t CtfeValue::to_uint64() const
{
  switch(m_type)
  {
    case Type::IntLiteral:
      return get<CtfeValueIntLiteral>(m_val).to_uint64();
    case Type::TypedIntI8:
      return get<CtfeValueTypedInt<int8_t>>(m_val).to_uint64();
    case Type::TypedIntU8:
      return get<CtfeValueTypedInt<uint8_t>>(m_val).to_uint64();
    case Type::TypedIntI16:
      return get<CtfeValueTypedInt<int16_t>>(m_val).to_uint64();
    case Type::TypedIntU16:
      return get<CtfeValueTypedInt<uint16_t>>(m_val).to_uint64();
    case Type::TypedIntI32:
      return get<CtfeValueTypedInt<int32_t>>(m_val).to_uint64();
    case Type::TypedIntU32:
      return get<CtfeValueTypedInt<uint32_t>>(m_val).to_uint64();
    case Type::TypedIntI64:
      return get<CtfeValueTypedInt<int64_t>>(m_val).to_uint64();
    case Type::TypedIntU64:
      return get<CtfeValueTypedInt<uint64_t>>(m_val).to_uint64();
    case Type::TypedIntI128:
      return get<CtfeValueTypedInt<CtfeI128Type>>(m_val).to_uint64();
    case Type::TypedIntU128:
      return get<CtfeValueTypedInt<CtfeU128Type>>(m_val).to_uint64();
   case Type::TypedIntILong:
      if(get_long_size() == 4)
      {
        return get<CtfeValueTypedInt<int32_t>>(m_val).to_uint64();
      }
      else if(get_long_size() == 8)
      {
        return get<CtfeValueTypedInt<int64_t>>(m_val).to_uint64();
      }
      break;
    case Type::TypedIntULong:
      if(get_long_size() == 4)
      {
        return get<CtfeValueTypedInt<uint32_t>>(m_val).to_uint64();
      }
      else if(get_long_size() == 8)
      {
        return get<CtfeValueTypedInt<uint64_t>>(m_val).to_uint64();
      }
      break;
    case Type::TypedIntISize:
      if(get_size_size() == 4)
      {
        return get<CtfeValueTypedInt<int32_t>>(m_val).to_uint64();
      }
      else if(get_size_size() == 8)
      {
        return get<CtfeValueTypedInt<int64_t>>(m_val).to_uint64();
      }
      break;
    case Type::TypedIntUSize:
      if(get_size_size() == 4)
      {
        return get<CtfeValueTypedInt<uint32_t>>(m_val).to_uint64();
      }
      else if(get_size_size() == 8)
      {
        return get<CtfeValueTypedInt<uint64_t>>(m_val).to_uint64();
      }
      break;
    default:
      break;
  }

  pony_assert(false);

  return 0;
}


bool CtfeValue::is_typed_int() const
{
  switch(m_type)
  {
    case Type::TypedIntI8:
    case Type::TypedIntU8:
    case Type::TypedIntI16:
    case Type::TypedIntU16:
    case Type::TypedIntI32:
    case Type::TypedIntU32:
    case Type::TypedIntI64:
    case Type::TypedIntU64:
    case Type::TypedIntI128:
    case Type::TypedIntU128:
    case Type::TypedIntILong:
    case Type::TypedIntULong:
    case Type::TypedIntISize:
    case Type::TypedIntUSize:
      return true;
    default:
      break;
  }

  return false;
}


void CtfeValue::write_to_memory(uint8_t* ptr) const
{
  switch(m_type)
  {
    case Type::Bool:
      get<CtfeValueBool>(m_val).write_to_memory(ptr);
      break;
    case Type::TypedIntI8:
      get<CtfeValueTypedInt<int8_t>>(m_val).write_to_memory(ptr);
      break;
    case Type::TypedIntU8:
      get<CtfeValueTypedInt<uint8_t>>(m_val).write_to_memory(ptr);
      break;
    case Type::TypedIntI16:
      get<CtfeValueTypedInt<int16_t>>(m_val).write_to_memory(ptr);
      break;
    case Type::TypedIntU16:
      get<CtfeValueTypedInt<uint16_t>>(m_val).write_to_memory(ptr);
      break;
    case Type::TypedIntI32:
      get<CtfeValueTypedInt<int32_t>>(m_val).write_to_memory(ptr);
      break;
    case Type::TypedIntU32:
      get<CtfeValueTypedInt<uint32_t>>(m_val).write_to_memory(ptr);
      break;
    case Type::TypedIntI64:
      get<CtfeValueTypedInt<int64_t>>(m_val).write_to_memory(ptr);
      break;
    case Type::TypedIntU64:
      get<CtfeValueTypedInt<uint64_t>>(m_val).write_to_memory(ptr);
      break;
    case Type::TypedIntI128:
      get<CtfeValueTypedInt<CtfeI128Type>>(m_val).write_to_memory(ptr);
      break;
    case Type::TypedIntU128:
      get<CtfeValueTypedInt<CtfeU128Type>>(m_val).write_to_memory(ptr);
      break;
    case Type::TypedIntILong:
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
      break;
    case Type::TypedIntULong:
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
      break;
    case Type::TypedIntISize:
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
      break;
    case Type::TypedIntUSize:
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
      break;
    default:
      throw CtfeValueException();
      break;
  }
}


CtfeValue CtfeValue::read_from_memory(Type type, uint8_t* ptr)
{
  switch(type)
  {
    case Type::Bool:
      return CtfeValueBool::read_from_memory(ptr);
    case Type::TypedIntI8:
      return CtfeValueTypedInt<int8_t>::read_from_memory(ptr);
    case Type::TypedIntU8:
      return CtfeValueTypedInt<uint8_t>::read_from_memory(ptr);
    case Type::TypedIntI16:
      return CtfeValueTypedInt<int16_t>::read_from_memory(ptr);
    case Type::TypedIntU16:
      return CtfeValueTypedInt<uint16_t>::read_from_memory(ptr);
    case Type::TypedIntI32:
      return CtfeValueTypedInt<int32_t>::read_from_memory(ptr);
    case Type::TypedIntU32:
      return CtfeValueTypedInt<uint32_t>::read_from_memory(ptr);
    case Type::TypedIntI64:
      return CtfeValueTypedInt<int64_t>::read_from_memory(ptr);
    case Type::TypedIntU64:
      return CtfeValueTypedInt<uint64_t>::read_from_memory(ptr);
    case Type::TypedIntI128:
      return CtfeValueTypedInt<CtfeI128Type>::read_from_memory(ptr);
    case Type::TypedIntU128:
      return CtfeValueTypedInt<CtfeU128Type>::read_from_memory(ptr);
      break;
    case Type::TypedIntILong:
      if(get_long_size() == 4)
      {
        return CtfeValueTypedInt<int32_t>::read_from_memory(ptr);
      }
      else if(get_long_size() == 8)
      {
        return CtfeValueTypedInt<int64_t>::read_from_memory(ptr);
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntULong:
      if(get_long_size() == 4)
      {
        return CtfeValueTypedInt<uint32_t>::read_from_memory(ptr);
      }
      else if(get_long_size() == 8)
      {
        return CtfeValueTypedInt<uint64_t>::read_from_memory(ptr);
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntISize:
      if(get_size_size() == 4)
      {
        return CtfeValueTypedInt<int32_t>::read_from_memory(ptr);
      }
      else if(get_size_size() == 8)
      {
        return CtfeValueTypedInt<int64_t>::read_from_memory(ptr);
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntUSize:
      if(get_size_size() == 4)
      {
        return CtfeValueTypedInt<uint32_t>::read_from_memory(ptr);
      }
      else if(get_size_size() == 8)
      {
        return CtfeValueTypedInt<uint64_t>::read_from_memory(ptr);
      }
      else
      {
        pony_assert(false);
      }
      break;
    default:
      throw CtfeValueException();
      break;
  }

  return CtfeValue();
}