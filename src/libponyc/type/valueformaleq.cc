#include "valueformaleq.h"
#include "../ast/frame.h"
#include "../pass/pass.h"
#include "../type/subtype.h"
#include "../ctfe/ctfe_reach.h"

#include "ponyassert.h"

#include <map>
#include <vector>
#include <algorithm>

using namespace std;

typedef struct
{
  ast_t* a;
  ast_t* b;
}TypePair;


class ValueFormalArgEqList
{
  size_t current_id;
  vector<TypePair> m_original_pair;

public:
  ValueFormalArgEqList() : current_id{ 1 }
  {
  }

  ~ValueFormalArgEqList()
  {
    for(TypePair pair : m_original_pair)
    {
      ast_free_unattached(pair.a);
      ast_free_unattached(pair.b);
    }
  }

  size_t get_comptime_id()
  {
    return current_id++;
  }

  void add_type_check_pair(ast_t* a, ast_t* b)
  {
    TypePair pair = { ast_dup(a), ast_dup(b) };
    m_original_pair.push_back(pair);
  }


  bool check_value_formal_eq_list(pass_opt_t* opt, ast_t* node, size_t comptime_id,
    deferred_reification_t* dr)
  {
    for(TypePair pair : m_original_pair)
    {
      size_t id_a = reinterpret_cast<size_t>(ast_data(pair.a));
      size_t id_b = reinterpret_cast<size_t>(ast_data(pair.b));
      if(id_a == comptime_id || id_b == comptime_id)
      {
        TypePair new_pair;
        if(id_a == comptime_id)
        {
          new_pair.a = ast_dup(node);
        }
        else
        {
          new_pair.a = ast_dup(pair.a);
        }

        if(id_b == comptime_id)
        {
          new_pair.b = ast_dup(node);
        }
        else
        {
          new_pair.b = ast_dup(pair.b);
        }

        if(ast_id(new_pair.a) == TK_COMPTIME)
        {
          ast_t* r_ast = deferred_reify(dr, new_pair.a, opt);
          ast_replace(&new_pair.a, r_ast);
          reach_comptime(opt, &new_pair.a);
        }

        if(ast_id(new_pair.b) == TK_COMPTIME)
        {
          ast_t* r_ast = deferred_reify(dr, new_pair.b, opt);
          ast_replace(&new_pair.b, r_ast);
          reach_comptime(opt, &new_pair.b);
        }

        bool success = is_literal_equal(new_pair.a, new_pair.b, opt, false);

        ast_free_unattached(new_pair.a);
        ast_free_unattached(new_pair.b);
        
        if(!success)
        {
          errorframe_t errors = NULL;
          errorframe_t frame = NULL;
          ast_error_frame(&frame, pair.a, "Calculated value formal argument differs from");
          ast_error_frame(&frame, pair.b, "this argument value");
          errorframe_append(&frame, &errors);
          errorframe_report(&frame, opt->check.errors);
          opt->check.evaluation_error = true;
          return false;
        }
      }
    }

    return true;
  }
};


static ValueFormalArgEqList& get_value_formal_arg_eq_list(pass_opt_t* opt)
{
  typecheck_t* check = &opt->check;

  if(check->value_formal_eq_list_data == nullptr)
  {
    check->value_formal_eq_list_data = new ValueFormalArgEqList;
  }

  return *reinterpret_cast<ValueFormalArgEqList*>(check->value_formal_eq_list_data);
}


void delete_value_formal_arg_eq(pass_opt_t* opt)
{
  typecheck_t* check = &opt->check;

  if(check->value_formal_eq_list_data != nullptr)
  {
    delete reinterpret_cast<ValueFormalArgEqList*>(check->value_formal_eq_list_data);
    check->value_formal_eq_list_data = nullptr;
  }
}


size_t get_comptime_id(pass_opt_t* opt)
{
  ValueFormalArgEqList& d = get_value_formal_arg_eq_list(opt);

  return d.get_comptime_id();
}


void add_value_formal_arg_expr(pass_opt_t* opt, ast_t* a, ast_t* b)
{
  ValueFormalArgEqList& d = get_value_formal_arg_eq_list(opt);

  d.add_type_check_pair(a, b);
}


bool check_value_formal_eq_list(pass_opt_t* opt, ast_t* node, size_t comptime_id,
  deferred_reification_t* dr)
{
  ValueFormalArgEqList& d = get_value_formal_arg_eq_list(opt);

  return d.check_value_formal_eq_list(opt, node, comptime_id, dr);
}
