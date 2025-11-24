#include "ctfe_ast_type.h"
#include "ctfe_exception.h"
#include "ctfe_value.h"

#include "../type/subtype.h"
#include "../codegen/genopt.h"
#include "../type/assemble.h"
#include "ponyassert.h"

#include <string>


using namespace std;

bool CtfeAstType::m_static_initialized = false;
uint8_t CtfeAstType::m_long_size = 0;
uint8_t CtfeAstType::m_size_size = 0;


void CtfeAstType::initialize(pass_opt_t* opt)
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


size_t CtfeAstType::get_size_of_type(ast_t* type)
{
  if(is_nominal(type))
  {
    if(is_machine_word(type))
    {
      const string type_name = ast_name(ast_childidx(type, 1));

      if(type_name == "Bool")
      {
        return sizeof(uint8_t);
      }
      else if(type_name == "I8")
      {
        return sizeof(int8_t);
      }
      else if(type_name == "U8")
      {
        return sizeof(uint8_t);
      }
      else if(type_name == "I16")
      {
        return sizeof(int16_t);
      }
      else if(type_name == "U16")
      {
        return sizeof(uint16_t);
      }
      else if(type_name == "I32")
      {
        return sizeof(int32_t);
      }
      else if(type_name == "U32")
      {
        return sizeof(uint32_t);
      }
      else if(type_name == "I64")
      {
        return sizeof(int64_t);
      }
      else if(type_name == "U64")
      {
        return sizeof(uint64_t);
      }
      else if(type_name == "I128")
      {
        return sizeof(CtfeI128Type);
      }
      else if(type_name == "U128")
      {
        return sizeof(CtfeU128Type);
      }
      else if(type_name == "ILong")
      {
        return m_long_size;
      }
      else if(type_name == "ULong")
      {
        return m_long_size;
      }
      else if(type_name == "ISize")
      {
        return m_size_size;
      }
      else if(type_name == "USize")
      {
        return m_size_size;
      }
    }
    if(is_struct(type) || is_interface(type))
    {
      return sizeof(void*);
    }
    else if (is_primitive(type))
    {
      return static_cast<size_t>(align_val_t(sizeof(CtfeValue)));
    }
    else
    {
      pony_assert(false);
    }
  }
  else if(is_tuple(type) || is_union_type(type))
  {
    return static_cast<size_t>(align_val_t(sizeof(CtfeValue)));
  }
  else
  {
    pony_assert(false);
  }

  return 0;
}


const string CtfeAstType::get_type_name(ast_t* type)
{
  return ast_name(ast_childidx(type, 1));
}


bool CtfeAstType::is_struct(ast_t* ast)
{
  if(!is_nominal(ast))
  {
    return false;
  }

  ast_t* underlying_type = (ast_t*)ast_data(ast);
  if(ast_id(underlying_type) == TK_STRUCT ||
      ast_id(underlying_type) == TK_CLASS)
  {
    return true;
  }

  return false;
}


bool CtfeAstType::is_primitive(ast_t* ast)
{
  if(!is_nominal(ast))
  {
    return false;
  }

  ast_t* underlying_type = (ast_t*)ast_data(ast);
  if(ast_id(underlying_type) == TK_PRIMITIVE)
  {
    return true;
  }

  return false;
}


bool CtfeAstType::is_interface(ast_t* ast)
{
  if(!is_nominal(ast))
  {
    return false;
  }

  ast_t* underlying_type = (ast_t*)ast_data(ast);
  if(ast_id(underlying_type) == TK_INTERFACE)
  {
    return true;
  }

  return false;
}


uint64_t CtfeAstType::ast_hash(ast_t* ast)
{
  uint64_t hashtype = 0;

  ast_t* type = ast_type(ast);
  if(type != NULL)
  {
    hashtype = ast_hash(type);
  }

  switch(ast_id(ast))
  {
    case TK_ID:
    case TK_STRING:
    {
      const char* name = ast_name(ast);
      if(name != NULL)
      {
        return ponyint_hash_str(ast_name(ast)) ^ hashtype;
      }

      return 0;
    }

    case TK_INT:
    {
      lexint_t* val = ast_int(ast);
      return val->low ^ val->high ^ hashtype;
    }

    case TK_FLOAT:
      return (uint64_t)ast_float(ast) ^ hashtype;

    default:
      break;
  }

  size_t hash = ast_id(ast);
  ast_t* child = ast_child(ast);

  while(child)
  {
    hash ^= ast_hash(child);
    child = ast_sibling(child);
  }

  return hash;
}

