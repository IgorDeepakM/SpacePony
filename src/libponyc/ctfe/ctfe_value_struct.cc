#include "ctfe_value_struct.h"


using namespace std;

CtfeValueStruct::CtfeValueStruct(const CtfeValueTypeRef& typeref):
  m_type_ref(typeref)
{

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
