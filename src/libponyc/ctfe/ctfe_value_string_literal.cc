#include "ctfe_value_string_literal.h"
#include "ctfe_value.h"

#include "../type/assemble.h"


using namespace std;


CtfeValueStringLiteral::CtfeValueStringLiteral():
  m_str{}
{

}


CtfeValueStringLiteral::CtfeValueStringLiteral(const std::string& str):
  m_str{str}
{

}
  

CtfeValueStringLiteral CtfeValueStringLiteral::add(const CtfeValueStringLiteral& b) const
{
  return CtfeValueStringLiteral(m_str + b.m_str);
}


ast_t* CtfeValueStringLiteral::create_ast_literal_node(pass_opt_t* opt, ast_t* from)
{
  ast_t* new_node = ast_blank(TK_STRING);
  ast_set_name(new_node, m_str.c_str());
  ast_t* type = type_builtin(opt, from, "String");
  ast_settype(new_node, type);
  return new_node;
}


bool CtfeValueStringLiteral::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  CtfeValue& recv, const std::vector<CtfeValue>& args, const std::string& method_name,
  CtfeValue& result)
{
  if(args.size() == 1)
  {
    if(method_name == "add")
    {
      const CtfeValueStringLiteral& rec_val = recv.get_string_literal();
      const CtfeValueStringLiteral& first_arg = args[0].get_string_literal();
      CtfeValueStringLiteral r = rec_val.add(first_arg);
      result = CtfeValue(r);
      return true;
    }
  }

  return false;
}