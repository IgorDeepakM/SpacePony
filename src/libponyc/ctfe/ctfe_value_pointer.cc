#include "ctfe_value_pointer.h"
#include "ctfe_value.h"
#include "ctfe_runner.h"
#include "ctfe_exception.h"

#include <algorithm>


using namespace std;


CtfeValuePointer::CtfeValuePointer(const CtfeValueTypeRef& typeref):
  m_array{nullptr},
  m_size{0},
  m_elem_size{0},
  m_typeref{typeref}
{
  m_elem_size = typeref.get_size_of_type();
}


CtfeValuePointer::CtfeValuePointer(size_t size, const CtfeValueTypeRef& typeref, CtfeRunner &ctfeRunner):
  m_array{nullptr},
  m_size{size},
  m_elem_size{0},
  m_typeref{typeref}
{
  m_elem_size = typeref.get_size_of_type();

  if(size > 0)
  {
    m_array = new uint8_t[size * m_elem_size];
    m_size = size;

    fill(m_array, m_array + size * m_elem_size, 0);

    ctfeRunner.add_allocated_reference(CtfeValue(*this));
  }
}


CtfeValuePointer::CtfeValuePointer(uint8_t *array, size_t size, const CtfeValueTypeRef& typeref):
  m_array{array},
  m_size{size},
  m_elem_size{0},
  m_typeref{typeref}
{
  m_elem_size = typeref.get_size_of_type();
}


CtfeValuePointer CtfeValuePointer::realloc(size_t size, CtfeRunner &ctfeRunner)
{
  if(size > 0)
  {
    CtfeValuePointer new_p = CtfeValuePointer(size, m_typeref, ctfeRunner);
    fill(m_array, m_array + size * m_elem_size, 0);
    ctfeRunner.add_allocated_reference(CtfeValue(new_p));
    
    for(size_t i = 0; (i < (m_size * m_elem_size)) && (i < (new_p.m_size * m_elem_size)); i++)
    {
      new_p.m_array[i] = m_array[i];
    }

    return new_p;
  }
  else
  {
    return CtfeValuePointer(m_typeref);
  }
}


CtfeValuePointer CtfeValuePointer::return_same_pointer() const
{
  return *this;
}


CtfeValue CtfeValuePointer::apply(size_t i) const
{
  if(i < m_size)
  {
    return CtfeValue::read_from_memory(m_typeref.get_type(), &m_array[i]);
  }
  else
  {
    throw CtfeValueException();
  }
}


CtfeValue CtfeValuePointer::update(size_t i, const CtfeValue& val)
{
  CtfeValue ret = CtfeValue();

  if(i < m_size)
  {
    ret = CtfeValue::read_from_memory(m_typeref.get_type(), &m_array[i]);
    val.write_to_memory(&m_array[i]);
  }
  else
  {
    throw CtfeValueException();
  }

  return ret;
}


CtfeValuePointer CtfeValuePointer::pointer_at_index(size_t i) const
{
  if(i < m_size)
  {
    return CtfeValuePointer(&m_array[i * m_elem_size], m_size - (i * m_elem_size), m_typeref);
  }
  else
  {
    throw CtfeValueException();
  }
}


CtfeValuePointer CtfeValuePointer::insert(size_t n, size_t len)
{
  if(n == 0)
  {
    return *this;
  }

  if(n + len < m_size)
  {
    move_backward(m_array, m_array + len * m_elem_size, m_array + ((n + len) * m_elem_size));

    return *this;
  }
  else
  {
    throw CtfeValueException();
  }
}


CtfeValue CtfeValuePointer::_delete(size_t n, size_t len)
{
  CtfeValue ret;

  if(n == 0)
  {
    ret = CtfeValue::read_from_memory(m_typeref.get_type(), &m_array[0]);
  }
  else if(n + len < m_size)
  {
    ret = CtfeValue::read_from_memory(m_typeref.get_type(), &m_array[0]);
    move(m_array + n * m_elem_size, m_array + (n + len) * m_elem_size, m_array);

    return ret;
  }
  else
  {
    throw CtfeValueException();
  }

  return ret;
}


void CtfeValuePointer::copy_to(CtfeValuePointer& that, size_t len)
{
  copy(m_array, m_array + (len * m_elem_size), that.m_array);
}


void CtfeValuePointer::delete_array_pointer()
{
  if(m_array != nullptr)
  {
    delete [] m_array;
    m_array = nullptr;
  }
}


bool CtfeValuePointer::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  CtfeValue& recv, const vector<CtfeValue>& args, const string& method_name, CtfeValue& result,
  CtfeRunner &ctfeRunner)
{
  if(args.size() == 2)
  {
    if(method_name == "update")
    {
      CtfeValuePointer rec_val = recv.get_pointer();
      uint64_t pos = args[0].to_uint64();
      result = rec_val.update(pos, args[1]);
      return true;
    }
  }
  else if(args.size() == 1)
  {
    if(method_name == "alloc")
    {
      CtfeValuePointer rec_val = recv.get_pointer();
      uint64_t size = args[0].to_uint64();
      result = CtfeValue(CtfeValuePointer(size, rec_val.m_typeref, ctfeRunner));
      return true;
    }
    else if(method_name == "realloc")
    {
      CtfeValuePointer rec_val = recv.get_pointer();
      uint64_t size = args[0].to_uint64();
      CtfeValuePointer ptr = rec_val.realloc(size, ctfeRunner);
      result = CtfeValue(ptr);
      return true;
    }
    else if(method_name == "apply")
    {
      CtfeValuePointer rec_val = recv.get_pointer();
      uint64_t pos = args[0].to_uint64();
      result = rec_val.apply(pos);
      return true;
    }
    else if(method_name == "pointer_at_index")
    {
      CtfeValuePointer rec_val = recv.get_pointer();
      uint64_t pos = args[0].to_uint64();
      CtfeValuePointer ptr = rec_val.pointer_at_index(pos);
      result = CtfeValue(ptr);
      return true;
    }
  }
  else if(args.size() == 1)
  {
    if(method_name == "_unsafe" || method_name == "convert")
    {
      CtfeValuePointer rec_val = recv.get_pointer();
      result = CtfeValue(rec_val.return_same_pointer());
      return true;
    }
  }
  else if(args.size() == 0)
  {
    if(method_name == "create")
    {
      result = recv;
      return true;
    }
  }

  return false;
}
