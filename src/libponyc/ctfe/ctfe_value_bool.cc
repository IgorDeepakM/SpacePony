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


ast_t* CtfeValueBool::create_ast_literal_node()
{
  ast_t* new_node = ast_blank(m_val ? TK_TRUE : TK_FALSE);
  return new_node;
}


bool CtfeValueBool::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result)
{
  if(args.size() == 1)
  {
    if(method_name == "op_and")
    {
      const CtfeValueBool& rec_val = recv.get_bool();
      const CtfeValueBool& first_arg = args[0].get_bool();
      CtfeValueBool r = rec_val.op_and(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "op_or")
    {
      const CtfeValueBool& rec_val = recv.get_bool();
      const CtfeValueBool& first_arg = args[0].get_bool();
      CtfeValueBool r = rec_val.op_or(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
    else if(method_name == "op_xor")
    {
      const CtfeValueBool& rec_val = recv.get_bool();
      const CtfeValueBool& first_arg = args[0].get_bool();
      CtfeValueBool r = rec_val.op_xor(first_arg);
      result = CtfeValue(r, res_type);
      return true;
    }
  }
  else if(args.size() == 0)
  {
    if(method_name == "op_not")
    {
      const CtfeValueBool& rec_val = recv.get_bool();
      CtfeValueBool r = rec_val.op_not();
      result = CtfeValue(r, res_type);
      return true;
    }
  }

  return false;
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