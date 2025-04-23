#include "genvaluepass.h"
#include "gentype.h"
#include "ponyassert.h"
#include "genopt.h"
#include "gencall.h"
#include <array>

#include "llvm_config_begin.h"

#include <llvm/IR/IRBuilder.h>


using namespace llvm;

typedef struct
{
  size_t current_word;
  size_t current_byte_in_word;
  size_t bytes_per_word;
  Type::TypeID last_type_kind;
  std::array<Type*, 4> types;
}RegisterPos_t;


static bool is_power_of_2(size_t x)
{
  return x > 0 && !(x & (x - 1));
}

extern "C" bool is_pass_by_value_lowering_supported(pass_opt_t* opt)
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

static bool insert_type_hfa_aarch64(RegisterPos_t *pos, Type* type)
{
  Type::TypeID kind = type->getTypeID();

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

static bool get_hfa_from_fixed_sized_array_aarch64(ArrayType* array, RegisterPos_t *pos)
{
  Type* element_type = array->getElementType();
  size_t num_elements = (size_t)array->getNumElements();

  for(size_t i = 0; i < num_elements; i++)
  {
    if(!insert_type_hfa_aarch64(pos, element_type))
    {
      return false;
    }
  }

  return true;
}

static bool get_hfa_from_structure_aarch64(StructType* structure, RegisterPos_t *pos)
{
  size_t num_elements = (size_t)structure->getNumElements();

  for(size_t i = 0; i < num_elements; i++)
  {
    Type* curr_type = structure->getTypeAtIndex(i);

    switch(curr_type->getTypeID())
    {
      case Type::TypeID::StructTyID:
        if(!get_hfa_from_structure_aarch64(dyn_cast<StructType>(curr_type), pos))
        {
          return false;
        }
        break;
      case Type::TypeID::ArrayTyID:
        if(!get_hfa_from_fixed_sized_array_aarch64(dyn_cast<ArrayType>(curr_type), pos))
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

extern "C" bool is_return_value_lowering_needed(compile_t* c, reach_type_t* pt)
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
      RegisterPos_t reg_pos{ 0, 0, 0, Type::TypeID::VoidTyID};
      reg_pos.current_word = 0;
      reg_pos.current_byte_in_word = 0;
      reg_pos.bytes_per_word = 0; // Not used
      reg_pos.last_type_kind = Type::TypeID::VoidTyID;

      if(get_hfa_from_structure_aarch64(unwrap<StructType>(p_t->structure), &reg_pos))
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


static Type* get_type_from_size(compile_t* c, size_t size)
{
  switch(size)
  {
    case 8:
      return unwrap(c->i64);
    case 4:
      return unwrap(c->i32);
    case 2:
      return unwrap(c->i16);
    case 1:
      return unwrap(c->i8);
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


static void insert_type_x86_64_systemv(compile_t* c, RegisterPos_t *pos, Type* type)
{
  auto target_data = unwrap(c->target_data);

  Type::TypeID kind = type->getTypeID();
  size_t size = (size_t)target_data->getTypeAllocSize(type);

  size_t next_byte_in_word = pos->current_byte_in_word + size;

  if(pos->current_word == 0 || next_byte_in_word > 8 ||
     kind == Type::TypeID::DoubleTyID)
  {
    if(kind == Type::TypeID::IntegerTyID)
    {
      pos->types.at(pos->current_word) =
        get_type_from_size(c, next_power_of_2(size));
    }
    else if(kind == Type::TypeID::DoubleTyID)
    {
      pos->types.at(pos->current_word) = unwrap(c->f64);
    }
    else if(kind == Type::TypeID::FloatTyID)
    {
      pos->types.at(pos->current_word) = unwrap(c->f32);
    }
    pos->current_word++;
    pos->current_byte_in_word = size;
    pos->last_type_kind = kind;
  }
  else
  {
    if(pos->last_type_kind == LLVMFloatTypeKind && kind == LLVMFloatTypeKind)
    {
      pos->types.at(pos->current_word - 1) = unwrap(c->f64);
    }
    else
    {
      pos->types.at(pos->current_word - 1) =
        get_type_from_size(c, next_power_of_2(next_byte_in_word));
    }
    pos->current_byte_in_word = next_byte_in_word;
    pos->last_type_kind = kind;
  }
}


static void lower_fixed_sized_array_x86_64_systemv(compile_t* c, ArrayType* array, RegisterPos_t *pos)
{
  Type* element_type = array->getElementType();
  size_t num_elements = (size_t)array->getNumElements();

  for(size_t i = 0; i < num_elements; i++)
  {
    insert_type_x86_64_systemv(c, pos, element_type);
  }
}


static void lower_structure_x86_64_systemv(compile_t* c, StructType* structure, RegisterPos_t *pos)
{
  size_t num_elements = (size_t)structure->getNumElements();

  for(size_t i = 0; i < num_elements; i++)
  {
    Type* curr_type = structure->getTypeAtIndex(i);

    switch(curr_type->getTypeID())
    {
      case Type::TypeID::StructTyID:
        lower_structure_x86_64_systemv(c, dyn_cast<StructType>(curr_type), pos);
        break;
      case Type::TypeID::ArrayTyID:
        lower_fixed_sized_array_x86_64_systemv(c, dyn_cast<ArrayType>(curr_type), pos);
        break;
      default:
      {
        insert_type_x86_64_systemv(c, pos, curr_type);
        break;
      }
    }
  }
}

static Type* generate_flattened_type(compile_t* c, RegisterPos_t *pos)
{
  ArrayRef<Type*> Tys(pos->types.data(), pos->current_word);
  return StructType::get(*unwrap(c->context), Tys);
}

extern "C" LLVMTypeRef lower_param_value_from_structure_type(compile_t* c, reach_type_t* pt)
{
  Type* ret = nullptr;
  char* triple = c->opt->triple;

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  auto target_data = unwrap(c->target_data);

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
        size_t ptr_size = (size_t)target_data->getTypeAllocSize(unwrap(c->intptr));

        RegisterPos_t reg_pos{ 0, 0, 0, Type::TypeID::VoidTyID };
        reg_pos.current_word = 0;
        reg_pos.current_byte_in_word = 0;
        reg_pos.bytes_per_word = ptr_size;
        reg_pos.last_type_kind = Type::TypeID::VoidTyID;

        lower_structure_x86_64_systemv(c, dyn_cast<StructType>(unwrap(p_t->structure)), &reg_pos);

        ret = generate_flattened_type(c, &reg_pos);
      }
    }
  }
  else if(target_is_arm(triple))
  {
    if(target_is_ilp32(triple))
    {
      size_t align = (size_t)target_data->getABITypeAlign(unwrap(p_t->structure)).value();

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
      Type* array_type = get_type_from_size(c, align);
      ret = ArrayType::get(array_type, (uint64_t)array_size);
    }
    else if(target_is_lp64(triple))
    {
      RegisterPos_t reg_pos{ 0, 0, 0, Type::TypeID::VoidTyID };
      reg_pos.current_word = 0;
      reg_pos.current_byte_in_word = 0;
      reg_pos.bytes_per_word = 0; // Not used
      reg_pos.last_type_kind = Type::TypeID::VoidTyID;

      if(get_hfa_from_structure_aarch64(unwrap<StructType>(p_t->structure), &reg_pos))
      {
        if(reg_pos.last_type_kind == Type::TypeID::FloatTyID)
        {
          ret = ArrayType::get(unwrap(c->f32), (uint64_t)reg_pos.current_word);
        }
        else if(reg_pos.last_type_kind == Type::TypeID::DoubleTyID)
        {
          ret = ArrayType::get(unwrap(c->f64), (uint64_t)reg_pos.current_word);
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
        ret = ArrayType::get(unwrap(c->i64), 2);
      }
      else
      {
        ret = unwrap(p_t->use_type);
      }
    }
  }

  return wrap(ret);
}

extern "C" LLVMTypeRef lower_return_value_from_structure_type(compile_t* c, reach_type_t* pt)
{
  Type* ret = NULL;
  char* triple = c->opt->triple;

  auto target_data = unwrap(c->target_data);

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
        size_t ptr_size = (size_t)target_data->getTypeAllocSize(unwrap(c->intptr));

        RegisterPos_t reg_pos{ 0, 0, 0, Type::TypeID::VoidTyID };
        reg_pos.current_word = 0;
        reg_pos.current_byte_in_word = 0;
        reg_pos.bytes_per_word = ptr_size;
        reg_pos.last_type_kind = Type::TypeID::VoidTyID;

        lower_structure_x86_64_systemv(c, unwrap<StructType>(p_t->structure), &reg_pos);

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
      RegisterPos_t reg_pos{ 0, 0, 0, Type::TypeID::VoidTyID };
      reg_pos.current_word = 0;
      reg_pos.current_byte_in_word = 0;
      reg_pos.bytes_per_word = 0; // Not used
      reg_pos.last_type_kind = Type::TypeID::VoidTyID;

      if(get_hfa_from_structure_aarch64(unwrap<StructType>(p_t->structure), &reg_pos))
      {
        ret = unwrap(p_t->structure);
      }
      else if(p_t->abi_size <= 8)
      {
        ret = get_type_from_size(c, next_power_of_2(p_t->abi_size));
      }
      else if(p_t->abi_size <= 16)
      {
        ret = ArrayType::get(unwrap(c->i64), 2);
      }
      else
      {
        ret = unwrap(p_t->use_type);
      }
    }
  }

  return wrap(ret);
}

static Value* copy_from_ptr_to_value_zero_extend(compile_t* c, Value* ptr,
  Type* param_type, compile_type_t* p_c_t)
{
  Value* ret = nullptr;

  auto builder = unwrap(c->builder);
  auto target_data = unwrap(c->target_data);

  if(p_c_t->abi_size == (size_t)target_data->getTypeAllocSize(param_type))
  {
    ret = builder->CreateLoad(param_type, ptr, "");
  }
  else
  {
    AllocaInst* tmp_array = builder->CreateAlloca(param_type, nullptr, "");
    ConstantInt* cpy_size = ConstantInt::get(unwrap<IntegerType>(c->intptr), p_c_t->abi_size);
    ConstantInt* zero = ConstantInt::get(unwrap<IntegerType>(c->i8), 0);
    builder->CreateMemSet(tmp_array, zero, cpy_size, tmp_array->getAlign());
    gencall_memcpy(c, wrap(tmp_array), wrap(ptr), wrap(cpy_size));
    ret = builder->CreateLoad(param_type, tmp_array, "");
  }

  return ret;
}

extern "C" LLVMValueRef load_lowered_param_value_from_ptr(compile_t* c, LLVMValueRef ptr,
  LLVMTypeRef param_type, reach_type_t* real_type)
{
  Value* ret = nullptr;
  char* triple = c->opt->triple;

  auto builder = unwrap(c->builder);

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
    ret = builder->CreateLoad(unwrap(param_type), unwrap(ptr), "");
  }
  else if(target_is_x86(triple) && target_is_linux(triple) && target_is_lp64(triple))
  {
    ret = copy_from_ptr_to_value_zero_extend(c, unwrap(ptr), unwrap(param_type), p_c_t);
  }
  else if(target_is_arm(triple))
  {
    if(target_is_lp64(triple) && p_c_t->abi_size > 16)
    {
      RegisterPos_t reg_pos{ 0, 0, 0, Type::TypeID::VoidTyID };
      reg_pos.current_word = 0;
      reg_pos.current_byte_in_word = 0;
      reg_pos.bytes_per_word = 0; // Not used
      reg_pos.last_type_kind = Type::TypeID::VoidTyID;

      bool is_hfa = get_hfa_from_structure_aarch64(unwrap<StructType>(p_c_t->structure), &reg_pos);

      if(!is_hfa)
      {
        // Aarch doesn't use byval and because of that you need to manually
        // create a copy of the structure on the stack and copy it.
        AllocaInst* caller_copy = builder->CreateAlloca(unwrap(p_c_t->structure), nullptr, "");
        ConstantInt* cpy_size = ConstantInt::get(unwrap<IntegerType>(c->intptr), p_c_t->abi_size);
        gencall_memcpy(c, wrap(caller_copy), ptr, wrap(cpy_size));
        ret = caller_copy;
      }
      else
      {
        ret = copy_from_ptr_to_value_zero_extend(c, unwrap(ptr), unwrap(param_type), p_c_t);
      }
    }
    else
    {
      ret = copy_from_ptr_to_value_zero_extend(c, unwrap(ptr), unwrap(param_type), p_c_t);
    }
  }
  else
  {
    pony_assert(false);
  }

  return wrap(ret);
}


extern "C" LLVMValueRef load_lowered_return_value_from_ptr(compile_t* c, LLVMValueRef ptr,
  LLVMTypeRef return_type, reach_type_t* real_type)
{
  Value* ret = nullptr;
  char* triple = c->opt->triple;

  auto builder = unwrap(c->builder);

  compile_type_t* p_c_t = (compile_type_t*)real_type->c_type;

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    // x86 only lowers structs with size of power of 2
    // so it can be loaded directely without a cast
    ret = builder->CreateLoad(unwrap(return_type), unwrap(ptr), "");
  }
  else if(target_is_arm(triple) ||
          (target_is_x86(triple) && target_is_linux(triple) && target_is_lp64(triple)))
  {
    ret = copy_from_ptr_to_value_zero_extend(c, unwrap(ptr), unwrap(return_type), p_c_t);
  }
  else
  {
    pony_assert(false);
  }

  return wrap(ret);
}


extern "C" void copy_lowered_param_value_to_ptr(compile_t* c, LLVMValueRef dest_ptr,
  LLVMValueRef param_value, reach_type_t* real_target_type)
{
  char* triple = c->opt->triple;
  compile_type_t* p_c_t = (compile_type_t*)real_target_type->c_type;

  auto builder = unwrap(c->builder);
  auto target_data = unwrap(c->target_data);

  // If no lowering is needed then just copy from the pointer provided
  if(!is_param_value_lowering_needed(c, p_c_t))
  {
    ConstantInt* l_size = ConstantInt::get(unwrap<IntegerType>(c->intptr), p_c_t->abi_size);
    gencall_memcpy(c, dest_ptr, param_value, wrap(l_size));
    return;
  }

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    builder->CreateStore(unwrap(param_value), unwrap(dest_ptr));
  }
  else if(target_is_arm(triple) ||
          (target_is_x86(triple) && target_is_linux(triple) && target_is_lp64(triple)))
  {
    if(target_is_arm(triple) && target_is_lp64(triple))
    {
      RegisterPos_t reg_pos{ 0, 0, 0, Type::TypeID::VoidTyID };
      reg_pos.current_word = 0;
      reg_pos.current_byte_in_word = 0;
      reg_pos.bytes_per_word = 0; // Not used
      reg_pos.last_type_kind = Type::TypeID::VoidTyID;

      bool is_hfa = get_hfa_from_structure_aarch64(unwrap<StructType>(p_c_t->structure), &reg_pos);

      if(p_c_t->abi_size > 16 && !is_hfa)
      {
       ConstantInt* l_size = ConstantInt::get(unwrap<IntegerType>(c->intptr), p_c_t->abi_size);
        gencall_memcpy(c, dest_ptr, param_value, wrap(l_size));
        return;
      }
    }

    Type* param_type = unwrap(param_value)->getType();
    if(p_c_t->abi_size == (size_t)target_data->getTypeAllocSize(param_type))
    {
      builder->CreateStore(unwrap(param_value), unwrap(dest_ptr));
    }
    else
    {
      AllocaInst* tmp_array = builder->CreateAlloca(param_type, nullptr, "");
      builder->CreateStore(unwrap(param_value), tmp_array);
      ConstantInt* cpy_size = ConstantInt::get(unwrap<IntegerType>(c->intptr), p_c_t->abi_size);
      gencall_memcpy(c, dest_ptr, wrap(tmp_array), wrap(cpy_size));
    }
  }
  else
  {
    pony_assert(false);
  }
}


extern "C" void copy_lowered_return_value_to_ptr(compile_t* c, LLVMValueRef dest_ptr,
  LLVMValueRef return_value, reach_type_t* real_target_type)
{
  char* triple = c->opt->triple;
  compile_type_t* p_c_t = (compile_type_t*)real_target_type->c_type;

  auto builder = unwrap(c->builder);

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    builder->CreateStore(unwrap(return_value), unwrap(dest_ptr));
  }
  else if(target_is_arm(triple) ||
          (target_is_x86(triple) && target_is_linux(triple) && target_is_lp64(triple)))
  {
    LLVMTypeRef return_type = LLVMTypeOf(return_value);
    if(p_c_t->abi_size == (size_t)LLVMABISizeOfType(c->target_data, return_type))
    {
      builder->CreateStore(unwrap(return_value), unwrap(dest_ptr));
    }
    else
    {
      AllocaInst* tmp_array = builder->CreateAlloca(unwrap(return_type), nullptr, "");
      builder->CreateStore(unwrap(return_value), tmp_array);
      ConstantInt* cpy_size = ConstantInt::get(unwrap<IntegerType>(c->intptr), p_c_t->abi_size);
      gencall_memcpy(c, dest_ptr, wrap(tmp_array), wrap(cpy_size));
    }
  }
  else
  {
    pony_assert(false);
  }
}


extern "C" void apply_function_value_param_attribute(compile_t* c, reach_type_t* pt,
  LLVMValueRef func, LLVMAttributeIndex param_nr)
{
  // Aarch64 doesn't use byval
  if(target_is_arm(c->opt->triple) && target_is_lp64(c->opt->triple))
  {
    return;
  }

  compile_type_t* p_c_t = (compile_type_t*)pt->c_type;

  if(!is_param_value_lowering_needed(c, p_c_t))
  {
    Attribute::AttrKind kind = Attribute::getAttrKindFromName("byval");
    Attribute byvalue_attr = Attribute::get(*unwrap(c->context), kind, unwrap(p_c_t->structure));
    // index 0 = return type, 1 ... paramters
    unwrap<Function>(func)->addAttributeAtIndex(param_nr, byvalue_attr);
  }
}


extern "C" void apply_call_site_value_param_attribute(compile_t* c, reach_type_t* pt,
  LLVMValueRef func, LLVMAttributeIndex param_nr)
{
  // Aarch64 doesn't use byval
  if(target_is_arm(c->opt->triple) && target_is_lp64(c->opt->triple))
  {
    return;
  }

  compile_type_t* p_c_t = (compile_type_t*)pt->c_type;

  if(!is_param_value_lowering_needed(c, p_c_t))
  {
    Attribute::AttrKind kind = Attribute::getAttrKindFromName("byval");
    Attribute byvalue_attr = Attribute::get(*unwrap(c->context), kind, unwrap(p_c_t->structure));
    // index 0 = return type, 1 ... paramters
    unwrap<CallBase>(func)->addAttributeAtIndex(param_nr, byvalue_attr);
  }
}


extern "C" void apply_function_value_return_attribute(compile_t* c, reach_type_t* pt,
  LLVMValueRef func)
{
  if(!is_return_value_lowering_needed(c, pt))
  {
    Attribute::AttrKind kind = Attribute::getAttrKindFromName("sret");
    compile_type_t* ret_c_t = (compile_type_t*)pt->c_type;
    Attribute sret_attr = Attribute::get(*unwrap(c->context), kind, unwrap(ret_c_t->structure));
    // index 1 = sret return type
    unwrap<Function>(func)->addAttributeAtIndex(1, sret_attr);
  }
}


extern "C" void apply_call_site_value_return_attribute(compile_t* c, reach_type_t* pt,
  LLVMValueRef func)
{
  if(!is_return_value_lowering_needed(c, pt))
  {
    Attribute::AttrKind kind = Attribute::getAttrKindFromName("sret");
    compile_type_t* ret_c_t = (compile_type_t*)pt->c_type;
    Attribute sret_attr = Attribute::get(*unwrap(c->context), kind, unwrap(ret_c_t->structure));
    // index 1 = sret return type
    unwrap<CallBase>(func)->addAttributeAtIndex(1, sret_attr);
  }
}