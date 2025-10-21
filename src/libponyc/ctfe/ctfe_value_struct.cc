#include "ctfe_value_struct.h"

#include <algorithm>


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
