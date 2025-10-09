#include "ctfe_value_bool.h"
#include "ctfe_value.h"

#include "../type/assemble.h"


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


ast_t* CtfeValueBool::create_ast_literal_node(pass_opt_t* opt, ast_t* from)
{
  ast_t* new_node = ast_blank(m_val ? TK_TRUE : TK_FALSE);
  ast_t* type = type_builtin(opt, from, "Bool");
  ast_settype(new_node, type);
  return new_node;
}


bool CtfeValueBool::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  const std::vector<CtfeValue>& args, const std::string& method_name, CtfeValue& result)
{
  if(args.size() == 2)
  {
    if(method_name == "op_and")
    {
      CtfeValueBool r = args[0].get_bool().op_and(args[1].get_bool());
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "op_or")
    {
      CtfeValueBool r = args[0].get_bool().op_or(args[1].get_bool());
      result = CtfeValue(r);
      return true;
    }
    else if(method_name == "op_xor")
    {
      CtfeValueBool r = args[0].get_bool().op_xor(args[1].get_bool());
      result = CtfeValue(r);
      return true;
    }
  }
  else if(args.size() == 1)
  {
    if(method_name == "op_not")
    {
      CtfeValueBool r = args[0].get_bool().op_not();
      result = CtfeValue(r);
      return true;
    }
  }

  return false;
}