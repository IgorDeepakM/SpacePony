#include "ctfe_value.h"
#include "ctfe_value_struct.h"
#include "ctfe_value_typed_int_run_method.h"
#include "ctfe_exception.h"

#include "ponyassert.h"
#include "../pass/pass.h"
#include "../codegen/genopt.h"
#include "../type/assemble.h"


using namespace std;

CtfeValue::CtfeValue():
  m_type{Type::None},
  m_ctrlFlow{ControlFlowModifier::None}
{

}


CtfeValue::~CtfeValue()
{
  switch(m_type)
  {
    case Type::Tuple:
      m_tuple_value.~CtfeValueTuple();
      break;
    default:
      break;
  }
}


CtfeValue::CtfeValue(Type type):
  m_type{type},
  m_ctrlFlow{ControlFlowModifier::None}
{

}


CtfeValue::CtfeValue(const CtfeValue& val):
  m_type{val.m_type},
  m_ctrlFlow{val.m_ctrlFlow}
{
  switch(val.m_type)
  {
    case Type::Bool:
      m_bool_value = val.m_bool_value;
      break;
    case Type::IntLiteral:
      m_int_literal_value = val.m_int_literal_value;
      break;
    case Type::TypedIntI8:
      m_i8_value = val.m_i8_value;
      break;
    case Type::TypedIntU8:
      m_u8_value = val.m_u8_value;
      break;
    case Type::TypedIntI16:
      m_i16_value = val.m_i16_value;
      break;
    case Type::TypedIntU16:
      m_u16_value = val.m_u16_value;
      break;
    case Type::TypedIntI32:
      m_i32_value = val.m_i32_value;
      break;
    case Type::TypedIntU32:
      m_u32_value = val.m_u32_value;
      break;
    case Type::TypedIntI64:
      m_i64_value = val.m_i64_value;
      break;
    case Type::TypedIntU64:
      m_u64_value = val.m_u64_value;
      break;
    case Type::TypedIntILong:
      if(m_long_size == 4)
      {
        m_i32_value = val.m_i32_value;
      }
      else if(m_long_size == 8)
      {
        m_i64_value = val.m_i64_value;
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntULong:
      if(m_long_size == 4)
      {
        m_u32_value = val.m_u32_value;
      }
      else if(m_long_size == 8)
      {
        m_u64_value = val.m_u64_value;
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntISize:
      if(m_size_size == 4)
      {
        m_i32_value = val.m_i32_value;
      }
      else if(m_size_size == 8)
      {
        m_i64_value = val.m_i64_value;
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntUSize:
      if(m_size_size == 4)
      {
        m_u32_value = val.m_u32_value;
      }
      else if(m_size_size == 8)
      {
        m_u64_value = val.m_u64_value;
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::StructRef:
      m_struct_ref = val.m_struct_ref;
      break;
    case Type::Tuple:
      // Trickery so that we can run the constructor outside a
      // constructor and run it lazily
      new(&m_tuple_value) CtfeValueTuple();
      m_tuple_value = val.m_tuple_value;
      break;
    case Type::StringLiteral:
      new(&m_string_literal_value) string();
      m_string_literal_value = val.m_string_literal_value;
      break;
    default:
      break;
  }
}


CtfeValue::CtfeValue(const CtfeValueIntLiteral& val):
  m_type{Type::IntLiteral},
  m_ctrlFlow{ControlFlowModifier::None}
{
  m_int_literal_value = val;
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
      convert_from_int_literal_to_type(val.m_int_literal_value, pony_type);
      break;
    default:
      break;
  }
}


CtfeValue::CtfeValue(CtfeValueStruct* ref):
  m_type{Type::StructRef},
  m_ctrlFlow{ControlFlowModifier::None}
{
  m_struct_ref = ref;
}


CtfeValue::CtfeValue(const CtfeValueBool& val):
  m_type{Type::Bool},
  m_ctrlFlow{ControlFlowModifier::None}
{
  m_bool_value = val;
}


CtfeValue::CtfeValue(const CtfeValueTuple& val):
  m_type{Type::Tuple},
  m_ctrlFlow{ControlFlowModifier::None},
  m_tuple_value(val)
{

}


CtfeValue::CtfeValue(const CtfeValueStringLiteral& str):
  m_type{Type::StringLiteral},
  m_ctrlFlow{ControlFlowModifier::None},
  m_string_literal_value{str}
{

}


CtfeValue& CtfeValue::operator=(const CtfeValue& val)
{
  switch(m_type)
  {
    case Type::Tuple:
      m_tuple_value.~CtfeValueTuple();
      break;
    case Type::StringLiteral:
      m_string_literal_value.~CtfeValueStringLiteral();
      break;
    default:
      break;
  }

  switch(val.m_type)
  {
    case Type::Bool:
      m_bool_value = val.m_bool_value;
      break;
    case Type::IntLiteral:
      m_int_literal_value = val.m_int_literal_value;
      break;
    case Type::TypedIntI8:
      m_i8_value = val.m_i8_value;
      break;
    case Type::TypedIntU8:
      m_u8_value = val.m_u8_value;
      break;
    case Type::TypedIntI16:
      m_i16_value = val.m_i16_value;
      break;
    case Type::TypedIntU16:
      m_u16_value = val.m_u16_value;
      break;
    case Type::TypedIntI32:
      m_i32_value = val.m_i32_value;
      break;
    case Type::TypedIntU32:
      m_u32_value = val.m_u32_value;
      break;
    case Type::TypedIntI64:
      m_i64_value = val.m_i64_value;
      break;
    case Type::TypedIntU64:
      m_u64_value = val.m_u64_value;
      break;
    case Type::TypedIntILong:
      if(m_long_size == 4)
      {
        m_i32_value = val.m_i32_value;
      }
      else if(m_long_size == 8)
      {
        m_i64_value = val.m_i64_value;
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntULong:
      if(m_long_size == 4)
      {
        m_u32_value = val.m_u32_value;
      }
      else if(m_long_size == 8)
      {
        m_u64_value = val.m_u64_value;
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntISize:
      if(m_size_size == 4)
      {
        m_i32_value = val.m_i32_value;
      }
      else if(m_size_size == 8)
      {
        m_i64_value = val.m_i64_value;
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntUSize:
      if(m_size_size == 4)
      {
        m_u32_value = val.m_u32_value;
      }
      else if(m_size_size == 8)
      {
        m_u64_value = val.m_u64_value;
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::StructRef:
      m_struct_ref = val.m_struct_ref;
      break;
    case Type::Tuple:
      if(m_type != Type::Tuple)
      {
        new(&m_tuple_value) CtfeValueTuple();
      }
      m_tuple_value = val.m_tuple_value;
      break;
    case Type::StringLiteral:
      if(m_type != Type::StringLiteral)
      {
        new(&m_string_literal_value) string();
      }
      m_string_literal_value = val.m_string_literal_value;
      break;
    default:
      break;
  }

  m_type = val.m_type;
  m_ctrlFlow = val.m_ctrlFlow;

  return *this;
}


void CtfeValue::convert_from_int_literal_to_type(const CtfeValueIntLiteral& val,
  const std::string& pony_type)
{
  if(pony_type == "I8")
  {
    m_i8_value = CtfeValueTypedInt<int8_t>(val);
    m_type = Type::TypedIntI8;
  }
  else if(pony_type == "U8")
  {
    m_u8_value = CtfeValueTypedInt<uint8_t>(val);
    m_type = Type::TypedIntU8;
  }
  else if(pony_type == "I16")
  {
    m_i16_value = CtfeValueTypedInt<int16_t>(val);
    m_type = Type::TypedIntI16;
  }
  else if(pony_type == "U16")
  {
    m_u16_value = CtfeValueTypedInt<uint16_t>(val);
    m_type = Type::TypedIntU16;
  }
  else if(pony_type == "I32")
  {
    m_i32_value = CtfeValueTypedInt<int32_t>(val);
    m_type = Type::TypedIntI32;
  }
  else if(pony_type == "U32")
  {
    m_u32_value = CtfeValueTypedInt<uint32_t>(val);
    m_type = Type::TypedIntU32;
  }
  else if(pony_type == "I64")
  {
    m_i64_value = CtfeValueTypedInt<int64_t>(val);
    m_type = Type::TypedIntI64;
  }
  else if(pony_type == "U64")
  {
    m_u64_value = CtfeValueTypedInt<uint64_t>(val);
    m_type = Type::TypedIntU64;
  }
  else if(pony_type == "ILong")
  {
    if(m_long_size == 4)
    {
      m_i32_value = CtfeValueTypedInt<int32_t>(val);
    }
    else if(m_long_size == 8)
    {
      m_i64_value = CtfeValueTypedInt<int64_t>(val);
    }
    else
    {
      pony_assert(false);
    }
    m_type = Type::TypedIntILong;
  }
  else if(pony_type == "ULong")
  {
    if(m_long_size == 4)
    {
      m_u32_value = CtfeValueTypedInt<uint32_t>(val);
    }
    else if(m_long_size == 8)
    {
      m_u64_value = CtfeValueTypedInt<uint64_t>(val);
    }
    else
    {
      pony_assert(false);
    }
    m_type = Type::TypedIntULong;
  }
  else if(pony_type == "ISize")
  {
    if(m_size_size == 4)
    {
      m_i32_value = CtfeValueTypedInt<int32_t>(val);
    }
    else if(m_size_size == 8)
    {
      m_i64_value = CtfeValueTypedInt<int64_t>(val);
    }
    else
    {
      pony_assert(false);
    }
    m_type = Type::TypedIntISize;
  }
  else if(pony_type == "USize")
  {
     if(m_size_size == 4)
    {
      m_u32_value = CtfeValueTypedInt<uint32_t>(val);
    }
    else if(m_size_size == 8)
    {
      m_u64_value = CtfeValueTypedInt<uint64_t>(val);
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
      return m_bool_value.create_ast_literal_node(opt, from);
    case Type::IntLiteral:
      return m_int_literal_value.create_ast_literal_node();
    case Type::TypedIntI8:
      new_node = m_i8_value.create_ast_literal_node();
      break;
    case Type::TypedIntU8:
      new_node = m_u8_value.create_ast_literal_node();
      break;
    case Type::TypedIntI16:
      new_node = m_i16_value.create_ast_literal_node();
      break;
    case Type::TypedIntU16:
      new_node = m_u16_value.create_ast_literal_node();
      break;
    case Type::TypedIntI32:
      new_node = m_i32_value.create_ast_literal_node();
      break;
    case Type::TypedIntU32:
      new_node = m_u32_value.create_ast_literal_node();
      break;
    case Type::TypedIntI64:
      new_node = m_i64_value.create_ast_literal_node();
      break;
    case Type::TypedIntU64:
      new_node = m_u64_value.create_ast_literal_node();
      break;
    case Type::TypedIntILong:
      if(m_long_size == 4)
      {
        new_node = m_i32_value.create_ast_literal_node();
      }
      else if(m_long_size == 8)
      {
        new_node = m_i64_value.create_ast_literal_node();
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntULong:
      if(m_long_size == 4)
      {
        new_node = m_u32_value.create_ast_literal_node();
      }
      else if(m_long_size == 8)
      {
        new_node = m_u64_value.create_ast_literal_node();
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntISize:
      if(m_size_size == 4)
      {
        new_node = m_i32_value.create_ast_literal_node();
      }
      else if(m_size_size == 8)
      {
        new_node = m_i64_value.create_ast_literal_node();
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntUSize:
      if(m_size_size == 4)
      {
        new_node = m_u32_value.create_ast_literal_node();
      }
      else if(m_size_size == 8)
      {
        new_node = m_u64_value.create_ast_literal_node();
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::StringLiteral:
      new_node = m_string_literal_value.create_ast_literal_node(opt, from);
      break;
    case Type::StructRef:
      ast_error_frame(errors, from,
              "It is currently not possible to create a literal from "
              "a class or struct reference.");
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
  const std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result)
{
  if(args.size() < 1)
  {
    return false;
  }

  switch(args[0].get_type())
  {
    case CtfeValue::Type::Bool:
      return CtfeValueBool::run_method(opt, errors, ast, args, method_name, result);
    case CtfeValue::Type::IntLiteral:
      return CtfeValueIntLiteral::run_method(opt, errors, ast, args, method_name, result);
    case Type::TypedIntI8:
      return CtfeValueTypedInt<int8_t>::run_method(opt, errors, ast, args, method_name, result);
    case Type::TypedIntU8:
      return CtfeValueTypedInt<uint8_t>::run_method(opt, errors, ast, args, method_name, result);
    case Type::TypedIntI16:
      return CtfeValueTypedInt<int16_t>::run_method(opt, errors, ast, args, method_name, result);
    case Type::TypedIntU16:
      return CtfeValueTypedInt<uint16_t>::run_method(opt, errors, ast, args, method_name, result);
    case Type::TypedIntI32:
      return CtfeValueTypedInt<int32_t>::run_method(opt, errors, ast, args, method_name, result);
    case Type::TypedIntU32:
      return CtfeValueTypedInt<uint32_t>::run_method(opt, errors, ast, args, method_name, result);
    case Type::TypedIntI64:
      return CtfeValueTypedInt<int64_t>::run_method(opt, errors, ast, args, method_name, result);
    case Type::TypedIntU64:
      return CtfeValueTypedInt<uint64_t>::run_method(opt, errors, ast, args, method_name, result);
    case Type::TypedIntILong:
      if(m_long_size == 4)
      {
        return CtfeValueTypedInt<int32_t>::run_method(opt, errors, ast, args, method_name, result);
      }
      else if(m_long_size == 8)
      {
        return CtfeValueTypedInt<int64_t>::run_method(opt, errors, ast, args, method_name, result);
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntULong:
      if(m_long_size == 4)
      {
        return CtfeValueTypedInt<uint32_t>::run_method(opt, errors, ast, args, method_name, result);
      }
      else if(m_long_size == 8)
      {
        return CtfeValueTypedInt<uint64_t>::run_method(opt, errors, ast, args, method_name, result);
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntISize:
      if(m_size_size == 4)
      {
        return CtfeValueTypedInt<int32_t>::run_method(opt, errors, ast, args, method_name, result);
      }
      else if(m_size_size == 8)
      {
        return CtfeValueTypedInt<int64_t>::run_method(opt, errors, ast, args, method_name, result);
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::TypedIntUSize:
      if(m_size_size == 4)
      {
        return CtfeValueTypedInt<uint32_t>::run_method(opt, errors, ast, args, method_name, result);
      }
      else if(m_size_size == 8)
      {
        return CtfeValueTypedInt<uint64_t>::run_method(opt, errors, ast, args, method_name, result);
      }
      else
      {
        pony_assert(false);
      }
      break;
    case Type::StringLiteral:
      return CtfeValueStringLiteral::run_method(opt, errors, ast, args, method_name, result);
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
      return m_int_literal_value.to_uint64();
    case Type::TypedIntI8:
      return m_i8_value.to_uint64();
    case Type::TypedIntU8:
      return m_u8_value.to_uint64();
    case Type::TypedIntI16:
      return m_i16_value.to_uint64();
    case Type::TypedIntU16:
      return m_u16_value.to_uint64();
    case Type::TypedIntI32:
      return m_i32_value.to_uint64();
    case Type::TypedIntU32:
      return m_u32_value.to_uint64();
    case Type::TypedIntI64:
      return m_i64_value.to_uint64();
    case Type::TypedIntU64:
      return m_u64_value.to_uint64();
   case Type::TypedIntILong:
      if(m_long_size == 4)
      {
        return m_i32_value.to_uint64();
      }
      else if(m_long_size == 8)
      {
        return m_i64_value.to_uint64();
      }
      break;
    case Type::TypedIntULong:
      if(m_long_size == 4)
      {
        return m_u32_value.to_uint64();
      }
      else if(m_long_size == 8)
      {
        return m_u64_value.to_uint64();
      }
      break;
    case Type::TypedIntISize:
      if(m_size_size == 4)
      {
        return m_i32_value.to_uint64();
      }
      else if(m_size_size == 8)
      {
        return m_i64_value.to_uint64();
      }
      break;
    case Type::TypedIntUSize:
      if(m_size_size == 4)
      {
        return m_u32_value.to_uint64();
      }
      else if(m_size_size == 8)
      {
        return m_u64_value.to_uint64();
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


bool CtfeValue::m_static_initialized = false;
uint8_t CtfeValue::m_long_size = 0;
uint8_t CtfeValue::m_size_size = 0;


void CtfeValue::initialize(pass_opt_t* opt)
{
  if(!m_static_initialized)
  {
    char* triple = opt->triple;
    bool ilp32 = target_is_ilp32(triple);
    bool llp64 = target_is_llp64(triple);
    bool lp64 = target_is_lp64(triple);

    if(ilp32)
    {
      m_long_size = 4;
      m_size_size = 4;
    }
    else if(lp64)
    {
      m_long_size = 8;
      m_size_size = 8;
    }
    else if(llp64)
    {
      m_long_size = 4;
      m_size_size = 8;
    }

    m_static_initialized = true;
  }
}


string CtfeValue::get_pony_type_name() const
{
  switch(m_type)
  {
    case Type::Bool:
      return "Bool";
    case Type::IntLiteral:
      return "Literal";
    case Type::TypedIntI8:
      return "I8";
    case Type::TypedIntU8:
      return "U8";
    case Type::TypedIntI16:
      return "U16";
    case Type::TypedIntU16:
      return "U16";
    case Type::TypedIntI32:
      return "I32";
    case Type::TypedIntU32:
      return "U32";
    case Type::TypedIntI64:
      return "I64";
    case Type::TypedIntU64:
      return "U64";
    case Type::TypedIntILong:
      return "ILong";
    case Type::TypedIntULong:
      return "ULong";
    case Type::TypedIntISize:
      return "ISize";
    case Type::TypedIntUSize:
      return "USize";
    case Type::StructRef:
      return "Struct Reference";
    default:
      return "No type name found";
  }
}