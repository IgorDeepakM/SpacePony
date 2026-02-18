#include "genvaluepass.h"
#include "gentype.h"
#include "ponyassert.h"
#include "genopt.h"
#include "gencall.h"
#include "../type/subtype.h"
#include <vector>

#include "llvm_config_begin.h"

#include <llvm/IR/IRBuilder.h>


using namespace llvm;
using namespace std;


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
  else if(target_is_riscv(triple))
  {
    if(target_is_lp64(triple))
    {
      ret = true;
    }
  }

  ret = true;

  return ret;
}


static bool flatten_structure(StructType* structure, vector<Type*>& flattened, size_t max_elements)
{
  size_t num_elements = (size_t)structure->getNumElements();

  for(size_t i = 0; i < num_elements; i++)
  {
    Type* curr_type = structure->getTypeAtIndex(i);

    switch(curr_type->getTypeID())
    {
      case Type::TypeID::StructTyID:
        return flatten_structure(dyn_cast<StructType>(curr_type), flattened, max_elements);
      case Type::TypeID::ArrayTyID:
      {
        ArrayType* array = dyn_cast<ArrayType>(curr_type);
        Type* element_type = array->getElementType();
        size_t num_elements = (size_t)array->getNumElements();

        for(size_t i = 0; i < num_elements; i++)
        {
          if(flattened.size() < max_elements)
          {
            flattened.push_back(element_type);
          }
          else
          {
            return false;
          }
        }
        break;
      }
      default:
      {
        if(flattened.size() < max_elements)
        {
          flattened.push_back(curr_type);
        }
        else
        {
          return false;
        }
        break;
      }
    }
  }

  return true;
}


static bool get_hfa_from_structure_aarch64(StructType* structure, Type*& type_ret, size_t& num_elem)
{
  vector<Type*> flat;

  if(!flatten_structure(structure, flat, 4))
  {
    return false;
  }

  Type::TypeID last_type_kind = Type::TypeID::VoidTyID;
  size_t current_word = 0;

  for(Type* t : flat)
  {
    Type::TypeID kind = t->getTypeID();

    if(current_word == 0)
    {
      if(kind != Type::TypeID::FloatTyID && kind != Type::TypeID::DoubleTyID)
      {
        return false;
      }
      else
      {
        type_ret = t;
      }

      current_word++;
      last_type_kind = kind;
    }
    else
    {
      if(last_type_kind != kind)
      {
        return false;
      }

      current_word++;
    }
  }

  num_elem = current_word;

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
  else if(target_is_riscv(triple))
  {
    if(target_is_lp64(triple))
    {
      // RISC-V is always lowered because byval is not used at all
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
      Type* hfa_type = nullptr;
      size_t num_elem = 0;

      if(get_hfa_from_structure_aarch64(unwrap<StructType>(p_t->structure), hfa_type, num_elem))
      {
        ret = true;
      }
      else if(p_t->abi_size <= 16)
      {
        ret = true;
      }
    }
  }
  else if(target_is_riscv(triple))
  {
    if(target_is_lp64(triple))
    {
      if(p_t->abi_size <= 16)
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


static void lower_structure_x86_64_systemv(compile_t* c, StructType* structure,
  size_t bytes_per_word, vector<Type*>& lowered)
{
  vector<Type*> flat;

  flatten_structure(structure, flat, 4);

  auto target_data = unwrap(c->target_data);

  size_t current_word = 0;
  size_t current_byte_in_word = 0;
  Type::TypeID last_type_kind = Type::TypeID::VoidTyID;

  for(Type* t : flat)
  {
    Type::TypeID kind = t->getTypeID();
    size_t size = (size_t)target_data->getTypeAllocSize(t);

    size_t next_byte_in_word = current_byte_in_word + size;

    if(current_byte_in_word == 0)
    {
      lowered.resize(current_word + 1);
    }

    if(current_byte_in_word == 0 || kind == Type::TypeID::DoubleTyID)
    {
      if(kind == Type::TypeID::IntegerTyID)
      {
        lowered[current_word] = get_type_from_size(c, next_power_of_2(size));
      }
      else if(kind == Type::TypeID::DoubleTyID)
      {
        lowered[current_word] = unwrap(c->f64);
      }
      else if(kind == Type::TypeID::FloatTyID)
      {
        lowered[current_word] = unwrap(c->f32);
      }

      current_byte_in_word = size;
    }
    else
    {
      if(last_type_kind == Type::TypeID::FloatTyID && kind == Type::TypeID::FloatTyID)
      {
        lowered[current_word] = unwrap(c->f64);
      }
      else
      {
        lowered[current_word] =
          get_type_from_size(c, next_power_of_2(next_byte_in_word));
      }

      current_byte_in_word = next_byte_in_word;
    }

    last_type_kind = kind;

    if(next_byte_in_word >= bytes_per_word)
    {
      current_word++;
      current_byte_in_word = 0;
    }
  }
}


static Type* generate_flattened_type(compile_t* c, const vector<Type*>& lowered)
{
  return StructType::get(*unwrap(c->context), lowered);
}


static Type* lower_riscv64_param_value_from_structure_type(compile_t* c, LoweringObject& lowering_object,
  reach_type_t* pt)
{
  Type* ret = nullptr;
  char* triple = c->opt->triple;

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  StructType* s = unwrap<StructType>(p_t->structure);

  if(p_t->abi_size <= 16)
  {
    unsigned int num_elem = s->getNumElements();

    bool contain_array = false;
    for(unsigned int i = 0; i < num_elem; i++)
    {
      if(is_c_fixed_sized_array(pt->fields[i].ast))
      {
        contain_array = true;
        break;
      }
    }

    if(num_elem <= 2 && !contain_array)
    {
      if(num_elem == 1)
      {
        Type* elem_type = s->getTypeAtIndex(0U);
        Type::TypeID kind = elem_type->getTypeID();
        if(kind == Type::TypeID::IntegerTyID)
        {
          ret = unwrap(c->i64);
        }
        else
        {
          if(kind == Type::TypeID::FloatTyID || kind == Type::TypeID::DoubleTyID)
          {
            lowering_object.riscv.num_fp_regs_in_use++;
          }
          ret = elem_type;
        }
      }
      else
      {
        size_t contain_float = 0;
        for(unsigned int i = 0; i < num_elem; i++)
        {
          Type* elem_type = s->getTypeAtIndex(i);
          Type::TypeID kind = elem_type->getTypeID();
          if(kind == Type::TypeID::FloatTyID || kind == Type::TypeID::DoubleTyID)
          {
            contain_float++;
          }
        }

        if((contain_float != 0) &&
           (contain_float + lowering_object.riscv.num_fp_regs_in_use <= 8))
        {
          lowering_object.riscv.num_fp_regs_in_use += contain_float;
          ret = unwrap(p_t->structure);
        }
      }
    }

    if(ret == nullptr)
    {
      if(p_t->abi_size <= 8)
      {
        ret = unwrap(c->i64);
      }
      else
      {
        ret = ArrayType::get(unwrap(c->i64), 2);
      }
    }
  }
  else
  {
    ret = unwrap(p_t->use_type);
  }

  pony_assert(ret != nullptr);

  return ret;
}


static void count_registers(compile_t* c, LoweringObject& lowering_object, Type* type)
{
  char* triple = c->opt->triple;

  if(target_is_riscv(triple))
  {
    if(target_is_lp64(triple))
    {
      Type::TypeID kind = type->getTypeID();

      if(kind == Type::TypeID::FloatTyID || kind == Type::TypeID::DoubleTyID)
      {
        lowering_object.riscv.num_fp_regs_in_use++;
      }
    }
  }
}


extern "C" LoweringObject init_lowering_object(compile_t* c)
{
  char* triple = c->opt->triple;

  LoweringObject ret;
  memset(&ret, 0, sizeof(ret));

  return ret;
}


extern "C" LLVMTypeRef lower_param(compile_t* c, LoweringObject* lowering_object,
  reach_type_t* pt, bool pass_by_value)
{
  Type* ret = nullptr;
  char* triple = c->opt->triple;

  compile_type_t* p_t = (compile_type_t*)pt->c_type;

  auto target_data = unwrap(c->target_data);

  // If no lowering is needed then just return with the same pointer to structure type
  if(!pass_by_value || !is_param_value_lowering_needed(c, p_t))
  {
    count_registers(c, *lowering_object, unwrap(p_t->use_type));
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

        vector<Type*> lowered;
        lower_structure_x86_64_systemv(c, dyn_cast<StructType>(unwrap(p_t->structure)),
          ptr_size, lowered);
        ret = generate_flattened_type(c, lowered);
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
      Type* hfa_type = nullptr;
      size_t num_elem = 0;

      if(get_hfa_from_structure_aarch64(unwrap<StructType>(p_t->structure), hfa_type, num_elem))
      {
        ret = ArrayType::get(hfa_type, num_elem);
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
  else if(target_is_riscv(triple))
  {
    if(target_is_lp64(triple))
    {
      ret = lower_riscv64_param_value_from_structure_type(c, *lowering_object, pt);
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

        vector<Type*> lowered;
        lower_structure_x86_64_systemv(c, dyn_cast<StructType>(unwrap(p_t->structure)),
          ptr_size, lowered);
        ret = generate_flattened_type(c, lowered);
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
      Type* hfa_type = nullptr;
      size_t num_elem = 0;

      if(get_hfa_from_structure_aarch64(unwrap<StructType>(p_t->structure), hfa_type, num_elem))
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
  else if(target_is_riscv(triple))
  {
    if(target_is_lp64(triple))
    {
      LoweringObject lo = init_lowering_object(c);
      ret = lower_riscv64_param_value_from_structure_type(c, lo, pt);
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
      Type* hfa_type = nullptr;
      size_t num_elem = 0;

      bool is_hfa = get_hfa_from_structure_aarch64(unwrap<StructType>(p_c_t->structure),
        hfa_type, num_elem);

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
  else if(target_is_riscv(triple))
  {
    if(target_is_lp64(triple))
    {
      if(p_c_t->abi_size <= 16)
      {
        ret = copy_from_ptr_to_value_zero_extend(c, unwrap(ptr), unwrap(param_type), p_c_t);
      }
      else
      {
        AllocaInst* caller_copy = builder->CreateAlloca(unwrap(p_c_t->structure), nullptr, "");
        ConstantInt* cpy_size = ConstantInt::get(unwrap<IntegerType>(c->intptr), p_c_t->abi_size);
        gencall_memcpy(c, wrap(caller_copy), ptr, wrap(cpy_size));
        ret = caller_copy;
      }
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
          (target_is_x86(triple) && target_is_linux(triple) && target_is_lp64(triple)) ||
          (target_is_riscv(triple) && target_is_lp64(triple)))
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
      Type* hfa_type = nullptr;
      size_t num_elem = 0;

      bool is_hfa = get_hfa_from_structure_aarch64(unwrap<StructType>(p_c_t->structure),
        hfa_type, num_elem);

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
  else if(target_is_riscv(triple) && target_is_lp64(triple))
  {
    if(p_c_t->abi_size <= 16)
    {
      builder->CreateStore(unwrap(param_value), unwrap(dest_ptr));
    }
    else
    {
      ConstantInt* cpy_size = ConstantInt::get(unwrap<IntegerType>(c->intptr), p_c_t->abi_size);
      gencall_memcpy(c, dest_ptr, param_value, wrap(cpy_size));
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
  auto target_data = unwrap(c->target_data);

  if(target_is_x86(triple) && target_is_windows(triple) &&
     (target_is_llp64(triple) || target_is_ilp32(triple)))
  {
    builder->CreateStore(unwrap(return_value), unwrap(dest_ptr));
  }
  else if(target_is_arm(triple) ||
          (target_is_x86(triple) && target_is_linux(triple) && target_is_lp64(triple)) ||
          (target_is_riscv(triple) && target_is_lp64(triple)))
  {
    Type* return_type = unwrap(return_value)->getType();
    if(p_c_t->abi_size == (size_t)target_data->getTypeAllocSize(return_type))
    {
      builder->CreateStore(unwrap(return_value), unwrap(dest_ptr));
    }
    else
    {
      AllocaInst* tmp_array = builder->CreateAlloca(return_type, nullptr, "");
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
  // Aarch64 amd RISC-V doesn't use byval
  if((target_is_arm(c->opt->triple) && target_is_lp64(c->opt->triple)) ||
     (target_is_riscv(c->opt->triple) && target_is_lp64(c->opt->triple)))
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
  // Aarch64 and RISC-V doesn't use byval
  if((target_is_arm(c->opt->triple) && target_is_lp64(c->opt->triple)) ||
     (target_is_riscv(c->opt->triple) && target_is_lp64(c->opt->triple)))
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