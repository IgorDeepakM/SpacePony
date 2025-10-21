#include "ctfe_value_tuple.h"
#include "ctfe_value.h"

#include <algorithm>

#include "ponyassert.h"


using namespace std;

CtfeValueTuple::CtfeValueTuple():
  m_vars{nullptr},
  m_size{0}
{

}


CtfeValueTuple::CtfeValueTuple(size_t size):
  m_vars{nullptr},
  m_size{0}
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


CtfeValueTuple::CtfeValueTuple(const CtfeValueTuple& val):
  m_vars{nullptr},
  m_size{0}
{
  m_vars = new CtfeValue[val.m_size];
  m_size = val.m_size;

  copy (val.m_vars, val.m_vars + m_size, m_vars);
}


CtfeValueTuple::CtfeValueTuple(CtfeValueTuple&& val) noexcept:
  m_vars{nullptr},
  m_size{0}
{
  swap(*this, val);
}


CtfeValueTuple::~CtfeValueTuple()
{
  if(m_vars != nullptr)
  {
    delete [] m_vars;
    m_vars = nullptr;
  }
}


void swap(CtfeValueTuple& a, CtfeValueTuple& b)
{
  swap(a.m_vars, b.m_vars);
  swap(a.m_size, b.m_size);
}


CtfeValueTuple& CtfeValueTuple::operator=(CtfeValueTuple val)
{
  swap(*this, val);

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


bool CtfeValueTuple::is_subtype(ast_t* pattern_type, pass_opt_t* opt)
{
  bool match = true;

  ast_t* b_type_elem = ast_child(pattern_type);

  for(size_t i = 0; i < m_size; i++)
  {
    pony_assert(b_type_elem != nullptr);
    ast_t* a_type_elem = m_vars[i].get_type_ast();

    if(!is_subtype_ignore_cap(a_type_elem, b_type_elem, nullptr, opt))
    {
      match = false;
      break;
    }

    b_type_elem = ast_sibling(b_type_elem);
  }

  return match;
}