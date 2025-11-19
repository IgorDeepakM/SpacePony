#pragma once

#include "ctfe_types.h"

#include "../ast/ast.h"
#include "../pass/pass.h"
#include "../type/subtype.h"

#include <string>


class CtfeAstType
{
  static bool m_static_initialized;
  static uint8_t m_long_size;
  static uint8_t m_size_size;

public:
  static const std::string get_type_name(ast_t *ast);
  static size_t get_size_of_type(ast_t* type);
  static bool is_pointer(ast_t* type) { return ::is_pointer(type); }
  static bool is_array(ast_t* type) { return ::is_literal(type, "Array"); }
  static bool is_typed_int(ast_t* type) { return ::is_integer(type); }
  static bool is_typed_float(ast_t* type) { return ::is_float(type); }
  static bool is_int_literal(ast_t* type) { return ast_id(type) == TK_LITERAL; }
  static bool is_bool(ast_t* type) { return ::is_bool(type); }
  static bool is_none(ast_t* type) { return ::is_none(type); }
  static bool is_struct(ast_t* ast);
  static bool is_interface(ast_t* ast);
  static bool is_tuple(ast_t* ast) { return ast_id(ast) == TK_TUPLETYPE; };
  static bool is_nominal(ast_t* ast) { return ast_id(ast) == TK_NOMINAL; };
  static bool is_machine_word(ast_t* ast) { return ::is_machine_word(ast); }
  static bool is_signed(ast_t* ast) { return ::is_signed(ast); }
  static bool is_primitive(ast_t* ast);

  static void initialize(pass_opt_t* opt);
  static uint8_t get_long_size() { return m_long_size; }
  static uint8_t get_size_size() { return m_size_size; }

  static uint64_t ast_hash(ast_t* ast);
};
