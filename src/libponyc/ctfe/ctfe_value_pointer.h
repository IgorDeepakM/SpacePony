#pragma once

#include <cstddef>
#include <vector>
#include <string>
#include <memory>
#include <map>

#include "../pass/pass.h"

#include "ctfe_value_bool.h"


class CtfeValue;
class CtfeRunner;

class CtfeValuePointer
{
  using CmpOpFunction = std::function<CtfeValueBool(const CtfeValuePointer*, const CtfeValuePointer&)>;

  using OperationFunction = std::variant<
    CmpOpFunction>;

  struct PointerCallbacks
  {
    std::function<bool(pass_opt_t*, errorframe_t*, ast_t*, ast_t*, CtfeValue&,
      const std::vector<CtfeValue>&, const std::string&, CtfeValue&, const OperationFunction&)> unpack_function;
    OperationFunction operation_function;
  };

  static const std::map<std::string, PointerCallbacks> m_functions;

  static std::map<uint64_t, std::string> m_stored_obj_names;

  uint8_t* m_array;
  size_t m_size;
  size_t m_elem_size;
  ast_t *m_pointer_type;

  static bool cmp_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
    CtfeValue& result, const OperationFunction& op);

public:
  CtfeValuePointer(ast_t* pointer_type);
  CtfeValuePointer(size_t size, ast_t* pointer_type, CtfeRunner &ctfeRunner);
  CtfeValuePointer(uint8_t *array, size_t size, ast_t* pointer_type);
  CtfeValuePointer(void *ptr, ast_t* pointer_type);
  CtfeValuePointer realloc(size_t size, size_t copy_len, CtfeRunner &ctfeRunner);
  CtfeValue apply(size_t i) const;
  CtfeValue update(size_t i, const CtfeValue& val);
  CtfeValuePointer pointer_at_index(size_t i) const;
  CtfeValuePointer insert(size_t n, size_t len);
  CtfeValue _delete(size_t n, size_t len);
  CtfeValuePointer copy_to(CtfeValuePointer& that, size_t len) const;

  void delete_array_pointer();

  uint8_t *get_cpointer() const { return m_array; }
  ast_t* get_pointer_type_ast() const { return m_pointer_type; }
  ast_t* get_pointer_elem_type_ast() const { return ast_child(ast_childidx(m_pointer_type, 2)); }

  CtfeValueBool eq(const CtfeValuePointer& b) const { return CtfeValueBool(m_array == b.m_array); }
  CtfeValueBool ne(const CtfeValuePointer& b) const { return CtfeValueBool(m_array != b.m_array); }
  CtfeValueBool lt(const CtfeValuePointer& b) const { return CtfeValueBool(m_array < b.m_array); }
  CtfeValueBool le(const CtfeValuePointer& b) const { return CtfeValueBool(m_array <= b.m_array); }
  CtfeValueBool gt(const CtfeValuePointer& b) const { return CtfeValueBool(m_array > b.m_array); }
  CtfeValueBool ge(const CtfeValuePointer& b) const { return CtfeValueBool(m_array >= b.m_array); }

  static bool run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
    CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
    CtfeValue& result, CtfeRunner &ctfeRunner);

  ast_t* create_ast_literal_node(pass_opt_t* opt, errorframe_t* errors, ast_t* from,
    size_t array_size = 0);
};
