#pragma once

#include <cstddef>
#include <vector>
#include <string>
#include <memory>
#include <variant>

#include "../pass/pass.h"



class CtfeValue;
class CtfeRunner;

class CtfeValuePointer
{
  uint8_t* m_array;
  size_t m_size;
  size_t m_elem_size;
  ast_t *m_pointer_type;

  friend void swap(CtfeValuePointer& a, CtfeValuePointer& b);

public:
  CtfeValuePointer(ast_t* pointer_type);
  CtfeValuePointer(size_t size, ast_t* pointer_typ, CtfeRunner &ctfeRunner);
  CtfeValuePointer(uint8_t *array, size_t size, ast_t* pointer_type);
  CtfeValuePointer(void *ptr, ast_t* pointer_type);
  CtfeValuePointer(const CtfeValuePointer& other);
  CtfeValuePointer(CtfeValuePointer&& other) noexcept;
  ~CtfeValuePointer();
  CtfeValuePointer& operator=(CtfeValuePointer other);
  CtfeValuePointer realloc(size_t size, size_t copy_len, CtfeRunner &ctfeRunner);
  CtfeValue apply(size_t i) const;
  CtfeValue update(size_t i, const CtfeValue& val);
  CtfeValuePointer pointer_at_index(size_t i) const;
  CtfeValuePointer insert(size_t n, size_t len);
  CtfeValue _delete(size_t n, size_t len);
  CtfeValuePointer copy_to(CtfeValuePointer& that, size_t len);

  void delete_array_pointer();

  uint8_t *get_cpointer() const { return m_array; }
  ast_t* get_pointer_type_ast() const { return m_pointer_type; }
  ast_t* get_pointer_elem_type_ast() const { return ast_child(ast_childidx(m_pointer_type, 2)); }

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
    CtfeValue& result, CtfeRunner &ctfeRunner);
};
