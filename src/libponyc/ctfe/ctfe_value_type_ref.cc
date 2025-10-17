#include "ctfe_value_type_ref.h"
#include "ctfe_exception.h"

#include "../type/subtype.h"
#include "../codegen/genopt.h"
#include "../type/assemble.h"
#include "ponyassert.h"

#include <string>


using namespace std;


CtfeValueTypeRef::CtfeValueTypeRef(ast_t* type):
  m_type(CtfeValueType::None),
  m_type_ast{type}
{
  if(is_machine_word(type))
  {
    string type_name = ast_name(ast_childidx(type, 1));

    if(type_name == "I8")
    {
      m_type = CtfeValueType::TypedIntI8;
    }
    else if(type_name == "U8")
    {
      m_type = CtfeValueType::TypedIntU8;
    }
    else if(type_name == "I16")
    {
      m_type = CtfeValueType::TypedIntI16;
    }
    else if(type_name == "U16")
    {
      m_type = CtfeValueType::TypedIntU16;
    }
    else if(type_name == "I32")
    {
      m_type = CtfeValueType::TypedIntI32;
    }
    else if(type_name == "U32")
    {
      m_type = CtfeValueType::TypedIntU32;
    }
    else if(type_name == "I64")
    {
      m_type = CtfeValueType::TypedIntI64;
    }
    else if(type_name == "U64")
    {
      m_type = CtfeValueType::TypedIntU64;
    }
    else if(type_name == "I128")
    {
      m_type = CtfeValueType::TypedIntI128;
    }
    else if(type_name == "U128")
    {
      m_type = CtfeValueType::TypedIntU128;
    }
    else if(type_name == "ILong")
    {
      m_type = CtfeValueType::TypedIntILong;
    }
    else if(type_name == "ULong")
    {
      m_type = CtfeValueType::TypedIntULong;
    }
    else if(type_name == "ISize")
    {
      m_type = CtfeValueType::TypedIntISize;
    }
    else if(type_name == "USize")
    {
      m_type = CtfeValueType::TypedIntUSize;
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(is_none(type))
  {
    m_type = CtfeValueType::None;
  }
  else
  {
    ast_t* underlying_type = (ast_t*)ast_data(type);
    if(ast_id(underlying_type) == TK_STRUCT ||
       ast_id(underlying_type) == TK_CLASS)
    {
      m_type = CtfeValueType::StructRef;
    }
    else
    {
      pony_assert(false);
    }
  }
}


bool CtfeValueTypeRef::m_static_initialized = false;
uint8_t CtfeValueTypeRef::m_long_size = 0;
uint8_t CtfeValueTypeRef::m_size_size = 0;


void CtfeValueTypeRef::initialize(pass_opt_t* opt)
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


size_t CtfeValueTypeRef::get_size_of_type(CtfeValueType type)
{
  switch(type)
  {
    case CtfeValueType::None:
      return 1;
    case CtfeValueType::Bool:
      return sizeof(uint8_t);
    case CtfeValueType::TypedIntI8:
      return sizeof(int8_t);
    case CtfeValueType::TypedIntU8:
      return sizeof(uint8_t);
    case CtfeValueType::TypedIntI16:
      return sizeof(int16_t);
    case CtfeValueType::TypedIntU16:
      return sizeof(uint16_t);
    case CtfeValueType::TypedIntI32:
      return sizeof(int32_t);
    case CtfeValueType::TypedIntU32:
      return sizeof(uint32_t);
    case CtfeValueType::TypedIntI64:
      return sizeof(int64_t);
    case CtfeValueType::TypedIntU64:
      return sizeof(uint64_t);
    case CtfeValueType::TypedIntI128:
      return sizeof(CtfeI128Type);
    case CtfeValueType::TypedIntU128:
      return sizeof(CtfeU128Type);
    case CtfeValueType::TypedIntILong:
    case CtfeValueType::TypedIntULong:
      return m_long_size;
    case CtfeValueType::TypedIntISize:
    case CtfeValueType::TypedIntUSize:
      return m_size_size;
    case CtfeValueType::StructRef:
      return sizeof(void*);
    default:
      throw CtfeValueException();
  }
}


size_t CtfeValueTypeRef::get_size_of_type() const
{
  return CtfeValueTypeRef::get_size_of_type(m_type);
}


const string CtfeValueTypeRef::get_type_name() const
{
  return ast_name(ast_childidx(m_type_ast, 1));
}


string CtfeValueTypeRef::get_pony_type_name(CtfeValueType type)
{
  switch(type)
  {
    case CtfeValueType::None:
      return "None";
    case CtfeValueType::Bool:
      return "Bool";
    case CtfeValueType::IntLiteral:
      return "Literal";
    case CtfeValueType::TypedIntI8:
      return "I8";
    case CtfeValueType::TypedIntU8:
      return "U8";
    case CtfeValueType::TypedIntI16:
      return "U16";
    case CtfeValueType::TypedIntU16:
      return "U16";
    case CtfeValueType::TypedIntI32:
      return "I32";
    case CtfeValueType::TypedIntU32:
      return "U32";
    case CtfeValueType::TypedIntI64:
      return "I64";
    case CtfeValueType::TypedIntU64:
      return "U64";
    case CtfeValueType::TypedIntI128:
      return "I128";
    case CtfeValueType::TypedIntU128:
      return "U128";
    case CtfeValueType::TypedIntILong:
      return "ILong";
    case CtfeValueType::TypedIntULong:
      return "ULong";
    case CtfeValueType::TypedIntISize:
      return "ISize";
    case CtfeValueType::TypedIntUSize:
      return "USize";
    case CtfeValueType::StructRef:
      return "Struct Reference";
    default:
      return "No type name found";
  }
}