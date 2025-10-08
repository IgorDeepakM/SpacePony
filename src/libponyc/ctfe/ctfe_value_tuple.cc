#include "ctfe_value_tuple.h"
#include "ctfe_value.h"

#include <algorithm>


using namespace std;

CtfeValueTuple::CtfeValueTuple(): m_vars{nullptr}, m_size{0}
{

}


CtfeValueTuple::CtfeValueTuple(size_t size)
{
  if(size > 0)
  {
    m_vars = new CtfeValue[size];
  }
  else
  {
    m_vars = nullptr;
  }
  m_size = size;
}


CtfeValueTuple::CtfeValueTuple(const CtfeValueTuple& val)
{
  m_vars = new CtfeValue[val.m_size];
  m_size = val.m_size;

  for(size_t i = 0; i < val.m_size; i++)
  {
    m_vars[i] = val.m_vars[i];
  }
}


CtfeValueTuple::CtfeValueTuple(CtfeValueTuple&& val)
{
  m_vars = val.m_vars;
  m_size = val.m_size;

  val.m_vars = nullptr;
  val.m_size = 0;
}


CtfeValueTuple::~CtfeValueTuple()
{
  if(m_vars != nullptr)
  {
    delete [] m_vars;
    m_vars = nullptr;
  }
}


CtfeValueTuple& CtfeValueTuple::operator=(const CtfeValueTuple& val)
{
  m_vars = new CtfeValue[val.m_size];
  m_size = val.m_size;

  for(size_t i = 0; i < val.m_size; i++)
  {
    m_vars[i] = val.m_vars[i];
  }

  return *this;
}


bool CtfeValueTuple::update_value(size_t pos, const CtfeValue& value)
{
  if(pos < m_size)
  {
    m_vars[pos] = value;
    return true;
  }

  return false;
}


bool CtfeValueTuple::get_value(size_t pos, CtfeValue& value) const
{
  if(pos < m_size)
  {
    value = m_vars[pos];
    return true;
  }

  return false;
}