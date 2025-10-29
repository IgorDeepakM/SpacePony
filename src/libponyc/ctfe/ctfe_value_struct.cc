#include "ctfe_value_struct.h"

#include <algorithm>

#include "../ast/astbuild.h"
#include "../codegen/genname.h"


using namespace std;

CtfeValueStruct::CtfeValueStruct(ast_t *type):
  m_type{ast_dup(type)}
{

}


CtfeValueStruct::~CtfeValueStruct()
{
  if(m_type != nullptr)
  {
    ast_free_unattached(m_type);
    m_type = nullptr;
  }
}


bool CtfeValueStruct::new_value(const string& name, const CtfeValue& value)
{
  const auto [it, success] = m_vars.insert({name, value});
  return success;
}


bool CtfeValueStruct::update_value(const string& name, const CtfeValue& value)
{
  auto it = m_vars.find(name); 
  if(it != m_vars.end())
  {
    it->second = value;
    return true;
  }
  
  return false;
}


bool CtfeValueStruct::get_value(const string& name, CtfeValue& value) const
{
  auto it = m_vars.find(name); 
  if(it != m_vars.end())
  {
    value = it->second;
    return true;
  }
  
  return false;
}


void CtfeValueStruct::write_to_memory(uint8_t* ptr) const
{
  const CtfeValueStruct* r = this;
  memcpy(ptr, &r, sizeof(CtfeValueStruct*));
}


CtfeValueStruct* CtfeValueStruct::read_from_memory(uint8_t* ptr)
{
  CtfeValueStruct* r = nullptr;
  memcpy(&r, ptr, sizeof(CtfeValueStruct*));
  return r;
}


ast_t* CtfeValueStruct::create_ast_literal_node(pass_opt_t* opt, errorframe_t* errors,
  ast_t* from)
{
  ast_t* underlying_type = (ast_t*)ast_data(m_type);
  ast_t* members = ast_childidx(underlying_type, 4);

  ast_t* obj = ast_blank(TK_CONSTANT_OBJECT);
  ast_t* name_node = ast_blank(TK_ID);

  const char* obj_name = object_hygienic_name(opt, m_type);
  ast_set_name(name_node, obj_name);

  ast_append(obj, name_node);
  ast_t* members_node = ast_blank(TK_MEMBERS);
  ast_append(obj, members_node);

  ast_t* obj_members = ast_childidx(obj, 1);

  ast_t* member = ast_child(members);
  while(member != NULL)
  {
    switch(ast_id(member))
    {
      case TK_FVAR:
      case TK_FLET:
      case TK_EMBED:
      {
        const char* var_name = ast_name(ast_child(member));

        CtfeValue var;
        if(!get_value(var_name, var))
        {
          pony_assert(false);
        }

        ast_t* var_node = ast_blank(ast_id(member));
        ast_settype(var_node, ast_type(member));
        ast_t* lit_node = var.create_ast_literal_node(opt, errors, from);
        ast_settype(lit_node, ast_type(member));
        ast_append(var_node, lit_node);
        ast_append(obj_members, var_node);

        break;
      }

      default:
        break;
    }

    member = ast_sibling(member);
  }

  ast_settype(obj, ast_dup(m_type));

  return obj;
}
