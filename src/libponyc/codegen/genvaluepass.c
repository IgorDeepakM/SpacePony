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
  LLVMTypeKind last_type_kind;
  LLVMTypeRef types[MAX_REG_TYPES];
}RegisterPos_t;


static bool is_power_of_2(size_t x)
{
  return x > 0 && !(x & (x - 1));
}

bool is_pass_by_value_lowering_supported(pass_opt_t* opt)
{
  bool ret = false;

  char* triple = opt->triple;

  if(target_is_x86(triple))
  {
    if(target_is_windows(triple) &&
       (target_is_llp64(triple) || target_is_ilp32(triple)))
    {
      ret = true;
    }
    else if(target_is_linux(triple) && target_is_lp64(triple))
    {
      return true;
    }
  }
  else if(target_is_arm(triple))
  {
    ret = true;
  }

  return ret;
}

static bool insert_type_hfa_aarch64(RegisterPos_t *pos, LLVMTypeRef type)
{
  LLVMTypeKind kind = LLVMGetTypeKind(type);

  if(pos->current_word == 0)
  {
    if(kind != LLVMFloatTypeKind && kind != LLVMDoubleTypeKind)
    {
      return false;
    }

    pos->current_word++;
    pos->last_type_kind = kind;
  }
  else
  {
    pos->current_word++;

    if(pos->last_type_kind != kind || pos->current_word > 4)
    {
      return false;
    }
  }

  return true;
}

static bool get_hfa_from_fixed_sized_array_aarch64(LLVMTypeRef array, RegisterPos_t *pos)
{
  LLVMTypeRef element_type = LLVMGetElementType(array);
  size_t num_elements = (size_t)LLVMGetArrayLength(array);

  for(size_t i = 0; i < num_elements; i++)
  {
    if(!insert_type_hfa_aarch64(pos, element_type))
    {
      return false;
    }
  }

  return true;
}

static bool get_hfa_from_structure_aarch64(LLVMTypeRef structure, RegisterPos_t *pos)
{
  size_t num_elements = (size_t)LLVMCountStructElementTypes(structure);

  for(size_t i = 0; i < num_elements; i++)
  {
    LLVMTypeRef curr_type = LLVMStructGetTypeAtIndex(structure, (unsigned int)i);

    switch(LLVMGetTypeKind(curr_type))
    {
      case LLVMStructTypeKind:
        if(!get_hfa_from_structure_aarch64(curr_type, pos))
        {
          return false;
        }
        break;
      case LLVMArrayTypeKind:
        if(!get_hfa_from_fixed_sized_array_aarch64(curr_type, pos))
        {
          return false;
        }
        break;
      default:
      {
        if(!insert_type_hfa_aarch64(pos, curr_type))
        {
          return false;
        }
        break;
      }
    }
  }

  return true;
}

static bool is_param_value_lowering_needed(compile_t* c, compile_type_t* p_t)
{
  bool ret = false;
  char* triple = c->opt->triple;

  if(target_is_x86(triple))
  {
    if(target_is_windows(triple))
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
    else if(target_is_linux(triple))
    {
      if(target_is_lp64(triple))
      {
        if(p_t->abi_size <= 16)
        {
          ret = true;
        }
      }
    }
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
    else if(target_is_lp64(triple))
    {
      // Aarch64 is always lowered because byval is not used at all
      ret = true;
    }
  }

  return ret;
}

bool is_return_value_lowering_needed(compile_t* c, reach_type_t* pt)
{
  bool ret = false;
  char* triple = c->opt->triple;

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  if(target_is_x86(triple))
  {
    if(target_is_windows(triple))
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
    else if(target_is_linux(triple))
    {
      if(target_is_lp64(triple))
      {
        if(p_t->abi_size <= 16)
        {
          ret = true;
        }
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
    else if(target_is_lp64(triple))
    {
      RegisterPos_t reg_pos;
      reg_pos.current_word = 0;
      reg_pos.current_byte_in_word = 0;
      reg_pos.bytes_per_word = 0; // Not used
      reg_pos.last_type_kind = LLVMVoidTypeKind;

      if(get_hfa_from_structure_aarch64(p_t->structure, &reg_pos))
      {
        ret = true;
      }
      else if(p_t->abi_size <= 16)
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


static void insert_type_x86_64_systemv(compile_t* c, RegisterPos_t *pos, LLVMTypeRef type)
{
  LLVMTypeKind kind = LLVMGetTypeKind(type);
  size_t size = (size_t)LLVMABISizeOfType(c->target_data, type);

  size_t next_byte_in_word = pos->current_byte_in_word + size;

  if(pos->current_word == 0 || next_byte_in_word > 8 ||
     kind == LLVMDoubleTypeKind)
  {
    if(kind == LLVMIntegerTypeKind)
    {
      pos->types[pos->current_word] =
        get_type_from_size(c, next_power_of_2(size));
    }
    else if(kind == LLVMDoubleTypeKind)
    {
      pos->types[pos->current_word] = c->f64;
    }
    else if(kind == LLVMFloatTypeKind)
    {
      pos->types[pos->current_word] = c->f32;
    }
    pos->current_word++;
    pos->current_byte_in_word = size;
    pos->last_type_kind = kind;
  }
  else
  {
    if(pos->last_type_kind == LLVMFloatTypeKind && kind == LLVMFloatTypeKind)
    {
      pos->types[pos->current_word - 1] = LLVMVectorType(c->f32, 2);
    }
    else
    {
      pos->types[pos->current_word - 1] =
        get_type_from_size(c, next_power_of_2(next_byte_in_word));
    }
    pos->current_byte_in_word = next_byte_in_word;
    pos->last_type_kind = kind;
  }
}


static void lower_fixed_sized_array_x86_64_systemv(compile_t* c, LLVMTypeRef array, RegisterPos_t *pos)
{
  LLVMTypeRef element_type = LLVMGetElementType(array);
  size_t num_elements = (size_t)LLVMGetArrayLength(array);

  for(size_t i = 0; i < num_elements; i++)
  {
    insert_type_x86_64_systemv(c, pos, element_type);
  }
}


static void lower_structure_x86_64_systemv(compile_t* c, LLVMTypeRef structure, RegisterPos_t *pos)
{
  size_t num_elements = (size_t)LLVMCountStructElementTypes(structure);

  for(size_t i = 0; i < num_elements; i++)
  {
    LLVMTypeRef curr_type = LLVMStructGetTypeAtIndex(structure, (unsigned int)i);

    switch(LLVMGetTypeKind(curr_type))
    {
      case LLVMStructTypeKind:
        lower_structure_x86_64_systemv(c, curr_type, pos);
        break;
      case LLVMArrayTypeKind:
        lower_fixed_sized_array_x86_64_systemv(c, curr_type, pos);
        break;
      default:
      {
        insert_type_x86_64_systemv(c, pos, curr_type);
        break;
      }
    }
  }
}

static LLVMTypeRef generate_flattened_type(compile_t* c, RegisterPos_t *pos)
{
  return LLVMStructTypeInContext(c->context, pos->types,
    (unsigned int)pos->current_word, false);
}

LLVMTypeRef lower_param_value_from_structure_type(compile_t* c, reach_type_t* pt)
{
  LLVMTypeRef ret = NULL;
  char* triple = c->opt->triple;

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  // If no lowering is needed then just return with the same pointer to structure type
  if(!is_param_value_lowering_needed(c, p_t))
  {
    return p_t->use_type;
  }

  if(target_is_x86(triple))
  {
    if(target_is_windows(triple))
    {
      if(target_is_llp64(triple))
      {
        ret = get_type_from_size(c, p_t->abi_size);
      }
    }
    else if(target_is_linux(triple))
    {
      if(target_is_lp64(triple))
      {
        size_t ptr_size = (size_t)LLVMABISizeOfType(c->target_data, c->intptr);

        RegisterPos_t reg_pos;
        reg_pos.current_word = 0;
        reg_pos.current_byte_in_word = 0;
        reg_pos.bytes_per_word = ptr_size;
        reg_pos.last_type_kind = LLVMVoidTypeKind;

        lower_structure_x86_64_systemv(c, p_t->structure, &reg_pos);

        ret = generate_flattened_type(c, &reg_pos);
      }
    }
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
    else if(target_is_lp64(triple))
    {
      RegisterPos_t reg_pos;
      reg_pos.current_word = 0;
      reg_pos.current_byte_in_word = 0;
      reg_pos.bytes_per_word = 0; // Not used
      reg_pos.last_type_kind = LLVMVoidTypeKind;

      LLVMTypeRef structure = p_t->structure;

      if(get_hfa_from_structure_aarch64(structure, &reg_pos))
      {
        if(reg_pos.last_type_kind == LLVMFloatTypeKind)
        {
          ret = LLVMArrayType(c->f32, (unsigned int)reg_pos.current_word);
        }
        else if(reg_pos.last_type_kind == LLVMDoubleTypeKind)
        {
          ret = LLVMArrayType(c->f64, (unsigned int)reg_pos.current_word);
        }
        else
        {
          pony_assert(false);
        }
      }
      else if(p_t->abi_size <= 8)
      {
        ret = get_type_from_size(c, next_power_of_2(p_t->abi_size));
      }
      else if(p_t->abi_size <= 16)
      {
        ret = LLVMArrayType(c->i64, 2);
      }
      else
      {
        ret = p_t->use_type;
      }
    }
  }

  return ret;
}

LLVMTypeRef lower_return_value_from_structure_type(compile_t* c, reach_type_t* pt)
{
  LLVMTypeRef ret = NULL;
  char* triple = c->opt->triple;

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  if(target_is_x86(triple))
  {
    if(target_is_windows(triple))
    {
      if(target_is_llp64(triple) || target_is_ilp32(triple))
      {
        ret = get_type_from_size(c, p_t->abi_size);
      }
    }
    else if(target_is_linux(triple))
    {
      if(target_is_lp64(triple))
      {
        size_t ptr_size = (size_t)LLVMABISizeOfType(c->target_data, c->intptr);

        RegisterPos_t reg_pos;
        reg_pos.current_word = 0;
        reg_pos.current_byte_in_word = 0;
        reg_pos.bytes_per_word = ptr_size;
        reg_pos.last_type_kind = LLVMVoidTypeKind;

        lower_structure_x86_64_systemv(c, p_t->structure, &reg_pos);

        ret = generate_flattened_type(c, &reg_pos);
      }
    }
  }
  else if(target_is_arm(triple))
  {
    if(target_is_ilp32(triple))
    {
      ret = get_type_from_size(c, next_power_of_2(p_t->abi_size));
    }
    else if(target_is_lp64(triple))
    {
      RegisterPos_t reg_pos;
      reg_pos.current_word = 0;
      reg_pos.current_byte_in_word = 0;
      reg_pos.bytes_per_word = 0; // Not used
      reg_pos.last_type_kind = LLVMVoidTypeKind;

      if(get_hfa_from_structure_aarch64(p_t->structure, &reg_pos))
      {
        ret = p_t->structure;
      }
      else if(p_t->abi_size <= 8)
      {
        ret = get_type_from_size(c, next_power_of_2(p_t->abi_size));
      }
      else if(p_t->abi_size <= 16)
      {
        ret = LLVMArrayType(c->i64, 2);
      }
      else
      {
        ret = p_t->use_type;
      }
    }
  }

  return ret;
}

static LLVMValueRef copy_from_ptr_to_value_zero_extend(compile_t* c, LLVMValueRef ptr,
  LLVMTypeRef param_type, compile_type_t* p_c_t)
{
  LLVMValueRef ret = NULL;

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

  return ret;
}

LLVMValueRef load_lowered_param_value_from_ptr(compile_t* c, LLVMValueRef ptr,
  LLVMTypeRef param_type, reach_type_t* real_type)
{
  LLVMValueRef ret = NULL;
  char* triple = c->opt->triple;

  compile_type_t* p_c_t = (compile_type_t*)real_type->c_type;

  // If no lowering is needed then just return the pointer and byval will be used
  if(!is_param_value_lowering_needed(c, p_c_t))
  {
    return ptr;
  }

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    // x86 only lowers structs with size of power of 2
    // so it can be loaded directely without a cast
    ret = LLVMBuildLoad2(c->builder, param_type, ptr, "");
  }
  else if(target_is_x86(triple) && target_is_linux(triple) && target_is_lp64(triple))
  {
    ret = copy_from_ptr_to_value_zero_extend(c, ptr, param_type, p_c_t);
  }
  else if(target_is_arm(triple))
  {
    if(target_is_lp64(triple) && p_c_t->abi_size > 16)
    {
      RegisterPos_t reg_pos;
      reg_pos.current_word = 0;
      reg_pos.current_byte_in_word = 0;
      reg_pos.bytes_per_word = 0; // Not used
      reg_pos.last_type_kind = LLVMVoidTypeKind;

      bool is_hfa = get_hfa_from_structure_aarch64(p_c_t->structure, &reg_pos);

      if(!is_hfa)
      {
        // Aarch doesn't use byval and because of that you need to manually
        // create a copy of the structure on the stack and copy it.
        LLVMValueRef caller_copy = LLVMBuildAlloca(c->builder, p_c_t->structure, "");
        LLVMValueRef cpy_size = LLVMConstInt(c->intptr, p_c_t->abi_size, false);
        gencall_memcpy(c, caller_copy, ptr, cpy_size);
        ret = caller_copy;
      }
      else
      {
        ret = copy_from_ptr_to_value_zero_extend(c, ptr, param_type, p_c_t);
      }
    }
    else
    {
      ret = copy_from_ptr_to_value_zero_extend(c, ptr, param_type, p_c_t);
    }
  }
  else
  {
    pony_assert(false);
  }

  return ret;
}


LLVMValueRef load_lowered_return_value_from_ptr(compile_t* c, LLVMValueRef ptr,
  LLVMTypeRef return_type, reach_type_t* real_type)
{
  LLVMValueRef ret = NULL;
  char* triple = c->opt->triple;

  compile_type_t* p_c_t = (compile_type_t*)real_type->c_type;

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    // x86 only lowers structs with size of power of 2
    // so it can be loaded directely without a cast
    ret = LLVMBuildLoad2(c->builder, return_type, ptr, "");
  }
  else if(target_is_arm(triple) ||
          (target_is_x86(triple) && target_is_linux(triple) && target_is_lp64(triple)))
  {
    ret = copy_from_ptr_to_value_zero_extend(c, ptr, return_type, p_c_t);
  }
  else
  {
    pony_assert(false);
  }

  return ret;
}

void copy_lowered_param_value_to_ptr(compile_t* c, LLVMValueRef dest_ptr,
  LLVMValueRef param_value, reach_type_t* real_target_type)
{
  char* triple = c->opt->triple;
  compile_type_t* p_c_t = (compile_type_t*)real_target_type->c_type;

  // If no lowering is needed then just copy from the pointer provided
  if(!is_param_value_lowering_needed(c, p_c_t))
  {
    LLVMValueRef l_size = LLVMConstInt(c->intptr, p_c_t->abi_size, false);
    gencall_memcpy(c, dest_ptr, param_value, l_size);
    return;
  }

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    LLVMBuildStore(c->builder, param_value, dest_ptr);
  }
  else if(target_is_arm(triple) ||
          (target_is_x86(triple) && target_is_linux(triple) && target_is_lp64(triple)))
  {
    if(target_is_arm(triple) && target_is_lp64(triple) && p_c_t->abi_size > 16)
    {
      LLVMValueRef l_size = LLVMConstInt(c->intptr, p_c_t->abi_size, false);
      gencall_memcpy(c, dest_ptr, param_value, l_size);
      return;
    }

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


void copy_lowered_return_value_to_ptr(compile_t* c, LLVMValueRef dest_ptr,
  LLVMValueRef return_value, reach_type_t* real_target_type)
{
  char* triple = c->opt->triple;

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    LLVMBuildStore(c->builder, return_value, dest_ptr);
  }
  else if(target_is_arm(triple) ||
          (target_is_x86(triple) && target_is_linux(triple) && target_is_lp64(triple)))
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
  // Aarch64 doesn't use byval
  if(target_is_arm(c->opt->triple) && target_is_lp64(c->opt->triple))
  {
    return;
  }

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  if(!is_param_value_lowering_needed(c, p_t))
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
  // Aarch64 doesn't use byval
  if(target_is_arm(c->opt->triple) && target_is_lp64(c->opt->triple))
  {
    return;
  }

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  if(!is_param_value_lowering_needed(c, p_t))
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