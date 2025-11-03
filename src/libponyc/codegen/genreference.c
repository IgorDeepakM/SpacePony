#include "genreference.h"
#include "genbox.h"
#include "gencall.h"
#include "gendesc.h"
#include "genexpr.h"
#include "genfun.h"
#include "genname.h"
#include "genopt.h"
#include "gentype.h"
#include "genvaluepass.h"
#include "../expr/literal.h"
#include "../pass/expr.h"
#include "../reach/subtype.h"
#include "../type/cap.h"
#include "../type/subtype.h"
#include "../type/viewpoint.h"
#include "../../libponyrt/mem/pool.h"
#include "../../libponyrt/mem/heap.h"
#include "ponyassert.h"
#include <string.h>

struct genned_string_t
{
  const char* string;
  LLVMValueRef global;
};

static size_t genned_string_hash(genned_string_t* s)
{
  return ponyint_hash_ptr(s->string);
}

static bool genned_string_cmp(genned_string_t* a, genned_string_t* b)
{
  return a->string == b->string;
}

static void genned_string_free(genned_string_t* s)
{
  POOL_FREE(genned_string_t, s);
}

DEFINE_HASHMAP(genned_strings, genned_strings_t, genned_string_t,
  genned_string_hash, genned_string_cmp, genned_string_free);

LLVMValueRef gen_this(compile_t* c, ast_t* ast)
{
  (void)ast;
  return LLVMGetParam(codegen_fun(c), 0);
}

LLVMValueRef gen_param(compile_t* c, ast_t* ast)
{
  ast_t* def = (ast_t*)ast_data(ast);
  pony_assert(def != NULL);
  int index = (int)ast_index(def);

  // Since parameter passed by value generation a heap allocated
  // shadow variable, it must be instead loaded.
  if(ast_has_annotation(ast_childidx(def, 1), "byval"))
  {
    return gen_localload(c, ast);
  }
  else
  {
    if(!c->frame->bare_function)
      index++;

    ast_t* fun = ast_parent(ast_parent(def));
    if(ast_id(fun) == TK_FUN)
    {
      // If return declaration is passed by value, then
      // parameters are shifted one step
      // TODO: this can be done better by having a return_by_value
      // in the frame structure like bare_function.
      ast_t* ret_decl = ast_childidx(fun, 4);
      ast_t* reified_ret = deferred_reify(c->frame->reify, ret_decl, c->opt);
      if(ast_has_annotation(reified_ret, "byval"))
      {
        reach_type_t* t = reach_type(c->reach, reified_ret, c->opt);
        if(!is_return_value_lowering_needed(c, t))
        {
          index++;
        }
      }
      ast_free_unattached(reified_ret);
    }

    return LLVMGetParam(codegen_fun(c), index);
  }
}

static LLVMValueRef make_fieldptr(compile_t* c, LLVMValueRef l_value,
  ast_t* l_type, ast_t* right)
{
  pony_assert(ast_id(l_type) == TK_NOMINAL);
  pony_assert(ast_id(right) == TK_ID);

  ast_t* def;
  ast_t* field;
  uint32_t index;
  get_fieldinfo(l_type, right, &def, &field, &index);

  if(ast_id(def) != TK_STRUCT)
    index++;

  if(ast_id(def) == TK_ACTOR)
    index++;

  reach_type_t* l_t = reach_type(c->reach, l_type, c->opt);
  pony_assert(l_t != NULL);
  compile_type_t* l_c_t = (compile_type_t*)l_t->c_type;

  return LLVMBuildStructGEP2(c->builder, l_c_t->structure, l_value, index, "");
}

static LLVMValueRef make_fieldoffset(compile_t* c, ast_t* l_type, ast_t* right)
{
  pony_assert(ast_id(l_type) == TK_NOMINAL);
  pony_assert(ast_id(right) == TK_ID);

  ast_t* def;
  ast_t* field;
  uint32_t index;
  get_fieldinfo(l_type, right, &def, &field, &index);

  if (ast_id(def) != TK_STRUCT)
    index++;

  if (ast_id(def) == TK_ACTOR)
    index++;

  reach_type_t* l_t = reach_type(c->reach, l_type, c->opt);
  pony_assert(l_t != NULL);
  compile_type_t* l_c_t = (compile_type_t*)l_t->c_type;

  LLVMValueRef zero_ptr = LLVMConstNull(c->intptr);
  LLVMValueRef ptr_to_member = LLVMBuildStructGEP2(c->builder, l_c_t->structure, zero_ptr, index, "");
  return LLVMBuildPtrToInt(c->builder, ptr_to_member, c->intptr, "");
}

LLVMValueRef gen_fieldptr(compile_t* c, ast_t* ast)
{
  AST_GET_CHILDREN(ast, left, right);

  LLVMValueRef l_value = gen_expr(c, left);

  if(l_value == NULL)
    return NULL;

  ast_t* l_type = deferred_reify(c->frame->reify, ast_type(left), c->opt);
  LLVMValueRef ret = make_fieldptr(c, l_value, l_type, right);
  ast_free_unattached(l_type);
  return ret;
}

LLVMValueRef gen_fieldoffset(compile_t* c, ast_t* ast)
{
  if(ast_childidx(ast, 1) == NULL)
    return NULL;

  AST_GET_CHILDREN(ast, left, right);

  if(ast == NULL || ast_id(left) == TK_NONE)
    return NULL;

  LLVMValueRef l_value = gen_fieldoffset(c, left);

  ast_t* l_type = deferred_reify(c->frame->reify, ast_type(left), c->opt);
  LLVMValueRef ret = make_fieldoffset(c, l_type, right);
  ast_free_unattached(l_type);

  if(l_value != NULL)
  {
    LLVMValueRef added = LLVMBuildAdd(c->builder, l_value, ret, "");
    return added;
  }
  else
  {
    return ret;
  }
}

LLVMValueRef gen_fieldload(compile_t* c, ast_t* ast)
{
  AST_GET_CHILDREN(ast, left, right);

  LLVMValueRef field = gen_fieldptr(c, ast);

  if(field == NULL)
    return NULL;

  deferred_reification_t* reify = c->frame->reify;

  ast_t* type = deferred_reify(reify, ast_type(right), c->opt);
  reach_type_t* t = reach_type(c->reach, type, c->opt);
  pony_assert(t != NULL);
  ast_free_unattached(type);
  compile_type_t* c_t = (compile_type_t*)t->c_type;

  field = LLVMBuildLoad2(c->builder, c_t->mem_type, field, "");

  return gen_assign_cast(c, c_t->use_type, field, t->ast_cap);
}


LLVMValueRef gen_fieldembed(compile_t* c, ast_t* ast)
{
  LLVMValueRef field = gen_fieldptr(c, ast);

  if(field == NULL)
    return NULL;

  return field;
}

static LLVMValueRef make_tupleelemptr(compile_t* c, LLVMValueRef l_value,
  ast_t* l_type, ast_t* right)
{
  (void)l_type;
  pony_assert(ast_id(l_type) == TK_TUPLETYPE);
  int index = (int)ast_int(right)->low;

  return LLVMBuildExtractValue(c->builder, l_value, index, "");
}

LLVMValueRef gen_tupleelemptr(compile_t* c, ast_t* ast)
{
  AST_GET_CHILDREN(ast, left, right);

  LLVMValueRef l_value = gen_expr(c, left);

  if(l_value == NULL)
    return NULL;

  deferred_reification_t* reify = c->frame->reify;

  ast_t* type = deferred_reify(reify, ast_type(ast), c->opt);
  reach_type_t* t = reach_type(c->reach, type, c->opt);
  pony_assert(t != NULL);
  ast_free_unattached(type);
  compile_type_t* c_t = (compile_type_t*)t->c_type;

  ast_t* l_type = deferred_reify(reify, ast_type(left), c->opt);
  LLVMValueRef value = make_tupleelemptr(c, l_value, l_type, right);
  ast_free_unattached(l_type);
  return gen_assign_cast(c, c_t->use_type, value, t->ast_cap);
}

LLVMValueRef gen_tuple(compile_t* c, ast_t* ast)
{
  ast_t* child = ast_child(ast);

  if(ast_sibling(child) == NULL)
    return gen_expr(c, child);

  deferred_reification_t* reify = c->frame->reify;

  ast_t* type = deferred_reify(reify, ast_type(ast), c->opt);

  // If we contain '_', we have no usable value.
  if(contains_dontcare(type))
  {
    ast_free_unattached(type);
    return GEN_NOTNEEDED;
  }

  reach_type_t* t = reach_type(c->reach, type, c->opt);
  ast_free_unattached(type);
  compile_type_t* c_t = (compile_type_t*)t->c_type;
  int count = LLVMCountStructElementTypes(c_t->primitive);
  size_t buf_size = count * sizeof(LLVMTypeRef);
  LLVMTypeRef* elements = (LLVMTypeRef*)ponyint_pool_alloc_size(buf_size);
  LLVMGetStructElementTypes(c_t->primitive, elements);

  LLVMValueRef tuple = LLVMGetUndef(c_t->primitive);
  int i = 0;

  while(child != NULL)
  {
    LLVMValueRef value = gen_expr(c, child);

    if(value == NULL)
    {
      ponyint_pool_free_size(buf_size, elements);
      return NULL;
    }

    // We'll have an undefined element if one of our source elements is a
    // variable declaration. This is ok, since the tuple value will never be
    // used.
    if(value == GEN_NOVALUE || value == GEN_NOTNEEDED)
    {
      ponyint_pool_free_size(buf_size, elements);
      return value;
    }

    ast_t* child_type = deferred_reify(reify, ast_type(child), c->opt);
    value = gen_assign_cast(c, elements[i], value, child_type);
    ast_free_unattached(child_type);
    tuple = LLVMBuildInsertValue(c->builder, tuple, value, i++, "");
    child = ast_sibling(child);
  }

  ponyint_pool_free_size(buf_size, elements);
  return tuple;
}

LLVMValueRef gen_localdecl(compile_t* c, ast_t* ast)
{
  ast_t* id = ast_child(ast);
  const char* name = ast_name(id);

  // If this local has already been generated, don't create another copy. This
  // can happen when the same ast node is generated more than once, such as
  // the condition block of a while expression.
  LLVMValueRef value = codegen_getlocal(c, name);

  if(value != NULL)
    return GEN_NOVALUE;

  ast_t* type = deferred_reify(c->frame->reify, ast_type(id), c->opt);
  reach_type_t* t = reach_type(c->reach, type, c->opt);
  ast_free_unattached(type);
  compile_type_t* c_t = (compile_type_t*)t->c_type;

  // All alloca should happen in the entry block of a function.
  LLVMBasicBlockRef this_block = LLVMGetInsertBlock(c->builder);
  LLVMBasicBlockRef entry_block = LLVMGetEntryBasicBlock(codegen_fun(c));
  LLVMValueRef inst = LLVMGetFirstInstruction(entry_block);

  if(inst != NULL)
    LLVMPositionBuilderBefore(c->builder, inst);
  else
    LLVMPositionBuilderAtEnd(c->builder, entry_block);

  LLVMValueRef alloc = LLVMBuildAlloca(c->builder, c_t->mem_type, name);

  // Store the alloca to use when we reference this local.
  codegen_setlocal(c, name, alloc);

  LLVMMetadataRef file = codegen_difile(c);
  LLVMMetadataRef scope = codegen_discope(c);

  uint32_t align_bytes = LLVMABIAlignmentOfType(c->target_data, c_t->mem_type);

  LLVMMetadataRef info = LLVMDIBuilderCreateAutoVariable(c->di, scope,
    name, strlen(name), file, (unsigned)ast_line(ast), c_t->di_type,
    true, LLVMDIFlagZero, align_bytes * 8);

  LLVMMetadataRef expr = LLVMDIBuilderCreateExpression(c->di, NULL, 0);

  LLVMDIBuilderInsertDeclare(c->di, alloc, info, expr,
    (unsigned)ast_line(ast), (unsigned)ast_pos(ast), scope,
    LLVMGetInsertBlock(c->builder));

  // Put the builder back where it was.
  LLVMPositionBuilderAtEnd(c->builder, this_block);
  return GEN_NOTNEEDED;
}

LLVMValueRef gen_localptr(compile_t* c, ast_t* ast)
{
  ast_t* id = ast_child(ast);
  const char* name = ast_name(id);

  LLVMValueRef value = codegen_getlocal(c, name);
  pony_assert(value != NULL);

  return value;
}

LLVMValueRef gen_localload(compile_t* c, ast_t* ast)
{
  LLVMValueRef local_ptr = gen_localptr(c, ast);

  if(local_ptr == NULL)
    return NULL;

  ast_t* type = deferred_reify(c->frame->reify, ast_type(ast), c->opt);
  reach_type_t* t = reach_type(c->reach, type, c->opt);
  ast_free_unattached(type);
  compile_type_t* c_t = (compile_type_t*)t->c_type;

  LLVMValueRef value = LLVMBuildLoad2(c->builder,
    LLVMGetAllocatedType(local_ptr), local_ptr, "");
  return gen_assign_cast(c, c_t->use_type, value, t->ast_cap);
}

LLVMValueRef gen_addressof(compile_t* c, ast_t* ast)
{
  ast_t* expr = ast_child(ast);

  switch(ast_id(expr))
  {
    case TK_VARREF:
      return gen_localptr(c, expr);

    case TK_FVARREF:
    case TK_EMBEDREF:
      return gen_fieldptr(c, expr);

    case TK_FUNREF:
    case TK_BEREF:
      return gen_funptr(c, expr);

    case TK_FFIREF:
      return generate_and_get_ffi_decl(c, expr, (ast_t*)ast_data(expr), NULL);

    default: {}
  }

  pony_assert(0);
  return NULL;
}

LLVMValueRef gen_offsetof(compile_t* c, ast_t* ast)
{
  ast_t* expr = ast_child(ast);

  switch (ast_id(expr))
  {
    case TK_FVARREF:
    case TK_EMBEDREF:
    {
      return gen_fieldoffset(c, expr);
    }
    default: {}
  }

  pony_assert(0);
  return NULL;
}

LLVMValueRef gen_sizeof(compile_t* c, ast_t* ast)
{
  ast_t* type = NULL;
  ast_t* child = ast_child(ast);

  switch (ast_id(child))
  {
    default:
    {
      type = ast_type(child);
    }
  }

  ast_t* r_type = deferred_reify(c->frame->reify, type, c->opt);
  reach_type_t* t = reach_type(c->reach, r_type, c->opt);

  compile_type_t* c_t = (compile_type_t*)t->c_type;

  size_t size_of_type = 0;

  if(is_integer(r_type) || is_float(r_type))
  {
    size_of_type = LLVMABISizeOfType(c->target_data, c_t->primitive);
  }
  else
  {
    size_of_type = c_t->abi_size;
  }

  ast_free_unattached(r_type);

  return LLVMConstInt(c->intptr, size_of_type, false);
}

static LLVMValueRef gen_digestof_box(compile_t* c, reach_type_t* type,
  LLVMValueRef value, int boxed_subtype)
{
  pony_assert(LLVMGetTypeKind(LLVMTypeOf(value)) == LLVMPointerTypeKind);

  LLVMBasicBlockRef box_block = NULL;
  LLVMBasicBlockRef nonbox_block = NULL;
  LLVMBasicBlockRef post_block = NULL;

  LLVMValueRef desc = gendesc_fetch(c, value);

  if((boxed_subtype & SUBTYPE_KIND_UNBOXED) != 0)
  {
    box_block = codegen_block(c, "digestof_box");
    nonbox_block = codegen_block(c, "digestof_nonbox");
    post_block = codegen_block(c, "digestof_post");

    // Check if it's a boxed value.
    LLVMValueRef type_id = gendesc_typeid(c, desc);
    LLVMValueRef boxed_mask = LLVMConstInt(c->i32, 1, false);
    LLVMValueRef is_boxed = LLVMBuildAnd(c->builder, type_id, boxed_mask, "");
    LLVMValueRef zero = LLVMConstInt(c->i32, 0, false);
    is_boxed = LLVMBuildICmp(c->builder, LLVMIntEQ, is_boxed, zero, "");
    LLVMBuildCondBr(c->builder, is_boxed, box_block, nonbox_block);
    LLVMPositionBuilderAtEnd(c->builder, box_block);
  }

  // Call the type-specific __digestof function, which will unbox the value.
  reach_method_t* digest_fn = reach_method(type, TK_BOX,
    stringtab("__digestof"), NULL, c->opt);
  pony_assert(digest_fn != NULL);
  LLVMValueRef func = gendesc_vtable(c, desc, digest_fn->vtable_index);
  LLVMTypeRef fn_type = LLVMFunctionType(c->intptr, &c->ptr, 1, false);
  LLVMValueRef box_digest = codegen_call(c, fn_type, func, &value, 1, true);

  if((boxed_subtype & SUBTYPE_KIND_UNBOXED) != 0)
  {
    LLVMBuildBr(c->builder, post_block);

    // Just cast the address.
    LLVMPositionBuilderAtEnd(c->builder, nonbox_block);
    LLVMValueRef nonbox_digest = LLVMBuildPtrToInt(c->builder, value, c->intptr,
      "");
    LLVMBuildBr(c->builder, post_block);

    LLVMPositionBuilderAtEnd(c->builder, post_block);
    LLVMValueRef phi = LLVMBuildPhi(c->builder, c->intptr, "");
    LLVMAddIncoming(phi, &box_digest, &box_block, 1);
    LLVMAddIncoming(phi, &nonbox_digest, &nonbox_block, 1);
    return phi;
  } else {
    return box_digest;
  }
}

static LLVMValueRef gen_digestof_int64(compile_t* c, LLVMValueRef value)
{
  pony_assert(LLVMTypeOf(value) == c->i64);

  if(target_is_ilp32(c->opt->triple))
  {
    LLVMValueRef shift = LLVMConstInt(c->i64, 32, false);
    LLVMValueRef high = LLVMBuildLShr(c->builder, value, shift, "");
    high = LLVMBuildTrunc(c->builder, high, c->i32, "");
    value = LLVMBuildTrunc(c->builder, value, c->i32, "");
    value = LLVMBuildXor(c->builder, value, high, "");
  }

  return value;
}

static LLVMValueRef gen_digestof_value(compile_t* c, ast_t* type,
  LLVMValueRef value)
{
  LLVMTypeRef impl_type = LLVMTypeOf(value);

  switch(LLVMGetTypeKind(impl_type))
  {
    case LLVMFloatTypeKind:
      value = LLVMBuildBitCast(c->builder, value, c->i32, "");
      return LLVMBuildZExt(c->builder, value, c->intptr, "");

    case LLVMDoubleTypeKind:
      value = LLVMBuildBitCast(c->builder, value, c->i64, "");
      return gen_digestof_int64(c, value);

    case LLVMIntegerTypeKind:
    {
      uint32_t width = LLVMGetIntTypeWidth(impl_type);

      if(width < 64)
      {
        return LLVMBuildZExt(c->builder, value, c->intptr, "");
      } else if(width == 64) {
        return gen_digestof_int64(c, value);
      } else if(width == 128) {
        LLVMValueRef shift = LLVMConstInt(c->i128, 64, false);
        LLVMValueRef high = LLVMBuildLShr(c->builder, value, shift, "");
        high = LLVMBuildTrunc(c->builder, high, c->i64, "");
        value = LLVMBuildTrunc(c->builder, value, c->i64, "");
        high = gen_digestof_int64(c, high);
        value = gen_digestof_int64(c, value);
        return LLVMBuildXor(c->builder, value, high, "");
      }
      break;
    }

    case LLVMStructTypeKind:
    {
      uint32_t count = LLVMCountStructElementTypes(impl_type);
      LLVMValueRef result = LLVMConstInt(c->intptr, 0, false);
      ast_t* child = ast_child(type);

      for(uint32_t i = 0; i < count; i++)
      {
        LLVMValueRef elem = LLVMBuildExtractValue(c->builder, value, i, "");
        elem = gen_digestof_value(c, child, elem);
        result = LLVMBuildXor(c->builder, result, elem, "");
        child = ast_sibling(child);
      }

      pony_assert(child == NULL);

      return result;
    }

    case LLVMPointerTypeKind:
      if(!is_known(type))
      {
        reach_type_t* t = reach_type(c->reach, type, c->opt);
        int sub_kind = subtype_kind(t);

        if((sub_kind & SUBTYPE_KIND_BOXED) != 0)
          return gen_digestof_box(c, t, value, sub_kind);
      }

      return LLVMBuildPtrToInt(c->builder, value, c->intptr, "");

    default: {}
  }

  pony_assert(0);
  return NULL;
}

LLVMValueRef gen_digestof(compile_t* c, ast_t* ast)
{
  ast_t* expr = ast_child(ast);
  LLVMValueRef value = gen_expr(c, expr);
  ast_t* type = deferred_reify(c->frame->reify, ast_type(expr), c->opt);
  LLVMValueRef ret = gen_digestof_value(c, type, value);
  ast_free_unattached(type);
  return ret;
}

void gen_digestof_fun(compile_t* c, reach_type_t* t)
{
  pony_assert(t->can_be_boxed);

  reach_method_t* m = reach_method(t, TK_BOX, stringtab("__digestof"), NULL, c->opt);

  if(m == NULL)
    return;

  compile_type_t* c_t = (compile_type_t*)t->c_type;
  compile_method_t* c_m = (compile_method_t*)m->c_method;
  c_m->func_type = LLVMFunctionType(c->intptr, &c_t->structure_ptr, 1, false);
  c_m->func = codegen_addfun(c, m->full_name, c_m->func_type, true);

  codegen_startfun(c, c_m->func, NULL, NULL, NULL, false);
  LLVMValueRef value = LLVMGetParam(codegen_fun(c), 0);

  value = gen_unbox(c, t->ast_cap, value);
  genfun_build_ret(c, gen_digestof_value(c, t->ast_cap, value));

  codegen_finishfun(c);
}

LLVMValueRef gen_int(compile_t* c, ast_t* ast)
{
  ast_t* type = deferred_reify(c->frame->reify, ast_type(ast), c->opt);
  reach_type_t* t = reach_type(c->reach, type, c->opt);
  ast_free_unattached(type);
  compile_type_t* c_t = (compile_type_t*)t->c_type;

  lexint_t* value = ast_int(ast);
  LLVMValueRef vlow = LLVMConstInt(c->i128, value->low, false);
  LLVMValueRef vhigh = LLVMConstInt(c->i128, value->high, false);
  LLVMValueRef shift = LLVMConstInt(c->i128, 64, false);
  vhigh = LLVMConstShl(vhigh, shift);
  vhigh = LLVMConstAdd(vhigh, vlow);

  if(c_t->primitive == c->i128)
    return vhigh;

  if((c_t->primitive == c->f32) || (c_t->primitive == c->f64))
    return LLVMBuildUIToFP(c->builder, vhigh, c_t->primitive, "");

  return LLVMConstTrunc(vhigh, c_t->primitive);
}

LLVMValueRef gen_float(compile_t* c, ast_t* ast)
{
  ast_t* type = deferred_reify(c->frame->reify, ast_type(ast), c->opt);
  reach_type_t* t = reach_type(c->reach, type, c->opt);
  ast_free_unattached(type);
  compile_type_t* c_t = (compile_type_t*)t->c_type;

  return LLVMConstReal(c_t->primitive, ast_float(ast));
}

LLVMValueRef gen_string(compile_t* c, ast_t* ast)
{
  const char* name = ast_name(ast);

  genned_string_t k;
  k.string = name;
  size_t index = HASHMAP_UNKNOWN;
  genned_string_t* string = genned_strings_get(&c->strings, &k, &index);

  if(string != NULL)
    return string->global;

  ast_t* type = ast_type(ast);
  pony_assert(is_literal(type, "String"));
  reach_type_t* t = reach_type(c->reach, type, c->opt);
  compile_type_t* c_t = (compile_type_t*)t->c_type;

  size_t len = ast_name_len(ast);

  LLVMValueRef args[4];
  args[0] = c_t->desc;
  args[1] = LLVMConstInt(c->intptr, len, false);
  args[2] = LLVMConstInt(c->intptr, len + 1, false);
  args[3] = codegen_string(c, name, len);

  LLVMValueRef inst = LLVMConstNamedStruct(c_t->structure, args, 4);
  LLVMValueRef g_inst = LLVMAddGlobal(c->module, c_t->structure, "");
  LLVMSetInitializer(g_inst, inst);
  LLVMSetGlobalConstant(g_inst, true);
  LLVMSetLinkage(g_inst, LLVMPrivateLinkage);
  LLVMSetUnnamedAddr(g_inst, true);

  string = POOL_ALLOC(genned_string_t);
  string->string = name;
  string->global = g_inst;
  genned_strings_putindex(&c->strings, string, index);

  return g_inst;
}

LLVMValueRef gen_valueformalparamref(compile_t* c, ast_t* ast)
{
  ast_t* reified = deferred_reify(c->frame->reify, ast, c->opt);
  LLVMValueRef ret = gen_expr(c, reified);
  ast_free_unattached(reified);

  return ret;
}


static LLVMValueRef gen_copy_constant_array(compile_t* c, ast_t* ast)
{
  const char* obj_name = ast_name(ast_child(ast));
  LLVMValueRef const_array = LLVMGetNamedGlobal(c->module, obj_name);
  pony_assert(const_array != NULL);

  const lexint_t* size_lex = ast_int(ast_childidx(ast, 2));
  const lexint_t* elem_size_lex = ast_int(ast_childidx(ast, 3));

  size_t array_size = size_lex->low * elem_size_lex->low;

  LLVMValueRef heap_allocated = gencall_pony_alloc(c, array_size);
  LLVMValueRef initializer = LLVMGetInitializer(const_array);
  LLVMBuildStore(c->builder, initializer, heap_allocated);

  return heap_allocated;
}


static LLVMValueRef gen_copy_constant_object(compile_t* c, reach_type_t* t, ast_t* ast)
{
  ast_t* members = ast_childidx(ast, 1);

  ast_t* underlying_type = (ast_t*)ast_data(t->ast);

  uint32_t field_count = t->field_count;
  size_t member_index = 0;
  if(ast_id(underlying_type) == TK_CLASS)
  {
    field_count++;
    member_index++;
  }

  compile_type_t* c_t = (compile_type_t*)t->c_type;

  const char* obj_name = ast_name(ast_child(ast));
  LLVMValueRef const_struct = LLVMGetNamedGlobal(c->module, obj_name);
  pony_assert(const_struct != NULL);

  LLVMValueRef heap_allocated = gen_constructor_receiver(c, t, ast);
  LLVMValueRef initializer = LLVMGetInitializer(const_struct);
  LLVMBuildStore(c->builder, initializer, heap_allocated);

  size_t reach_index = 0;

  for(ast_t* member = ast_child(members); member != NULL; member = ast_sibling(member))
  {
    if(member_index < field_count)
    {
      ast_t* member_obj = ast_child(member);
      if(ast_id(member_obj) == TK_CONSTANT_OBJECT && ast_id(member) != TK_EMBED)
      {
        ast_t* member_underlying_type = (ast_t*)ast_data(ast_type(member_obj));
        if(ast_id(member_underlying_type) == TK_STRUCT ||
           ast_id(underlying_type) == TK_CLASS)
        {
          ast_t* member_type = t->fields[reach_index].ast;

          token_id dest_cap = ast_id(ast_childidx(member_type, 3));
          if(dest_cap != TK_VAL && dest_cap != TK_BOX)
          {
            LLVMValueRef allocated = gen_copy_constant_object(c, t->fields[reach_index].type, member_obj);
            LLVMValueRef elem_ptr = LLVMBuildStructGEP2(c->builder, c_t->structure, heap_allocated,
              (unsigned int)member_index, "");
            LLVMBuildStore(c->builder, allocated, elem_ptr);
          }
        }
      }
      else if (ast_id(member_obj) == TK_CONSTANT_ARRAY)
      {
        ast_t* member_type = t->fields[reach_index].ast;

        token_id dest_cap = ast_id(ast_childidx(member_type, 3));
        if(dest_cap != TK_VAL && dest_cap != TK_BOX)
        {
          LLVMValueRef allocated = gen_copy_constant_array(c, member_obj);
          LLVMValueRef elem_ptr = LLVMBuildStructGEP2(c->builder, c_t->structure, heap_allocated,
            (unsigned int)member_index, "");
          LLVMBuildStore(c->builder, allocated, elem_ptr);
        }
      }
    }
    else
    {
      pony_assert(false);
    }

    member_index++;
    reach_index++;
  }

  return heap_allocated;
}

LLVMValueRef gen_constant_object(compile_t* c, ast_t* ast)
{
  ast_t* members = ast_childidx(ast, 1);

  ast_t* r_type = deferred_reify(c->frame->reify, ast_type(ast), c->opt);
  reach_type_t* t = reach_type(c->reach, r_type, c->opt);
  ast_t* underlying_type = (ast_t*)ast_data(r_type);
  ast_free_unattached(r_type);

  const char* obj_name = ast_name(ast_child(ast));
  LLVMValueRef ret = LLVMGetNamedGlobal(c->module, obj_name);
  if(ret == NULL)
  {
    compile_type_t* c_t = (compile_type_t*)t->c_type;
    uint32_t field_count = t->field_count;
    if(ast_id(underlying_type) == TK_CLASS)
    {
      field_count++;
    }

    size_t buf_size = sizeof(LLVMValueRef) * field_count;
    LLVMValueRef* args = (LLVMValueRef*)ponyint_pool_alloc_size(buf_size);

    size_t member_index = 0;
    if(ast_id(underlying_type) == TK_CLASS)
    {
      args[member_index] = c_t->desc;
      member_index++;
    }

    for(ast_t* member = ast_child(members); member != NULL; member = ast_sibling(member))
    {
      if(member_index < field_count)
      {
        args[member_index] = gen_expr(c, ast_child(member));
      }
      else
      {
        pony_assert(false);
      }
      member_index++;
    }

    LLVMValueRef inst = LLVMConstNamedStruct(c_t->structure, args, field_count);
    ponyint_pool_free_size(buf_size, args);

    LLVMValueRef g_inst = LLVMAddGlobal(c->module, c_t->structure, obj_name);
    LLVMSetInitializer(g_inst, inst);
    LLVMSetGlobalConstant(g_inst, true);
    LLVMSetLinkage(g_inst, LLVMPrivateLinkage);

    ret = g_inst;
  }

  ast_t* parent = ast_parent(ast);
  if(ast_id(parent) == TK_EMBED)
  {
    return LLVMGetInitializer(ret);
  }

  // if the parent is TK_FVAR or TK_FLET, then this constant object is a member
  // in another struct.
  if(ast_id(parent) != TK_FVAR && ast_id(parent) != TK_FLET)
  {
    bool is_recovered = false;
    ast_t* dest_type = find_antecedent_type(c->opt, ast, &is_recovered);

    pony_assert(ast_id(dest_type) == TK_NOMINAL &&
      is_subtype_ignore_cap(dest_type, t->ast, NULL, c->opt));

    token_id dest_cap = ast_id(ast_childidx(dest_type, 3));

    // If the destination is not a val or box, then we must copy the struct
    if(dest_cap != TK_VAL && dest_cap != TK_BOX)
    {
      ret = gen_copy_constant_object(c, t, ast);
    }
  }

  return ret;
}


LLVMValueRef gen_constant_array(compile_t* c, ast_t* ast)
{
  ast_t* type = ast_type(ast);
  pony_assert(is_literal(type, "Pointer"));

  const char* obj_name = ast_name(ast_child(ast));
  LLVMValueRef ret = LLVMGetNamedGlobal(c->module, obj_name);
  if(ret == NULL)
  {
    AST_GET_CHILDREN(ast, id, homogeneous_node, size_node, elem_size_node, members_node);

    const lexint_t* size_lex = ast_int(size_node);

    size_t array_size = size_lex->low;
    size_t buf_size = sizeof(LLVMValueRef) * array_size;
    LLVMValueRef *args = (LLVMValueRef*)ponyint_pool_alloc_size(buf_size);

    // Check if the array is homogeneous
    if(ast_id(ast_childidx(ast, 1)) == TK_TRUE)
    {
      ast_t* member = ast_child(members_node);
      LLVMValueRef elem = gen_expr(c, member);

      for(size_t i = 0; i < array_size; i++)
      {
        args[i] = elem;
      }
    }
    else
    {
      ast_t* member = ast_child(members_node);
      for(size_t i = 0; i < array_size; i++)
      {
        args[i] = gen_expr(c, member);

        member = ast_sibling(member);
      }
    }

    ast_t* pointer_type = ast_child(ast_childidx(type, 2));
    ast_t* r_type = deferred_reify(c->frame->reify, pointer_type, c->opt);
    reach_type_t* t_elem = reach_type(c->reach, r_type, c->opt);
    ast_free_unattached(r_type);

    compile_type_t* c_t = (compile_type_t*)t_elem->c_type;

    LLVMValueRef inst = LLVMConstArray2(c_t->use_type, args, array_size);
    ponyint_pool_free_size(buf_size, args);

    LLVMTypeRef array_type = LLVMArrayType2(c_t->use_type, array_size);
    LLVMValueRef g_inst = LLVMAddGlobal(c->module, array_type, obj_name);
    LLVMSetInitializer(g_inst, inst);
    LLVMSetGlobalConstant(g_inst, true);
    LLVMSetLinkage(g_inst, LLVMPrivateLinkage);

    ret = g_inst;
  }

  return ret;
}
