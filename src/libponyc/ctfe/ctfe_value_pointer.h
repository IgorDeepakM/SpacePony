#pragma once

#include <cstddef>
#include <vector>
#include <string>
#include <memory>
#include <variant>

#include "ctfe_value_type_ref.h"

#include "../pass/pass.h"



class CtfeValue;
class CtfeRunner;

class CtfeValuePointer
{
  uint8_t* m_array;
  size_t m_size;
  size_t m_elem_size;
  CtfeValueTypeRef m_typeref;

public:
  CtfeValuePointer(const CtfeValueTypeRef& typeref);
  CtfeValuePointer(size_t size, const CtfeValueTypeRef& typeref, CtfeRunner &ctfeRunner);
  CtfeValuePointer(uint8_t *array, size_t size, const CtfeValueTypeRef& typeref);
  CtfeValuePointer(void *ptr, const CtfeValueTypeRef& typeref);
  CtfeValuePointer realloc(size_t size, CtfeRunner &ctfeRunner);
  CtfeValuePointer return_same_pointer() const;
  CtfeValue apply(size_t i) const;
  CtfeValue update(size_t i, const CtfeValue& val);
  CtfeValuePointer pointer_at_index(size_t i) const;
  CtfeValuePointer insert(size_t n, size_t len);
  CtfeValue _delete(size_t n, size_t len);
  void copy_to(CtfeValuePointer& that, size_t len);

  void delete_array_pointer();

  uint8_t *get_cpointer() const { return m_array; }

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
    CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result,
    CtfeRunner &ctfeRunner);
};
