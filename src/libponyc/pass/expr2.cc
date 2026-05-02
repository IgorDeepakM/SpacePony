#include "expr2.h"

#include "ponyassert.h"

#include <vector>

using namespace std;

typedef struct
{
  ast_t* a;
  ast_t* b;
}TypePair;


class Expr2Runner
{
  vector<TypePair> type_pair_check;

  ast_t* add_annotation_string(ast_t* a, const char* str)
  {
    ast_t* annotation = ast_annotation(a);
    if(annotation == nullptr)
    {
      annotation = ast_from(a, TK_ANNOTATION);
      ast_setannotation(a, annotation);
    }

    ast_append(annotation, ast_from_string(annotation, stringtab(str)));

    return annotation;
  }

public:
  void add_type_check_pair(ast_t* a, ast_t* b)
  {
    type_pair_check.push_back({ a, b });

    ast_t* a_annotation = add_annotation_string(a, "expr2_is_subtype");
    ast_t* b_annotation = add_annotation_string(b, "expr2_is_subtype");

    void* index = reinterpret_cast<void*>(type_pair_check.size() - 1);

    ast_setdata(a_annotation, index);
    ast_setdata(b_annotation, index);
  }

  TypePair& get_type_pair(void* index)
  {
    size_t i = reinterpret_cast<size_t>(index);
    return type_pair_check[i];
  }

  void update_check_pair(void* index, ast_t* old, ast_t* current)
  {
    TypePair pair = get_type_pair(index);
    if(pair.a == old)
    {
      pair.a = current;
    }
    else if(pair.b == old)
    {
      pair.b = current;
    }
    else
    {
      pony_assert(false);
    }
  }
};


static Expr2Runner& get_expr2_runner(pass_opt_t* opt)
{
  if(opt->expr2_data == nullptr)
  {
    opt->expr2_data = new Expr2Runner;
  }

  return *reinterpret_cast<Expr2Runner*>(opt->expr2_data);
}

void delete_expr2_data(pass_opt_t* opt)
{
  if(opt->expr2_data != nullptr)
  {
    delete opt->expr2_data;
    opt->expr2_data = nullptr;
  }
}

void add_to_expr2(pass_opt_t* opt, ast_t* a, ast_t* b)
{
  Expr2Runner& d = get_expr2_runner(opt);

  d.add_type_check_pair(a, b);
}

void update_check_pair(pass_opt_t* opt, void* index, ast_t* old, ast_t* current)
{
  Expr2Runner& d = get_expr2_runner(opt);

  d.update_check_pair(index, old, current);
}