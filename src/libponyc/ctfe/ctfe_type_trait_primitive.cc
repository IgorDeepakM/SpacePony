#include "ctfe_type_trait_primitive.h"
#include "ctfe_value.h"
#include "ctfe_exception.h"

#include "ponyassert.h"
#include "../type/subtype.h"


using namespace std;
using namespace std::placeholders;

const map<string, CtfeTypeTraitPrimitive::TypeTraitPrimitiveCallbacks> CtfeTypeTraitPrimitive::m_functions {
  { "is_struct", { bind(&CtfeTypeTraitPrimitive::one_type_arg, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                OneTypeArg([](ast_t* type) { return is_entity(type, TK_STRUCT); }) } },
  { "is_class", { bind(&CtfeTypeTraitPrimitive::one_type_arg, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                OneTypeArg([](ast_t* type) { return is_entity(type, TK_CLASS); }) } },
  { "is_primitive", { bind(&CtfeTypeTraitPrimitive::one_type_arg, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                OneTypeArg([](ast_t* type) { return is_entity(type, TK_PRIMITIVE); }) } },
  { "is_subtype", { bind(&CtfeTypeTraitPrimitive::two_type_args, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                TwoTypeArgs([](ast_t* sub, ast_t* super, pass_opt_t* opt)
                  { return is_subtype(sub, super, NULL, opt); }) } },
  { "is_subtype_constraint", { bind(&CtfeTypeTraitPrimitive::two_type_args, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                TwoTypeArgs([](ast_t* sub, ast_t* super, pass_opt_t* opt)
                  { return is_subtype_constraint(sub, super, NULL, opt); }) } },
  { "is_subtype_ignore_cap", { bind(&CtfeTypeTraitPrimitive::two_type_args, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                TwoTypeArgs([](ast_t* sub, ast_t* super, pass_opt_t* opt)
                  { return is_subtype_ignore_cap(sub, super, NULL, opt); }) } },
  { "is_eqtype", { bind(&CtfeTypeTraitPrimitive::two_type_args, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                TwoTypeArgs([](ast_t* sub, ast_t* super, pass_opt_t* opt)
                  { return is_eqtype(sub, super, NULL, opt); }) } },
  { "is_bool", { bind(&CtfeTypeTraitPrimitive::one_type_arg, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                OneTypeArg([](ast_t* type) { return is_bool(type); }) } },
  { "is_float", { bind(&CtfeTypeTraitPrimitive::one_type_arg, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                OneTypeArg([](ast_t* type) { return is_float(type); }) } },
  { "is_integer", { bind(&CtfeTypeTraitPrimitive::one_type_arg, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                OneTypeArg([](ast_t* type) { return is_integer(type); }) } },
  { "is_machine_word", { bind(&CtfeTypeTraitPrimitive::one_type_arg, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                OneTypeArg([](ast_t* type) { return is_machine_word(type); }) } },
  { "is_signed", { bind(&CtfeTypeTraitPrimitive::one_type_arg, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10),
                OneTypeArg([](ast_t* type) { return ::is_signed(type); }) } },
};

bool CtfeTypeTraitPrimitive::run_method(pass_opt_t* opt, errorframe_t* errors, ast_t* ast, ast_t* res_type,
  CtfeValue& recv, const std::vector<CtfeValue>& args, ast_t* typeargs, const std::string& method_name,
  CtfeValue& result)
{
  auto it = m_functions.find(method_name);
  if(it != m_functions.end())
  {
    return it->second.unpack_function(opt, errors, ast, res_type, recv, args, typeargs,
      method_name, result, it->second.operation_function);
  }

  return false;
}


bool CtfeTypeTraitPrimitive::one_type_arg(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  ast_t* res_type, CtfeValue& recv, const std::vector<CtfeValue>& args, ast_t* typeargs,
  const std::string& method_name, CtfeValue& result, const OperationFunction& op)
{
  ast_t* first_type_arg = ast_child(typeargs);
  if(ast_id(first_type_arg) == TK_TYPEPARAMREF)
  {
    ast_error_frame(errors, ast,
      "CTFE: unable to evaluate 'TypeTrait.%s' because the type refers to an unreified typeparamref",
      method_name.c_str());
    throw CtfeFailToEvaluateException();
  }

  function f = get<OneTypeArg>(op);
  bool r = f(first_type_arg);
  result = CtfeValue(CtfeValueBool(r), res_type);
  return true;
}


bool CtfeTypeTraitPrimitive::two_type_args(pass_opt_t* opt, errorframe_t* errors, ast_t* ast,
  ast_t* res_type, CtfeValue& recv, const std::vector<CtfeValue>& args, ast_t* typeargs,
  const std::string& method_name, CtfeValue& result, const OperationFunction& op)
{
  ast_t* first_type_arg = ast_child(typeargs);
  ast_t* second_type_arg = ast_childidx(typeargs, 1);
  if(ast_id(first_type_arg) == TK_TYPEPARAMREF || ast_id(second_type_arg) == TK_TYPEPARAMREF)
  {
    ast_error_frame(errors, ast,
      "CTFE: unable to evaluate 'TypeTrait.%s' because a type argument refers to an unreified typeparamref",
      method_name.c_str());
    throw CtfeFailToEvaluateException();
  }

  function f = get<TwoTypeArgs>(op);
  bool r = f(first_type_arg, second_type_arg, opt);
  result = CtfeValue(CtfeValueBool(r), res_type);
  return true;
}