use "lib:ffi-pass-by-value-additional"
use "collections"

use @pony_exitcode[None](code: I32)
use @exit[None](status: I32)

use @FFI_clobber_small[None](x: \byval\ S1Small, y: \byval\ S1Small)
use @FFI_clobber_large[None](x: \byval\ S1Large, y: \byval\ S1Large)
use @FFI_add_small[\byval\ S1Small](x: \byval\ S1Small, y: \byval\ S1Small)
use @FFI_add_large[\byval\ S1Large](x: \byval\ S1Large, y: \byval\ S1Large)

use @get_FFI_clobber_small[@{(\byval\ S1Small, \byval\ S1Small)}]()
use @get_FFI_clobber_large[@{(\byval\ S1Large, \byval\ S1Large)}]()
use @get_FFI_add_small[@{(\byval\ S1Small, \byval\ S1Small): \byval\ S1Small}]()
use @get_FFI_add_large[@{(\byval\ S1Large, \byval\ S1Large): \byval\ S1Large}]()

use @FFI_Test_1_int8_t[\byval\ S1Member[I8]](s1: \byval\ S1Member[I8], s2: \byval\ S1Member[I8])
use @FFI_Test_1_int16_t[\byval\ S1Member[I16]](s1: \byval\ S1Member[I16], s2: \byval\ S1Member[I16])
use @FFI_Test_1_int32_t[\byval\ S1Member[I32]](s1: \byval\ S1Member[I32], s2: \byval\ S1Member[I32])
use @FFI_Test_1_int64_t[\byval\ S1Member[I64]](s1: \byval\ S1Member[I64], s2: \byval\ S1Member[I64])
use @FFI_Test_1_float[\byval\ S1Member[F32]](s1: \byval\ S1Member[F32], s2: \byval\ S1Member[F32])
use @FFI_Test_1_double[\byval\ S1Member[F64]](s1: \byval\ S1Member[F64], s2: \byval\ S1Member[F64])
use @FFI_Test_1_Pointer[\byval\ S1Member[C]](s1: \byval\ S1Member[C], s2: \byval\ S1Member[C])

use @FFI_Test_2_int8_t_int8_t[\byval\ S2Member[I8, I8]](s1: \byval\ S2Member[I8, I8], s2: \byval\ S2Member[I8, I8])
use @FFI_Test_2_int8_t_int32_t[\byval\ S2Member[I8, I32]](s1: \byval\ S2Member[I8, I32], s2: \byval\ S2Member[I8, I32])
use @FFI_Test_2_int32_t_float[\byval\ S2Member[I32, F32]](s1: \byval\ S2Member[I32, F32], s2: \byval\ S2Member[I32, F32])
use @FFI_Test_2_int64_t_double[\byval\ S2Member[I64, F64]](s1: \byval\ S2Member[I64, F64], s2: \byval\ S2Member[I64, F64])
use @FFI_Test_2_int32_t_int32_t[\byval\ S2Member[I32, I32]](s1: \byval\ S2Member[I32, I32], s2: \byval\ S2Member[I32, I32])
use @FFI_Test_2_int64_t_int64_t[\byval\ S2Member[I64, I64]](s1: \byval\ S2Member[I64, I64], s2: \byval\ S2Member[I64, I64])
use @FFI_Test_2_float_float[\byval\ S2Member[F32, F32]](s1: \byval\ S2Member[F32, F32], s2: \byval\ S2Member[F32, F32])
use @FFI_Test_2_double_double[\byval\ S2Member[F64, F64]](s1: \byval\ S2Member[F64, F64], s2: \byval\ S2Member[F64, F64])
use @FFI_Test_2_int8_t_Pointer[\byval\ S2Member[I8, C]](s1: \byval\ S2Member[I8, C], s2: \byval\ S2Member[I8, C])
use @FFI_Test_2_Pointer_int32_t[\byval\ S2Member[C, I32]](s1: \byval\ S2Member[C, I32], s2: \byval\ S2Member[C, I32])

use @FFI_Test_3_int32_t_int32_t_double[\byval\ S3Member[I32, I32, F64]](s1: \byval\ S3Member[I32, I32, F64], s2: \byval\ S3Member[I32, I32, F64])
use @FFI_Test_3_float_float_int64_t[\byval\ S3Member[F32, F32, I64]](s1: \byval\ S3Member[F32, F32, I64], s2: \byval\ S3Member[F32, F32, I64])
use @FFI_Test_3_int32_t_float_int64_t[\byval\ S3Member[I32, F32, I64]](s1: \byval\ S3Member[I32, F32, I64], s2: \byval\ S3Member[I32, F32, I64])
use @FFI_Test_3_double_int32_t_int32_t[\byval\ S3Member[F64, I32, I32]](s1: \byval\ S3Member[F64, I32, I32], s2: \byval\ S3Member[F64, I32, I32])
use @FFI_Test_3_float_float_float[\byval\ S3Member[F32, F32, F32]](s1: \byval\ S3Member[F32, F32, F32], s2: \byval\ S3Member[F32, F32, F32])
use @FFI_Test_3_double_double_double[\byval\ S3Member[F64, F64, F64]](s1: \byval\ S3Member[F64, F64, F64], s2: \byval\ S3Member[F64, F64, F64])
use @FFI_Test_3_double_Pointer_double[\byval\ S3Member[F64, C, F64]](s1: \byval\ S3Member[F64, C, F64], s2: \byval\ S3Member[F64, C, F64])
use @FFI_Test_3_int32_t_Pointer_int64_t[\byval\ S3Member[I32, C, I64]](s1: \byval\ S3Member[I32, C, I64], s2: \byval\ S3Member[I32, C, I64])

use @FFI_Test_4_int32_t_int32_t_int32_t_int32_t[\byval\ S4Member[I32, I32, I32, I32]](s1: \byval\ S4Member[I32, I32, I32, I32], s2: \byval\ S4Member[I32, I32, I32, I32])
use @FFI_Test_4_int64_t_int64_t_int64_t_int64_t[\byval\ S4Member[I64, I64, I64, I64]](s1: \byval\ S4Member[I64, I64, I64, I64], s2: \byval\ S4Member[I64, I64, I64, I64])
use @FFI_Test_4_float_float_float_float[\byval\ S4Member[F32, F32, F32, F32]](s1: \byval\ S4Member[F32, F32, F32, F32], s2: \byval\ S4Member[F32, F32, F32, F32])
use @FFI_Test_4_double_double_double_double[\byval\ S4Member[F64, F64, F64, F64]](s1: \byval\ S4Member[F64, F64, F64, F64], s2: \byval\ S4Member[F64, F64, F64, F64])
use @FFI_Test_4_Pointer_Pointer_Pointer_Pointer[\byval\ S4Member[C, C, C, C]](s1: \byval\ S4Member[C, C, C, C], s2: \byval\ S4Member[C, C, C, C])
use @FFI_Test_4_int64_t_Pointer_int64_t_int64_t[\byval\ S4Member[I64, C, I64, I64]](s1: \byval\ S4Member[I64, C, I64, I64], s2: \byval\ S4Member[I64, C, I64, I64])

use @FFI_Test_2_12_int32_t_int32_t[\byval\ S2Member[I32, I32]](s1: \byval\ S2Member[I32, I32], s2: \byval\ S2Member[I32, I32],
  s3: \byval\ S2Member[I32, I32], s4: \byval\ S2Member[I32, I32],
  s5: \byval\ S2Member[I32, I32], s6: \byval\ S2Member[I32, I32],
  s7: \byval\ S2Member[I32, I32], s8: \byval\ S2Member[I32, I32],
  s9: \byval\ S2Member[I32, I32], s10: \byval\ S2Member[I32, I32],
  s11: \byval\ S2Member[I32, I32], s12: \byval\ S2Member[I32, I32])

use @FFI_Test_2_12_int64_t_int64_t[\byval\ S2Member[I64, I64]](s1: \byval\ S2Member[I64, I64], s2: \byval\ S2Member[I64, I64],
  s3: \byval\ S2Member[I64, I64], s4: \byval\ S2Member[I64, I64],
  s5: \byval\ S2Member[I64, I64], s6: \byval\ S2Member[I64, I64],
  s7: \byval\ S2Member[I64, I64], s8: \byval\ S2Member[I64, I64],
  s9: \byval\ S2Member[I64, I64], s10: \byval\ S2Member[I64, I64],
  s11: \byval\ S2Member[I64, I64], s12: \byval\ S2Member[I64, I64])

use @FFI_Test_2_12_float_float[\byval\ S2Member[F32, F32]](s1: \byval\ S2Member[F32, F32], s2: \byval\ S2Member[F32, F32],
  s3: \byval\ S2Member[F32, F32], s4: \byval\ S2Member[F32, F32],
  s5: \byval\ S2Member[F32, F32], s6: \byval\ S2Member[F32, F32],
  s7: \byval\ S2Member[F32, F32], s8: \byval\ S2Member[F32, F32],
  s9: \byval\ S2Member[F32, F32], s10: \byval\ S2Member[F32, F32],
  s11: \byval\ S2Member[F32, F32], s12: \byval\ S2Member[F32, F32])

use @FFI_Test_2_12_double_double[\byval\ S2Member[F64, F64]](s1: \byval\ S2Member[F64, F64], s2: \byval\ S2Member[F64, F64],
  s3: \byval\ S2Member[F64, F64], s4: \byval\ S2Member[F64, F64],
  s5: \byval\ S2Member[F64, F64], s6: \byval\ S2Member[F64, F64],
  s7: \byval\ S2Member[F64, F64], s8: \byval\ S2Member[F64, F64],
  s9: \byval\ S2Member[F64, F64], s10: \byval\ S2Member[F64, F64],
  s11: \byval\ S2Member[F64, F64], s12: \byval\ S2Member[F64, F64])

class C

struct S1Member[T1]
  var a: T1

  new create(a': I32) =>
    iftype T1 <: C ref then
      a = Pointer[C].from_usize(a'.usize_unsafe()).to_reftype_no_check()
    elseif T1 <: (Real[T1] val & Number) then
      a = T1.from[I32](a')
    else
      compile_error "type not supported by S1Member"
    end

  fun clone(): S1Member[T1]^ =>
    S1Member[T1](apply(0))

  fun num_fields(): USize => 1

  fun apply(i: USize): I32 =>
    match i
    | 0 =>
      iftype T1 <: C ref then
        Pointer[C ref].from_any[this->T1](a).usize().i32_unsafe()
      elseif T1 <: (Real[T1] val & Number) then
        a.i32_unsafe()
      else
        compile_error "type not supported by S1Member"
      end
    else
      0
    end

struct S2Member[T1, T2]
  var a: T1
  var b: T2

  new create(a': I32, b': I32) =>
    iftype T1 <: C ref then
      a = Pointer[C].from_usize(a'.usize_unsafe()).to_reftype_no_check()
    elseif T1 <: (Real[T1] val & Number) then
      a = T1.from[I32](a')
    else
      compile_error "type not supported by S2Member"
    end

    iftype T2 <: C ref then
      b = Pointer[C].from_usize(b'.usize_unsafe()).to_reftype_no_check()
    elseif T2 <: (Real[T2] val & Number) then
      b = T2.from[I32](b')
    else
      compile_error "type not supported by S2Member"
    end

  fun ref clone(): S2Member[T1, T2]^ =>
    S2Member[T1, T2](apply(0), apply(1))

  fun num_fields(): USize => 2

  fun apply(i: USize): I32 =>
    match i
    | 0 =>
      iftype T1 <: C ref then
        Pointer[C ref].from_any[this->T1](a).usize().i32_unsafe()
      elseif T1 <: (Real[T1] val & Number) then
        a.i32_unsafe()
      else
        compile_error "type not supported by S2Member"
      end
    | 1 =>
      iftype T2 <: C ref then
        Pointer[C ref].from_any[this->T2](b).usize().i32_unsafe()
      elseif T2 <: (Real[T2] val & Number) then
        b.i32_unsafe()
      else
        compile_error "type not supported by S2Member"
      end
    else
      0
    end

struct S3Member[T1, T2, T3]
  var a: T1
  var b: T2
  var c: T3

  new create(a': I32, b': I32, c': I32) =>
    iftype T1 <: C ref then
      a = Pointer[C].from_usize(a'.usize_unsafe()).to_reftype_no_check()
    elseif T1 <: (Real[T1] val & Number) then
      a = T1.from[I32](a')
    else
      compile_error "type not supported by S3Member"
    end

    iftype T2 <: C ref then
      b = Pointer[C].from_usize(b'.usize_unsafe()).to_reftype_no_check()
    elseif T2 <: (Real[T2] val & Number) then
      b = T2.from[I32](b')
    else
      compile_error "type not supported by S3Member"
    end

    iftype T3 <: C ref then
      c = Pointer[C].from_usize(c'.usize_unsafe()).to_reftype_no_check()
    elseif T3 <: (Real[T3] val & Number) then
      c = T3.from[I32](c')
    else
      compile_error "type not supported by S3Member"
    end

  fun ref clone(): S3Member[T1, T2, T3]^ =>
    S3Member[T1, T2, T3](apply(0), apply(1), apply(2))

  fun num_fields(): USize => 3

  fun apply(i: USize): I32 =>
    match i
    | 0 =>
      iftype T1 <: C ref then
        Pointer[C ref].from_any[this->T1](a).usize().i32_unsafe()
      elseif T1 <: (Real[T1] val & Number) then
        a.i32_unsafe()
      else
        compile_error "type not supported by S3Member"
      end
    | 1 =>
      iftype T2 <: C ref then
        Pointer[C ref].from_any[this->T2](b).usize().i32_unsafe()
      elseif T2 <: (Real[T2] val & Number) then
        b.i32_unsafe()
      else
        compile_error "type not supported by S3Member"
      end
    | 2 =>
      iftype T3 <: C ref then
        Pointer[C ref].from_any[this->T3](c).usize().i32_unsafe()
      elseif T3 <: (Real[T3] val & Number) then
        c.i32_unsafe()
      else
        compile_error "type not supported by S3Member"
      end
    else
      0
    end

struct S4Member[T1, T2, T3, T4]
  var a: T1
  var b: T2
  var c: T3
  var d: T4

  new create(a': I32, b': I32, c': I32, d': I32) =>
    iftype T1 <: C ref then
      a = Pointer[C].from_usize(a'.usize_unsafe()).to_reftype_no_check()
    elseif T1 <: (Real[T1] val & Number) then
      a = T1.from[I32](a')
    else
      compile_error "type not supported by S4Member"
    end

    iftype T2 <: C ref then
      b = Pointer[C].from_usize(b'.usize_unsafe()).to_reftype_no_check()
    elseif T2 <: (Real[T2] val & Number) then
      b = T2.from[I32](b')
    else
      compile_error "type not supported by S4Member"
    end

    iftype T3 <: C ref then
      c = Pointer[C].from_usize(c'.usize_unsafe()).to_reftype_no_check()
    elseif T3 <: (Real[T3] val & Number) then
      c = T3.from[I32](c')
    else
      compile_error "type not supported by S4Member"
    end

    iftype T4 <: C ref then
      d = Pointer[C].from_usize(d'.usize_unsafe()).to_reftype_no_check()
    elseif T4 <: (Real[T4] val & Number) then
      d = T4.from[I32](d')
    else
      compile_error "type not supported by S4Member"
    end

  fun ref clone(): S4Member[T1, T2, T3, T4]^ =>
    S4Member[T1, T2, T3, T4](apply(0), apply(1), apply(2), apply(3))

  fun num_fields(): USize => 4

  fun apply(i: USize): I32 =>
    match i
    | 0 =>
      iftype T1 <: C ref then
        Pointer[C ref].from_any[this->T1](a).usize().i32_unsafe()
      elseif T1 <: (Real[T1] val & Number) then
        a.i32_unsafe()
      else
        compile_error "type not supported by S4Member"
      end
    | 1 =>
      iftype T2 <: C ref then
        Pointer[C ref].from_any[this->T2](b).usize().i32_unsafe()
      elseif T2 <: (Real[T2] val & Number) then
        b.i32_unsafe()
      else
        compile_error "type not supported by S4Member"
      end
    | 2 =>
      iftype T3 <: C ref then
        Pointer[C ref].from_any[this->T3](c).usize().i32_unsafe()
      elseif T3 <: (Real[T3] val & Number) then
        c.i32_unsafe()
      else
        compile_error "type not supported by S4Member"
      end
    | 3 =>
      iftype T4 <: C ref then
        Pointer[C ref].from_any[this->T4](d).usize().i32_unsafe()
      elseif T4 <: (Real[T4] val & Number) then
        d.i32_unsafe()
      else
        compile_error "type not supported by S4Member"
      end
    else
      0
    end

struct S1Small
  embed ar: CFixedSizedArray[I32, 2]

  new create(v: I32) =>
    ar = CFixedSizedArray[I32, 2](v)

struct S1Large
  embed ar: CFixedSizedArray[I32, 64]

  new create(v: I32) =>
    ar = CFixedSizedArray[I32, 64](v)

actor Main
  new create(env: Env) =>
    var ret = test_small_struct[0]()
    if ret != 0 then
      @pony_exitcode(ret)
      return
    end

    ret = test_large_struct[20]()
    if ret != 0 then
      @pony_exitcode(ret)
      return
    end

    ret = test_small_struct_bare_lambda[40]()
    if ret != 0 then
      @pony_exitcode(ret)
      return
    end

    ret = test_large_struct_bare_lambda[60]()
    if ret != 0 then
      @pony_exitcode(ret)
      return
    end

    test_1_member_struct[I8](
      @{(s1: \byval\ S1Member[I8], s2: \byval\ S1Member[I8]): \byval\ S1Member[I8] =>
        @FFI_Test_1_int8_t(s1, s2)
      })
    test_1_member_struct[I16](
      @{(s1: \byval\ S1Member[I16], s2: \byval\ S1Member[I16]): \byval\ S1Member[I16] =>
        @FFI_Test_1_int16_t(s1, s2)
      })
    test_1_member_struct[I32](
      @{(s1: \byval\ S1Member[I32], s2: \byval\ S1Member[I32]): \byval\ S1Member[I32] =>
        @FFI_Test_1_int32_t(s1, s2)
      })
    test_1_member_struct[I64](
      @{(s1: \byval\ S1Member[I64], s2: \byval\ S1Member[I64]): \byval\ S1Member[I64] =>
        @FFI_Test_1_int64_t(s1, s2)
      })
    test_1_member_struct[F32](
      @{(s1: \byval\ S1Member[F32], s2: \byval\ S1Member[F32]): \byval\ S1Member[F32] =>
        @FFI_Test_1_float(s1, s2)
      })
    test_1_member_struct[F64](
      @{(s1: \byval\ S1Member[F64], s2: \byval\ S1Member[F64]): \byval\ S1Member[F64] =>
        @FFI_Test_1_double(s1, s2)
      })
    test_1_member_struct[C](
      @{(s1: \byval\ S1Member[C], s2: \byval\ S1Member[C]): \byval\ S1Member[C] =>
        @FFI_Test_1_Pointer(s1, s2)
      })

    test_2_member_struct[I8, I8](
      @{(s1: \byval\ S2Member[I8, I8], s2: \byval\ S2Member[I8, I8]): \byval\ S2Member[I8, I8] =>
        @FFI_Test_2_int8_t_int8_t(s1, s2)
      })
    test_2_member_struct[I8, I32](
      @{(s1: \byval\ S2Member[I8, I32], s2: \byval\ S2Member[I8, I32]): \byval\ S2Member[I8, I32] =>
        @FFI_Test_2_int8_t_int32_t(s1, s2)
      })
    test_2_member_struct[I32, F32](
      @{(s1: \byval\ S2Member[I32, F32], s2: \byval\ S2Member[I32, F32]): \byval\ S2Member[I32, F32] =>
        @FFI_Test_2_int32_t_float(s1, s2)
      })
    test_2_member_struct[I64, F64](
      @{(s1: \byval\ S2Member[I64, F64], s2: \byval\ S2Member[I64, F64]): \byval\ S2Member[I64, F64] =>
        @FFI_Test_2_int64_t_double(s1, s2)
      })
    test_2_member_struct[I32, I32](
      @{(s1: \byval\ S2Member[I32, I32], s2: \byval\ S2Member[I32, I32]): \byval\ S2Member[I32, I32] =>
        @FFI_Test_2_int32_t_int32_t(s1, s2)
      })
    test_2_member_struct[I64, I64](
      @{(s1: \byval\ S2Member[I64, I64], s2: \byval\ S2Member[I64, I64]): \byval\ S2Member[I64, I64] =>
        @FFI_Test_2_int64_t_int64_t(s1, s2)
      })
    test_2_member_struct[F32, F32](
      @{(s1: \byval\ S2Member[F32, F32], s2: \byval\ S2Member[F32, F32]): \byval\ S2Member[F32, F32] =>
        @FFI_Test_2_float_float(s1, s2)
      })
    test_2_member_struct[F64, F64](
      @{(s1: \byval\ S2Member[F64, F64], s2: \byval\ S2Member[F64, F64]): \byval\ S2Member[F64, F64] =>
        @FFI_Test_2_double_double(s1, s2)
      })
    test_2_member_struct[I8, C](
      @{(s1: \byval\ S2Member[I8, C], s2: \byval\ S2Member[I8, C]): \byval\ S2Member[I8, C] =>
        @FFI_Test_2_int8_t_Pointer(s1, s2)
      })
    test_2_member_struct[C, I32](
      @{(s1: \byval\ S2Member[C, I32], s2: \byval\ S2Member[C, I32]): \byval\ S2Member[C, I32] =>
        @FFI_Test_2_Pointer_int32_t(s1, s2)
      })

    test_3_member_struct[I32, I32, F64](
      @{(s1: \byval\ S3Member[I32, I32, F64], s2: \byval\ S3Member[I32, I32, F64]): \byval\ S3Member[I32, I32, F64] =>
        @FFI_Test_3_int32_t_int32_t_double(s1, s2)
      })
    test_3_member_struct[F32, F32, I64](
      @{(s1: \byval\ S3Member[F32, F32, I64], s2: \byval\ S3Member[F32, F32, I64]): \byval\ S3Member[F32, F32, I64] =>
        @FFI_Test_3_float_float_int64_t(s1, s2)
      })
    test_3_member_struct[I32, F32, I64](
      @{(s1: \byval\ S3Member[I32, F32, I64], s2: \byval\ S3Member[I32, F32, I64]): \byval\ S3Member[I32, F32, I64] =>
        @FFI_Test_3_int32_t_float_int64_t(s1, s2)
      })
    test_3_member_struct[F64, I32, I32](
      @{(s1: \byval\ S3Member[F64, I32, I32], s2: \byval\ S3Member[F64, I32, I32]): \byval\ S3Member[F64, I32, I32] =>
        @FFI_Test_3_double_int32_t_int32_t(s1, s2)
      })
    test_3_member_struct[F32, F32, F32](
      @{(s1: \byval\ S3Member[F32, F32, F32], s2: \byval\ S3Member[F32, F32, F32]): \byval\ S3Member[F32, F32, F32] =>
        @FFI_Test_3_float_float_float(s1, s2)
      })
    test_3_member_struct[F64, F64, F64](
      @{(s1: \byval\ S3Member[F64, F64, F64], s2: \byval\ S3Member[F64, F64, F64]): \byval\ S3Member[F64, F64, F64] =>
        @FFI_Test_3_double_double_double(s1, s2)
      })
    test_3_member_struct[F64, C, F64](
      @{(s1: \byval\ S3Member[F64, C, F64], s2: \byval\ S3Member[F64, C, F64]): \byval\ S3Member[F64, C, F64] =>
        @FFI_Test_3_double_Pointer_double(s1, s2)
      })
    test_3_member_struct[I32, C, I64](
      @{(s1: \byval\ S3Member[I32, C, I64], s2: \byval\ S3Member[I32, C, I64]): \byval\ S3Member[I32, C, I64] =>
        @FFI_Test_3_int32_t_Pointer_int64_t(s1, s2)
      })

    test_4_member_struct[I32, I32, I32, I32](
      @{(s1: \byval\ S4Member[I32, I32, I32, I32], s2: \byval\ S4Member[I32, I32, I32, I32]): \byval\ S4Member[I32, I32, I32, I32] =>
        @FFI_Test_4_int32_t_int32_t_int32_t_int32_t(s1, s2)
      })
    test_4_member_struct[I64, I64, I64, I64](
      @{(s1: \byval\ S4Member[I64, I64, I64, I64], s2: \byval\ S4Member[I64, I64, I64, I64]): \byval\ S4Member[I64, I64, I64, I64] =>
        @FFI_Test_4_int64_t_int64_t_int64_t_int64_t(s1, s2)
      })
    test_4_member_struct[F32, F32, F32, F32](
      @{(s1: \byval\ S4Member[F32, F32, F32, F32], s2: \byval\ S4Member[F32, F32, F32, F32]): \byval\ S4Member[F32, F32, F32, F32] =>
        @FFI_Test_4_float_float_float_float(s1, s2)
      })
    test_4_member_struct[F64, F64, F64, F64](
      @{(s1: \byval\ S4Member[F64, F64, F64, F64], s2: \byval\ S4Member[F64, F64, F64, F64]): \byval\ S4Member[F64, F64, F64, F64] =>
        @FFI_Test_4_double_double_double_double(s1, s2)
      })
    test_4_member_struct[C, C, C, C](
      @{(s1: \byval\ S4Member[C, C, C, C], s2: \byval\ S4Member[C, C, C, C]): \byval\ S4Member[C, C, C, C] =>
        @FFI_Test_4_Pointer_Pointer_Pointer_Pointer(s1, s2)
      })
    test_4_member_struct[I64, C, I64, I64](
      @{(s1: \byval\ S4Member[I64, C, I64, I64], s2: \byval\ S4Member[I64, C, I64, I64]): \byval\ S4Member[I64, C, I64, I64] =>
        @FFI_Test_4_int64_t_Pointer_int64_t_int64_t(s1, s2)
      })

    test_2_member_struct_12_params[I32, I32](
      @{(s1: \byval\ S2Member[I32, I32], s2: \byval\ S2Member[I32, I32],
        s3: \byval\ S2Member[I32, I32], s4: \byval\ S2Member[I32, I32],
        s5: \byval\ S2Member[I32, I32], s6: \byval\ S2Member[I32, I32],
        s7: \byval\ S2Member[I32, I32], s8: \byval\ S2Member[I32, I32],
        s9: \byval\ S2Member[I32, I32], s10: \byval\ S2Member[I32, I32],
        s11: \byval\ S2Member[I32, I32], s12: \byval\ S2Member[I32, I32]
      ): \byval\ S2Member[I32, I32] =>
        @FFI_Test_2_12_int32_t_int32_t(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)
      })

    test_2_member_struct_12_params[I64, I64](
      @{(s1: \byval\ S2Member[I64, I64], s2: \byval\ S2Member[I64, I64],
        s3: \byval\ S2Member[I64, I64], s4: \byval\ S2Member[I64, I64],
        s5: \byval\ S2Member[I64, I64], s6: \byval\ S2Member[I64, I64],
        s7: \byval\ S2Member[I64, I64], s8: \byval\ S2Member[I64, I64],
        s9: \byval\ S2Member[I64, I64], s10: \byval\ S2Member[I64, I64],
        s11: \byval\ S2Member[I64, I64], s12: \byval\ S2Member[I64, I64]
      ): \byval\ S2Member[I64, I64] =>
        @FFI_Test_2_12_int64_t_int64_t(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)
      })

   test_2_member_struct_12_params[F32, F32](
      @{(s1: \byval\ S2Member[F32, F32], s2: \byval\ S2Member[F32, F32],
        s3: \byval\ S2Member[F32, F32], s4: \byval\ S2Member[F32, F32],
        s5: \byval\ S2Member[F32, F32], s6: \byval\ S2Member[F32, F32],
        s7: \byval\ S2Member[F32, F32], s8: \byval\ S2Member[F32, F32],
        s9: \byval\ S2Member[F32, F32], s10: \byval\ S2Member[F32, F32],
        s11: \byval\ S2Member[F32, F32], s12: \byval\ S2Member[F32, F32]
      ): \byval\ S2Member[F32, F32] =>
        @FFI_Test_2_12_float_float(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)
      })

    test_2_member_struct_12_params[F64, F64](
      @{(s1: \byval\ S2Member[F64, F64], s2: \byval\ S2Member[F64, F64],
        s3: \byval\ S2Member[F64, F64], s4: \byval\ S2Member[F64, F64],
        s5: \byval\ S2Member[F64, F64], s6: \byval\ S2Member[F64, F64],
        s7: \byval\ S2Member[F64, F64], s8: \byval\ S2Member[F64, F64],
        s9: \byval\ S2Member[F64, F64], s10: \byval\ S2Member[F64, F64],
        s11: \byval\ S2Member[F64, F64], s12: \byval\ S2Member[F64, F64]
      ): \byval\ S2Member[F64, F64] =>
        @FFI_Test_2_12_double_double(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)
      })

  fun test_small_struct[ret_add: I32](): I32 =>
    //============== Test small struct

    var x = S1Small(100)
    var y = S1Small(200)

    // Check that passing by value retains the data at call site
    @FFI_clobber_small(x, y)

    if not check_small(x, 100) then
      return 1 + ret_add
    end

    if not check_small(y, 200) then
      return 2 + ret_add
    end
    
    // Test return with new allocation
    let z = @FFI_add_small(x, y)

    if not check_small(z, 300) then
      return 3 + ret_add
    end

    // Test return with already allocated
    var z2 = S1Small(0)
    z2 = @FFI_add_small(x, y)

    if not check_small(z2, 300) then
      return 4 + ret_add
    end

    // Test return but without any assignment, shouldn't crash
    @FFI_add_small(x, y)

    0

  fun test_large_struct[ret_add: I32](): I32 =>
    //============== Test large struct

    var x = S1Large(100)
    var y = S1Large(200)

    // Check that passing by value retains the data at call site
    @FFI_clobber_large(x, y)

    if not check_large(x, 100) then
      return 1 + ret_add
    end

    if not check_large(y, 200) then
      return 2 + ret_add
    end
    
    // Test return with new allocation
    let z = @FFI_add_large(x, y)

    if not check_large(z, 300) then
      return 3 + ret_add
    end

    // Test return with already allocated
    var z2 = S1Large(0)
    z2 = @FFI_add_large(x, y)

    if not check_large(z2, 300) then
      return 4 + ret_add
    end

    // Test return but without any assignment, shouldn't crash
    @FFI_add_large(x, y)

    0

  fun test_small_struct_bare_lambda[ret_add: I32](): I32 =>
    //============== Test small struct bare lambda

    let clobber_lambda = @get_FFI_clobber_small()
    let add_lambda = @get_FFI_add_small()

    var x = S1Small(100)
    var y = S1Small(200)

    // Check that passing by value retains the data at call site
    clobber_lambda(x, y)

    if not check_small(x, 100) then
      return 1 + ret_add
    end

    if not check_small(y, 200) then
      return 2 + ret_add
    end

    // Test return with new allocation
    let z = add_lambda(x, y)

    if not check_small(z, 300) then
      return 3 + ret_add
    end

    // Test return with already allocated
    var z2 = S1Small(0)
    z2 = add_lambda(x, y)

    if not check_small(z2, 300) then
      return 4 + ret_add
    end

    // Test return but without any assignment, shouldn't crash
    add_lambda(x, y)

    0

  fun test_large_struct_bare_lambda[ret_add: I32](): I32 =>
    //============== Test large struct bare lambda

    let clobber_lambda = @get_FFI_clobber_large()
    let add_lambda = @get_FFI_add_large()

    var x = S1Large(100)
    var y = S1Large(200)

    // Check that passing by value retains the data at call site
    clobber_lambda(x, y)

    if not check_large(x, 100) then
      return 1 + ret_add
    end

    if not check_large(y, 200) then
      return 2 + ret_add
    end

    // Test return with new allocation
    let z = add_lambda(x, y)

    if not check_large(z, 300) then
      return 3 + ret_add
    end

    // Test return with already allocated
    var z2 = S1Large(0)
    z2 = add_lambda(x, y)

    if not check_large(z2, 300) then
      return 4 + ret_add
    end

    // Test return but without any assignment, shouldn't crash
    add_lambda(x, y)

    0

  fun check_small(s: S1Small, v: I32): Bool =>
    for i in s.ar.values() do
      if i != v then
        return false
      end
    end

    true

  fun check_large(s: S1Large, v: I32): Bool =>
    for i in s.ar.values() do
      if i != v then
        return false
      end
    end

    true

  fun test_1_member_struct[T1](ffi_function:
    @{(\byval\ S1Member[T1], \byval\ S1Member[T1]): \byval\ S1Member[T1]}) =>

    let s1 = S1Member[T1](1)
    let s2 = S1Member[T1](2)

    let old_s1 = s1.clone()
    let old_s2 = s2.clone()

    let s_ret = ffi_function(s1, s2)

    for i in Range(0, s1.num_fields()) do
      if s1(i) != old_s1(i) then
       @exit(1)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s2(i) != old_s2(i) then
       @exit(2)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s_ret(i) != (old_s1(i) + old_s2(i)) then
        @exit(3)
      end
    end

  fun test_2_member_struct[T1, T2](ffi_function:
    @{(\byval\ S2Member[T1, T2], \byval\ S2Member[T1, T2]): \byval\ S2Member[T1, T2]}) =>

    var s1: S2Member[T1, T2] = S2Member[T1, T2](1, 1)
    var s2: S2Member[T1, T2] = S2Member[T1, T2](2, 2)

    let old_s1 = s1.clone()
    let old_s2 = s2.clone()

    let s_ret = ffi_function(s1, s2)

    for i in Range(0, s1.num_fields()) do
      if s1(i) != old_s1(i) then
       @exit(1)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s2(i) != old_s2(i) then
       @exit(2)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s_ret(i) != (old_s1(i) + old_s2(i)) then
        @exit(3)
      end
    end

fun test_3_member_struct[T1, T2, T3](ffi_function:
    @{(\byval\ S3Member[T1, T2, T3], \byval\ S3Member[T1, T2, T3]): \byval\ S3Member[T1, T2, T3]}) =>

    var s1: S3Member[T1, T2, T3] = S3Member[T1, T2, T3](1, 1, 1)
    var s2: S3Member[T1, T2, T3] = S3Member[T1, T2, T3](2, 2, 2)

    let old_s1 = s1.clone()
    let old_s2 = s2.clone()

    let s_ret = ffi_function(s1, s2)

    for i in Range(0, s1.num_fields()) do
      if s1(i) != old_s1(i) then
       @exit(1)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s2(i) != old_s2(i) then
       @exit(2)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s_ret(i) != (old_s1(i) + old_s2(i)) then
        @exit(3)
      end
    end

fun test_4_member_struct[T1, T2, T3, T4](
    ffi_function:
    @{(\byval\ S4Member[T1, T2, T3, T4], \byval\ S4Member[T1, T2, T3, T4]): \byval\ S4Member[T1, T2, T3, T4]}) =>

    var s1: S4Member[T1, T2, T3, T4] = S4Member[T1, T2, T3, T4](1, 1, 1, 1)
    var s2: S4Member[T1, T2, T3, T4] = S4Member[T1, T2, T3, T4](2, 2, 2, 2)

    let old_s1 = s1.clone()
    let old_s2 = s2.clone()

    let s_ret = ffi_function(s1, s2)

    for i in Range(0, s1.num_fields()) do
      if s1(i) != old_s1(i) then
       @exit(1)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s2(i) != old_s2(i) then
       @exit(2)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s_ret(i) != (old_s1(i) + old_s2(i)) then
        @exit(3)
      end
    end

  fun test_2_member_struct_12_params[T1: Real[T1] val, T2: Real[T2] val](
    ffi_function:
    @{(\byval\ S2Member[T1, T2], \byval\ S2Member[T1, T2],
       \byval\ S2Member[T1, T2], \byval\ S2Member[T1, T2],
       \byval\ S2Member[T1, T2], \byval\ S2Member[T1, T2],
       \byval\ S2Member[T1, T2], \byval\ S2Member[T1, T2],
       \byval\ S2Member[T1, T2], \byval\ S2Member[T1, T2],
       \byval\ S2Member[T1, T2], \byval\ S2Member[T1, T2]
       ): \byval\ S2Member[T1, T2]}) =>

    var s1: S2Member[T1, T2] = S2Member[T1, T2](1,1)
    var s2: S2Member[T1, T2] = S2Member[T1, T2](2, 2)
    var s3: S2Member[T1, T2] = S2Member[T1, T2](3, 3)
    var s4: S2Member[T1, T2] = S2Member[T1, T2](4, 4)
    var s5: S2Member[T1, T2] = S2Member[T1, T2](5, 5)
    var s6: S2Member[T1, T2] = S2Member[T1, T2](6, 6)
    var s7: S2Member[T1, T2] = S2Member[T1, T2](7, 7)
    var s8: S2Member[T1, T2] = S2Member[T1, T2](8, 8)
    var s9: S2Member[T1, T2] = S2Member[T1, T2](9, 9)
    var s10: S2Member[T1, T2] = S2Member[T1, T2](10, 10)
    var s11: S2Member[T1, T2] = S2Member[T1, T2](11, 11)
    var s12: S2Member[T1, T2] = S2Member[T1, T2](12, 12)

    let old_s1 = s1.clone()
    let old_s2 = s2.clone()
    let old_s3 = s3.clone()
    let old_s4 = s4.clone()
    let old_s5 = s5.clone()
    let old_s6 = s6.clone()
    let old_s7 = s7.clone()
    let old_s8 = s8.clone()
    let old_s9 = s9.clone()
    let old_s10 = s10.clone()
    let old_s11 = s11.clone()
    let old_s12 = s12.clone()

    let s_ret = ffi_function(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)

    for i in Range(0, s1.num_fields()) do
      if s1(i) != old_s1(i) then
       @exit(1)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s2(i) != old_s2(i) then
       @exit(2)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s3(i) != old_s3(i) then
       @exit(3)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s4(i) != old_s4(i) then
       @exit(4)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s5(i) != old_s5(i) then
       @exit(5)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s6(i) != old_s6(i) then
       @exit(6)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s7(i) != old_s7(i) then
       @exit(7)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s8(i) != old_s8(i) then
       @exit(8)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s9(i) != old_s9(i) then
       @exit(9)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s10(i) != old_s10(i) then
       @exit(10)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s11(i) != old_s11(i) then
       @exit(11)
      end
    end

    for i in Range(0, s1.num_fields()) do
      if s12(i) != old_s12(i) then
       @exit(12)
      end
    end

    let expected_a = old_s1(0) + old_s2(0) + old_s3(0) + old_s4(0) +
                     old_s5(0) + old_s6(0) + old_s7(0) + old_s8(0) +
                     old_s9(0) + old_s10(0) + old_s11(0) + old_s12(0)

    let expected_b = old_s1(1) + old_s2(1) + old_s3(1) + old_s4(1) +
                     old_s5(1) + old_s6(1) + old_s7(1) + old_s8(1) +
                     old_s9(1) + old_s10(1) + old_s11(1) + old_s12(1)

    if (s_ret(0) != expected_a) or (s_ret(1) != expected_b) then
      @exit(13)
    end