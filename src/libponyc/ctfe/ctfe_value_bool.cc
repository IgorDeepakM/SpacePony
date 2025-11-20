#include "ctfe_value_bool.h"
#include "ctfe_value.h"

#include "../type/assemble.h"


using namespace std;
using namespace std::placeholders;

const map<string, CtfeValueBool::BoolCallbacks> CtfeValueBool::m_functions {
  { "op_and", { bind(&CtfeValueBool::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
                BinOpFunction(bind(&CtfeValueBool::op_and, _1, _2)) } },
  { "op_or", { bind(&CtfeValueBool::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
                BinOpFunction(bind(&CtfeValueBool::op_or, _1, _2)) } },
  { "op_xor", { bind(&CtfeValueBool::bin_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
                BinOpFunction(bind(&CtfeValueBool::op_xor, _1, _2)) } },
  { "op_not", { bind(&CtfeValueBool::unary_op, _1, _2, _3, _4, _5, _6, _7, _8, _9),
                UnaryOpFunction(bind(&CtfeValueBool::op_not, _1)) } },
};


CtfeValueBool::CtfeValueBool()
{
  m_val = 0;
}


CtfeValueBool::CtfeValueBool(bool val)
{
  m_val = val;
}
  

CtfeValueBool CtfeValueBool::op_and(const CtfeValueBool& b) const
{
  return CtfeValueBool(m_val && b.m_val);
}


CtfeValueBool CtfeValueBool::op_or(const CtfeValueBool& b) const
{
  return CtfeValueBool(m_val || b.m_val);
}


CtfeValueBool CtfeValueBool::op_xor(const CtfeValueBool& b) const
{
  return CtfeValueBool(m_val != b.m_val);
}


CtfeValueBool CtfeValueBool::op_not() const
{
  return CtfeValueBool(!m_val);
}


ast_t* CtfeValueBool::create_ast_literal_node()
{
  ast_t* new_node = ast_blank(m_val ? TK_TRUE : TK_FALSE);
  return new_node;
}


bool CtfeValueBool::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result)
{
  auto it = m_functions.find(method_name);
  if(it != m_functions.end())
  {
    return it->second.unpack_function(opt, errors, ast, res_type, recv, args, method_name, result,
      it->second.operation_function);
  }

  return false;
}


bool CtfeValueBool::bin_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
{
  const CtfeValueBool& rec_val = recv.get_bool();
  const CtfeValueBool& first_arg = args[0].get_bool();
  function f = get<BinOpFunction>(op);
  CtfeValueBool r = f(&rec_val, first_arg);
  result = CtfeValue(r, res_type);
  return true;
}


bool CtfeValueBool::unary_op(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result, const OperationFunction& op)
{
  const CtfeValueBool& rec_val = recv.get_bool();
  function f = get<UnaryOpFunction>(op);
  CtfeValueBool r = f(&rec_val);
  result = CtfeValue(r, res_type);
  return true;
}


void CtfeValueBool::write_to_memory(uint8_t* ptr) const
{
  uint8_t b = static_cast<uint8_t>(m_val);
  *ptr = b;
}


CtfeValueBool CtfeValueBool::read_from_memory(uint8_t* ptr)
{
  uint8_t b = *ptr;
  return CtfeValueBool(static_cast<bool>(b));
}