#pragma once

#include "ctfe_types.h"

#include "../ast/ast.h"
#include "../pass/pass.h"

#include <string>


class CtfeValueTypeRef
{
  static bool m_static_initialized;
  static uint8_t m_long_size;
  static uint8_t m_size_size;

  CtfeValueType m_type;
  ast_t* m_type_ast;

public:
  CtfeValueTypeRef(ast_t* type);

  CtfeValueType get_type() const { return m_type; }
  size_t get_size_of_type() const;
  const std::string get_type_name() const;

  static void initialize(pass_opt_t* opt);
  static uint8_t get_long_size() { return m_long_size; }
  static uint8_t get_size_size() { return m_size_size; }
  static size_t get_size_of_type(CtfeValueType type);
  static std::string get_pony_type_name(CtfeValueType type);
};
