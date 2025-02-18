#include "genvaluepass.h"
#include "gentype.h"
#include "ponyassert.h"
#include "genopt.h"
#include "gencall.h"

#define MAX_REG_TYPES 4

typedef struct
{
  size_t current_word;
  size_t current_byte_in_word;
  size_t bytes_per_word;
  LLVMTypeRef types[MAX_REG_TYPES];
}RegisterPos_t;


static bool is_power_of_2(size_t x)
{
    return x > 0 && !(x & (x - 1));
}

bool is_pass_by_value_lowering_supported(compile_t* c)
{
  bool ret = false;

  char* triple = c->opt->triple;

  if(target_is_x86(triple) && target_is_windows(triple) && target_is_llp64(triple))
  {
    if(target_is_llp64(triple) || target_is_ilp32(triple))
    {
      ret = true;
    }
  }
  else if(target_is_arm(triple))
  {
    if(target_is_ilp32(triple))
    {
      ret = true;
    }
  }

  return ret;
}

bool is_param_value_lowering_needed(compile_t* c, reach_type_t* pt)
{
  bool ret = false;
  char* triple = c->opt->triple;

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  if(target_is_x86(triple) && target_is_windows(triple))
  {
    if(target_is_llp64(triple))
    {
      if(p_t->abi_size <= 8 && is_power_of_2(p_t->abi_size))
      {
        ret = true;
      }
    }
    // 32-bit Windows always use byval when value passing, so always false
  }
  else if(target_is_arm(triple))
  {
    if(target_is_ilp32(triple))
    {
      if(p_t->abi_size <= 64)
      {
        ret = true;
      }
    }
  }

  return ret;
}

bool is_return_value_lowering_needed(compile_t* c, reach_type_t* pt)
{
  bool ret = false;
  char* triple = c->opt->triple;

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  if(target_is_x86(triple) && target_is_windows(triple))
  {
    if(target_is_llp64(triple))
    {
      if(p_t->abi_size <= 8 && is_power_of_2(p_t->abi_size))
      {
        ret = true;
      }
    }
    else if(target_is_ilp32(triple))
    {
      if(p_t->abi_size <= 4)
      {
        ret = true;
      }
    }
  }
  else if(target_is_arm(triple))
  {
    if(target_is_ilp32(triple))
    {
      if(p_t->abi_size <= 4)
      {
        ret = true;
      }
    }
  }

  return ret;
}


static LLVMTypeRef get_type_from_size(compile_t* c, size_t size)
{
  switch(size)
  {
    case 8:
      return c->i64;
    case 4:
      return c->i32;
    case 2:
      return c->i16;
    case 1:
      return c->i8;
    default:
      pony_assert(false);
      return NULL;
  }
}

static size_t next_power_of_2(size_t v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  v++;

  return v;
}


static void insert_size(compile_t* c, RegisterPos_t *pos, size_t size)
{
  pos->current_byte_in_word += size;

  if(pos->current_byte_in_word == pos->bytes_per_word)
  {
    pos->types[pos->current_word] = get_type_from_size(c, pos->bytes_per_word);
    pos->current_word++;
    pos->current_byte_in_word = 0;
  }
  else if(pos->current_byte_in_word > pos->bytes_per_word)
  {
    pony_assert(false);
  }
}


static void lower_fixed_sized_array(compile_t* c, LLVMTypeRef array, RegisterPos_t *pos)
{
  LLVMTypeRef element_type = LLVMGetElementType(array);
  size_t element_size = (size_t)LLVMABISizeOfType(c->target_data, element_type);
  size_t num_elements = (size_t)LLVMGetArrayLength(array);

  size_t total_bytes = num_elements * element_size;
  size_t num_whole_words = total_bytes / pos->bytes_per_word;
  size_t nibble = total_bytes % pos->bytes_per_word;

  for(size_t i = 0; i < num_whole_words; i++)
  {
    insert_size(c, pos, pos->bytes_per_word);
  }

  if(nibble != 0)
  {
    insert_size(c, pos, nibble);
  }
}


static void lower_structure(compile_t* c, LLVMTypeRef structure, RegisterPos_t *pos)
{
  size_t num_elements = (size_t)LLVMCountStructElementTypes(structure);

  for(size_t i = 0; i < num_elements; i++)
  {
    LLVMTypeRef curr_type = LLVMStructGetTypeAtIndex(structure, (unsigned int)i);

    switch(LLVMGetTypeKind(curr_type))
    {
      case LLVMStructTypeKind:
        lower_structure(c, curr_type, pos);
        break;
      case LLVMArrayTypeKind:
        lower_fixed_sized_array(c, curr_type, pos);
        break;
      default:
      {
        size_t size = (size_t)LLVMABISizeOfType(c->target_data, curr_type);
        insert_size(c, pos, size);
        break;
      }
    }
  }
}

static LLVMTypeRef lower_and_generate_type(compile_t* c, RegisterPos_t *pos)
{
  char* triple = c->opt->triple;

  if(target_is_x86(triple) && target_is_windows(triple) && 
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    if(pos->current_byte_in_word != 0)
    {
      return get_type_from_size(c, pos->current_byte_in_word);
    }
    else
    {
      return pos->types[0];
    }
  }
  else
  {
    pony_assert(false);
    return NULL;
  }
}

LLVMTypeRef lower_param_value_from_structure_type(compile_t* c, reach_type_t* pt)
{
  LLVMTypeRef ret = NULL;
  char* triple = c->opt->triple;

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  if(target_is_x86(triple) && target_is_windows(triple) && target_is_llp64(triple))
  {
    ret = get_type_from_size(c, p_t->abi_size);
  }
  else if(target_is_arm(triple))
  {
    if(target_is_ilp32(triple))
    {
      size_t align = (size_t)LLVMABIAlignmentOfType(c->target_data, p_t->structure);
      if(align < 4)
      {
        align = 4;
      }
      size_t type_size = p_t->abi_size;
      size_t array_size = type_size / align;
      if((type_size % align) != 0)
      {
        array_size++;
      }
      LLVMTypeRef array_type = get_type_from_size(c, align);
      ret = LLVMArrayType(array_type, (unsigned int)array_size);
    }
  }
  else
  {
    size_t ptr_size = (size_t)LLVMABISizeOfType(c->target_data, c->intptr);

    RegisterPos_t reg_pos;
    reg_pos.current_word = 0;
    reg_pos.current_byte_in_word = 0;
    reg_pos.bytes_per_word = ptr_size;

    LLVMTypeRef structure = p_t->structure;

    lower_structure(c, structure, &reg_pos);

    ret = lower_and_generate_type(c, &reg_pos);
    
     //LLVMStructType(reg_pos.types, reg_pos.current_word, false);
  }

  return ret;
}

LLVMTypeRef lower_return_value_from_structure_type(compile_t* c, reach_type_t* pt)
{
  LLVMTypeRef ret = NULL;
  char* triple = c->opt->triple;

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    ret = get_type_from_size(c, p_t->abi_size);
  }
  else if(target_is_arm(triple))
  {
    if(target_is_ilp32(triple))
    {
      ret = get_type_from_size(c, next_power_of_2(p_t->abi_size));
    }
  }
  else
  {
    size_t ptr_size = (size_t)LLVMABISizeOfType(c->target_data, c->intptr);

    RegisterPos_t reg_pos;
    reg_pos.current_word = 0;
    reg_pos.current_byte_in_word = 0;
    reg_pos.bytes_per_word = ptr_size;

    LLVMTypeRef structure = p_t->structure;

    lower_structure(c, structure, &reg_pos);

    ret = lower_and_generate_type(c, &reg_pos);
    
     //LLVMStructType(reg_pos.types, reg_pos.current_word, false);
  }

  return ret;
}

LLVMValueRef load_lowered_param_value(compile_t* c, LLVMValueRef ptr,
  LLVMTypeRef param_type, reach_type_t* real_type)
{
  LLVMValueRef ret = NULL;
  char* triple = c->opt->triple;

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    // x86 only lowers structs with size of power of 2
    // so it can be loaded directely without a cast
    ret = LLVMBuildLoad2(c->builder, param_type, ptr, "");
  }
  else if(target_is_arm(c->opt->triple))
  {
    compile_type_t* p_c_t = (compile_type_t*)real_type->c_type;
    if(p_c_t->abi_size == (size_t)LLVMABISizeOfType(c->target_data, param_type))
    {
      ret = LLVMBuildLoad2(c->builder, param_type, ptr, "");
    }
    else
    {
      LLVMValueRef tmp_array = LLVMBuildAlloca(c->builder, param_type, "");
      LLVMValueRef cpy_size = LLVMConstInt(c->intptr, p_c_t->abi_size, false);
      LLVMValueRef zero = LLVMConstInt(c->i8, 0, false);
      LLVMBuildMemSet(c->builder, tmp_array, zero, cpy_size, LLVMGetAlignment(tmp_array));
      gencall_memcpy(c, tmp_array, ptr, cpy_size);
      ret = LLVMBuildLoad2(c->builder, param_type, tmp_array, "");
    }
  }
  else
  {
    pony_assert(false);
  }

  return ret;
}


LLVMValueRef load_lowered_return_value(compile_t* c, LLVMValueRef ptr,
  LLVMTypeRef return_type, reach_type_t* real_type)
{
  LLVMValueRef ret = NULL;
  char* triple = c->opt->triple;

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    // x86 only lowers structs with size of power of 2
    // so it can be loaded directely without a cast
    ret = LLVMBuildLoad2(c->builder, return_type, ptr, "");
  }
  else if(target_is_arm(c->opt->triple))
  {
    compile_type_t* p_c_t = (compile_type_t*)real_type->c_type;
    if(p_c_t->abi_size == (size_t)LLVMABISizeOfType(c->target_data, return_type))
    {
      ret = LLVMBuildLoad2(c->builder, return_type, ptr, "");
    }
    else
    {
      LLVMValueRef tmp_array = LLVMBuildAlloca(c->builder, return_type, "");
      LLVMValueRef cpy_size = LLVMConstInt(c->intptr, p_c_t->abi_size, false);
      LLVMValueRef zero = LLVMConstInt(c->i8, p_c_t->abi_size, false);
      LLVMBuildMemSet(c->builder, tmp_array, zero, cpy_size, LLVMGetAlignment(tmp_array));
      gencall_memcpy(c, tmp_array, ptr, cpy_size);
      ret = LLVMBuildLoad2(c->builder, return_type, tmp_array, "");
    }
  }
  else
  {
    pony_assert(false);
  }

  return ret;
}


void copy_lowered_param_value(compile_t* c, LLVMValueRef dest_ptr,
  LLVMValueRef param_value, reach_type_t* real_target_type)
{
  char* triple = c->opt->triple;

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    LLVMBuildStore(c->builder, param_value, dest_ptr);
  }
  else if(target_is_arm(c->opt->triple))
  {
    compile_type_t* p_c_t = (compile_type_t*)real_target_type->c_type;
    LLVMTypeRef param_type = LLVMTypeOf(param_value);
    if(p_c_t->abi_size == (size_t)LLVMABISizeOfType(c->target_data, param_type))
    {
      LLVMBuildStore(c->builder, param_value, dest_ptr);
    }
    else
    {
      LLVMValueRef tmp_array = LLVMBuildAlloca(c->builder, param_type, "");
      LLVMBuildStore(c->builder, param_value, tmp_array);
      LLVMValueRef cpy_size = LLVMConstInt(c->intptr, p_c_t->abi_size, false);
      gencall_memcpy(c, dest_ptr, tmp_array, cpy_size);
    }
  }
  else
  {
    pony_assert(false);
  }
}


void copy_lowered_return_value(compile_t* c, LLVMValueRef dest_ptr,
  LLVMValueRef return_value, reach_type_t* real_target_type)
{
  char* triple = c->opt->triple;

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    LLVMBuildStore(c->builder, return_value, dest_ptr);
  }
  else if(target_is_arm(c->opt->triple))
  {
    compile_type_t* p_c_t = (compile_type_t*)real_target_type->c_type;
    LLVMTypeRef return_type = LLVMTypeOf(return_value);
    if(p_c_t->abi_size == (size_t)LLVMABISizeOfType(c->target_data, return_type))
    {
      LLVMBuildStore(c->builder, return_value, dest_ptr);
    }
    else
    {
      LLVMValueRef tmp_array = LLVMBuildAlloca(c->builder, return_type, "");
      LLVMBuildStore(c->builder, return_value, tmp_array);
      LLVMValueRef cpy_size = LLVMConstInt(c->intptr, p_c_t->abi_size, false);
      gencall_memcpy(c, dest_ptr, tmp_array, cpy_size);
    }
  }
  else
  {
    pony_assert(false);
  }
}


void apply_function_value_param_attribute(compile_t* c, reach_type_t* pt, LLVMValueRef func,
  LLVMAttributeIndex param_nr)
{
  // Aarch64 Doesn't use byval
  if(target_is_arm(c->opt->triple) && target_is_lp64(c->opt->triple))
  {
    return;
  }

  if(!is_param_value_lowering_needed(c, pt))
  {
    unsigned int kind = LLVMGetEnumAttributeKindForName("byval", sizeof("byval") - 1);
    LLVMAttributeRef byvalue_attr = LLVMCreateTypeAttribute(
      c->context,
      kind,
      ((compile_type_t*)pt->c_type)->structure);
    // index 0 = return type, 1 ... paramters
    LLVMAddAttributeAtIndex(func, param_nr, byvalue_attr);
  }
}


void apply_call_site_value_param_attribute(compile_t* c, reach_type_t* pt, LLVMValueRef func,
  LLVMAttributeIndex param_nr)
{
  // Aarch64 Doesn't use byval
  if(target_is_arm(c->opt->triple) && target_is_lp64(c->opt->triple))
  {
    return;
  }

  if(!is_param_value_lowering_needed(c, pt))
  {
    unsigned int kind = LLVMGetEnumAttributeKindForName("byval", sizeof("byval") - 1);
    compile_type_t* p_c_t = (compile_type_t*)pt->c_type;
    LLVMAttributeRef byvalue_attr = LLVMCreateTypeAttribute(
      c->context,
      kind,
      p_c_t->structure);
    // index 0 = return type, 1 ... paramters
    LLVMAddCallSiteAttribute(func, param_nr, byvalue_attr);
  }
}


void apply_function_value_return_attribute(compile_t* c, reach_type_t* pt, LLVMValueRef func)
{
  if(!is_return_value_lowering_needed(c, pt))
  {
    unsigned int kind = LLVMGetEnumAttributeKindForName("sret", sizeof("sret") - 1);
    compile_type_t* ret_c_t = (compile_type_t*)pt->c_type;
    LLVMAttributeRef sret_attr = LLVMCreateTypeAttribute(
      c->context,
      kind,
      ret_c_t->structure);
    // index 1 = sret return type
    LLVMAddAttributeAtIndex(func, 1, sret_attr);
  }
}


void apply_call_site_value_return_attribute(compile_t* c, reach_type_t* pt,
  LLVMValueRef func)
{
  if(!is_return_value_lowering_needed(c, pt))
  {
    unsigned int kind = LLVMGetEnumAttributeKindForName("sret", sizeof("sret") - 1);
    compile_type_t* ret_c_t = (compile_type_t*)pt->c_type;
    LLVMAttributeRef sret_attr = LLVMCreateTypeAttribute(
      c->context,
      kind,
      ret_c_t->structure);
    // index 1 = sret return type
    LLVMAddCallSiteAttribute(func, 1, sret_attr);
  }
}