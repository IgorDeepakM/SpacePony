#include "ctfe_value_pointer.h"
#include "ctfe_value.h"
#include "ctfe_runner.h"
#include "ctfe_exception.h"
#include "ctfe_types.h"
#include "ctfe_ast_type.h"

#include <algorithm>

#include "../ast/ast.h"


using namespace std;


CtfeValuePointer::CtfeValuePointer(ast_t* pointer_type):
  m_array{nullptr},
  m_size{0},
  m_elem_size{0},
  m_pointer_type{ast_dup(pointer_type)}
{
  m_elem_size = CtfeAstType::get_size_of_type(ast_child(ast_childidx(pointer_type, 2)));
}


CtfeValuePointer::CtfeValuePointer(size_t size, ast_t* pointer_type, CtfeRunner &ctfeRunner):
  m_array{nullptr},
  m_size{size},
  m_elem_size{0},
  m_pointer_type{ast_dup(pointer_type)}
{
  m_elem_size = CtfeAstType::get_size_of_type(ast_child(ast_childidx(pointer_type, 2)));

  if(size > 0)
  {
    m_array = new uint8_t[size * m_elem_size];
    m_size = size;

    fill(m_array, m_array + size * m_elem_size, 0);

    ctfeRunner.add_allocated_reference(CtfeValue(*this, pointer_type));
  }
}


CtfeValuePointer::CtfeValuePointer(uint8_t *array, size_t size, ast_t* pointer_type):
  m_array{array},
  m_size{size},
  m_elem_size{0},
  m_pointer_type{ast_dup(pointer_type)}
{
  m_elem_size = CtfeAstType::get_size_of_type(ast_child(ast_childidx(pointer_type, 2)));
}


CtfeValuePointer::CtfeValuePointer(void *ptr, ast_t* pointer_type):
  m_array{reinterpret_cast<uint8_t*>(ptr)},
  m_size{0},
  m_elem_size{0},
  m_pointer_type{ast_dup(pointer_type)}
{
  m_elem_size = CtfeAstType::get_size_of_type(ast_child(ast_childidx(pointer_type, 2)));
}


CtfeValuePointer::~CtfeValuePointer()
{
  if(m_pointer_type != nullptr)
  {
    ast_free_unattached(m_pointer_type);
    m_pointer_type = nullptr;
  }
}


void swap(CtfeValuePointer& a, CtfeValuePointer& b)
{
  swap(a.m_array, b.m_array);
  swap(a.m_size, b.m_size);
  swap(a.m_elem_size, b.m_elem_size);
  swap(a.m_pointer_type, b.m_pointer_type);
}


CtfeValuePointer::CtfeValuePointer(const CtfeValuePointer &other):
  m_array{other.m_array},
  m_size{other.m_size},
  m_elem_size{other.m_elem_size},
  m_pointer_type{ast_dup(other.m_pointer_type)}
{

}


CtfeValuePointer::CtfeValuePointer(CtfeValuePointer&& other) noexcept:
  m_array{nullptr},
  m_size{0},
  m_elem_size{0},
  m_pointer_type{nullptr}
{
  swap(*this, other);
}


CtfeValuePointer& CtfeValuePointer::operator=(CtfeValuePointer other)
{
  swap(*this, other);

  return *this;
}


CtfeValuePointer CtfeValuePointer::realloc(size_t size, CtfeRunner &ctfeRunner)
{
  if(size > 0)
  {
    CtfeValuePointer new_p = CtfeValuePointer(size, m_pointer_type, ctfeRunner);
    fill(m_array, m_array + size * m_elem_size, 0);
    ctfeRunner.add_allocated_reference(CtfeValue(new_p, m_pointer_type));
    
    for(size_t i = 0; (i < (m_size * m_elem_size)) && (i < (new_p.m_size * m_elem_size)); i++)
    {
      new_p.m_array[i] = m_array[i];
    }

    return new_p;
  }
  else
  {
    return CtfeValuePointer(m_pointer_type);
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
    return CtfeValue::read_from_memory(get_pointer_elem_type_ast(), &m_array[i * m_elem_size]);
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
    ret = CtfeValue::read_from_memory(get_pointer_elem_type_ast(), &m_array[i * m_elem_size]);
    val.write_to_memory(&m_array[i * m_elem_size]);
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
    return CtfeValuePointer(&m_array[i * m_elem_size], m_size - (i * m_elem_size), m_pointer_type);
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
    ret = CtfeValue::read_from_memory(get_pointer_elem_type_ast(), &m_array[0]);
  }
  else if(n + len < m_size)
  {
    ret = CtfeValue::read_from_memory(get_pointer_elem_type_ast(), &m_array[0]);
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
  ast_t* res_type, CtfeValue& recv, const vector<CtfeValue>& args, const string& method_name,
  CtfeValue& result, CtfeRunner &ctfeRunner)
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
      result = CtfeValue(CtfeValuePointer(size, res_type, ctfeRunner), res_type);
      return true;
    }
    else if(method_name == "realloc")
    {
      CtfeValuePointer rec_val = recv.get_pointer();
      uint64_t size = args[0].to_uint64();
      CtfeValuePointer ptr = rec_val.realloc(size, ctfeRunner);
      result = CtfeValue(ptr, res_type);
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
      result = CtfeValue(ptr, res_type);
      return true;
    }
  }
  else if(args.size() == 1)
  {
    if(method_name == "_unsafe" || method_name == "convert")
    {
      CtfeValuePointer rec_val = recv.get_pointer();
      result = CtfeValue(rec_val.return_same_pointer(), res_type);
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
