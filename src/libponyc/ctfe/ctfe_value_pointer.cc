#include "ctfe_value_pointer.h"
#include "ctfe_value.h"
#include "ctfe_runner.h"
#include "ctfe_exception.h"
#include "ctfe_types.h"
#include "ctfe_ast_type.h"

#include <algorithm>

#include "../ast/ast.h"
#include "../codegen/genname.h"


using namespace std;


map<uint64_t, string> CtfeValuePointer::m_stored_obj_names;


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


CtfeValuePointer CtfeValuePointer::realloc(size_t size, size_t copy_len, CtfeRunner &ctfeRunner)
{
  if(copy_len > m_size)
  {
    throw CtfeValueException();
  }

  if(size > m_size)
  {
    CtfeValuePointer new_p = CtfeValuePointer(size, m_pointer_type, ctfeRunner);
    copy(m_array, m_array + copy_len * m_elem_size, new_p.m_array);
    fill(new_p.m_array + copy_len * m_elem_size, new_p.m_array + size * m_elem_size, 0);

    return new_p;
  }
  else
  {
    return *this;
  }
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
    CtfeValue ret = CtfeValue::read_from_memory(get_pointer_elem_type_ast(), &m_array[i * m_elem_size]);
    val.write_to_memory(&m_array[i * m_elem_size]);
    return ret;
  }
  else
  {
    throw CtfeValueException();
  }
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


CtfeValuePointer CtfeValuePointer::copy_to(CtfeValuePointer& that, size_t len)
{
  if(len > m_size || len > that.m_size)
  {
    throw CtfeValueException();
  }

  copy(m_array, m_array + (len * m_elem_size), that.m_array);

  return *this;
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
    else if(method_name == "_copy_to")
    {
      CtfeValuePointer rec_val = recv.get_pointer();
      CtfeValuePointer to = args[0].get_pointer();
      uint64_t len = args[1].to_uint64();
      CtfeValuePointer ptr = rec_val.copy_to(to, len);
      result = CtfeValue(ptr, res_type);
      return true;
    }
    else if(method_name == "realloc")
    {
      CtfeValuePointer rec_val = recv.get_pointer();
      uint64_t size = args[0].to_uint64();
      uint64_t copy_len = args[1].to_uint64();
      CtfeValuePointer ptr = rec_val.realloc(size, copy_len, ctfeRunner);
      result = CtfeValue(ptr, res_type);
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
  else if(args.size() == 0)
  {
    if(method_name == "_unsafe")
    {
      result = CtfeValue(recv.get_pointer(), res_type);
      return true;
    }
    else if(method_name == "convert")
    {
      CtfeValuePointer from = recv.get_pointer();
      CtfeValuePointer cp = CtfeValuePointer(from.m_array, from.m_size, res_type);
      result = CtfeValue(cp, res_type);
      return true;
    }
    if(method_name == "create")
    {
      result = recv;
      return true;
    }
  }

  return false;
}


ast_t* CtfeValuePointer::create_ast_literal_node(pass_opt_t* opt, errorframe_t* errors,
  ast_t* from, size_t array_size)
{
  if(m_size == 0)
  {
    ast_error_frame(errors, from,
      "Constant object creation from raw pointers is currently not supported, "
      "only when the pointer is allocated as an array.");
  }

  size_t size = m_size;
  if(array_size != 0)
  {
    size = array_size;
  }

  bool homogeneous_array = true;

  CtfeValue first = CtfeValue::read_from_memory(get_pointer_elem_type_ast(), &m_array[0]);
  for(size_t i = 1; i < size; i++)
  {
    CtfeValue e = CtfeValue::read_from_memory(get_pointer_elem_type_ast(), &m_array[i * m_elem_size]);
    if(first != e)
    {
      homogeneous_array = false;
      break;
    }
  }

  ast_t* obj = ast_blank(TK_CONSTANT_ARRAY);
  ast_t* name_node = ast_blank(TK_ID);
  ast_append(obj, name_node);

  ast_t* homogeneous_node = ast_blank(homogeneous_array ? TK_TRUE : TK_FALSE);
  ast_append(obj, homogeneous_node);

  ast_t* repeat_node = ast_blank(TK_INT);
  lexint_t n = lexint_zero();
  n.low = size;
  ast_set_int(repeat_node, &n);
  ast_append(obj, repeat_node);

  ast_t* elem_size_node = ast_blank(TK_INT);
  lexint_t e = lexint_zero();
  e.low = m_elem_size;
  ast_set_int(elem_size_node, &e);
  ast_append(obj, elem_size_node);

  ast_t* members_node = ast_blank(TK_MEMBERS);
  ast_append(obj, members_node);

  if(homogeneous_array)
  {
    ast_t* member_node = first.create_ast_literal_node(opt, errors, from);
    ast_append(members_node, member_node);
  }
  else
  {
    for(size_t i = 0; i < size; i++)
    {
      CtfeValue e = CtfeValue::read_from_memory(get_pointer_elem_type_ast(), &m_array[i * m_elem_size]);
      ast_t* member_node = e.create_ast_literal_node(opt, errors, from);
      ast_append(members_node, member_node);
    }
  }

  ast_settype(obj, ast_dup(m_pointer_type));

  uint64_t ast_hash = CtfeAstType::ast_hash(obj);
  const char* obj_name = NULL;

  auto it = m_stored_obj_names.find(ast_hash);
  if(it != m_stored_obj_names.end())
  {
    obj_name = it->second.c_str();
  }
  else
  {
    obj_name = object_hygienic_name(opt, m_pointer_type);
    m_stored_obj_names.insert({ast_hash, string(obj_name)});
  }

  ast_set_name(name_node, obj_name);

  return obj;
}
