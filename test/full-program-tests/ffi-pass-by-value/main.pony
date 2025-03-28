use "lib:ffi-pass-by-value-additional"

use @pony_exitcode[None](code: I32)
use @exit[None](status: I32)

use @FFI_clobber_small[None](x: \passbyvalue\ S1Small, y: \passbyvalue\ S1Small)
use @FFI_clobber_large[None](x: \passbyvalue\ S1Large, y: \passbyvalue\ S1Large)
use @FFI_add_small[\passbyvalue\ S1Small](x: \passbyvalue\ S1Small, y: \passbyvalue\ S1Small)
use @FFI_add_large[\passbyvalue\ S1Large](x: \passbyvalue\ S1Large, y: \passbyvalue\ S1Large)

use @get_FFI_clobber_small[@{(\passbyvalue\ S1Small, \passbyvalue\ S1Small)}]()
use @get_FFI_clobber_large[@{(\passbyvalue\ S1Large, \passbyvalue\ S1Large)}]()
use @get_FFI_add_small[@{(\passbyvalue\ S1Small, \passbyvalue\ S1Small): \passbyvalue\ S1Small}]()
use @get_FFI_add_large[@{(\passbyvalue\ S1Large, \passbyvalue\ S1Large): \passbyvalue\ S1Large}]()

use @FFI_Test_1_int8_t[\passbyvalue\ S1Member[I8]](s1: \passbyvalue\ S1Member[I8], s2: \passbyvalue\ S1Member[I8])
use @FFI_Test_1_int16_t[\passbyvalue\ S1Member[I16]](s1: \passbyvalue\ S1Member[I16], s2: \passbyvalue\ S1Member[I16])
use @FFI_Test_1_int32_t[\passbyvalue\ S1Member[I32]](s1: \passbyvalue\ S1Member[I32], s2: \passbyvalue\ S1Member[I32])
use @FFI_Test_1_int64_t[\passbyvalue\ S1Member[I64]](s1: \passbyvalue\ S1Member[I64], s2: \passbyvalue\ S1Member[I64])
use @FFI_Test_1_float[\passbyvalue\ S1Member[F32]](s1: \passbyvalue\ S1Member[F32], s2: \passbyvalue\ S1Member[F32])
use @FFI_Test_1_double[\passbyvalue\ S1Member[F64]](s1: \passbyvalue\ S1Member[F64], s2: \passbyvalue\ S1Member[F64])

use @FFI_Test_2_int8_t_int8_t[\passbyvalue\ S2Member[I8, I8]](s1: \passbyvalue\ S2Member[I8, I8], s2: \passbyvalue\ S2Member[I8, I8])
use @FFI_Test_2_int8_t_int32_t[\passbyvalue\ S2Member[I8, I32]](s1: \passbyvalue\ S2Member[I8, I32], s2: \passbyvalue\ S2Member[I8, I32])
use @FFI_Test_2_int32_t_float[\passbyvalue\ S2Member[I32, F32]](s1: \passbyvalue\ S2Member[I32, F32], s2: \passbyvalue\ S2Member[I32, F32])
use @FFI_Test_2_int64_t_double[\passbyvalue\ S2Member[I64, F64]](s1: \passbyvalue\ S2Member[I64, F64], s2: \passbyvalue\ S2Member[I64, F64])
use @FFI_Test_2_int32_t_int32_t[\passbyvalue\ S2Member[I32, I32]](s1: \passbyvalue\ S2Member[I32, I32], s2: \passbyvalue\ S2Member[I32, I32])
use @FFI_Test_2_int64_t_int64_t[\passbyvalue\ S2Member[I64, I64]](s1: \passbyvalue\ S2Member[I64, I64], s2: \passbyvalue\ S2Member[I64, I64])
use @FFI_Test_2_float_float[\passbyvalue\ S2Member[F32, F32]](s1: \passbyvalue\ S2Member[F32, F32], s2: \passbyvalue\ S2Member[F32, F32])
use @FFI_Test_2_double_double[\passbyvalue\ S2Member[F64, F64]](s1: \passbyvalue\ S2Member[F64, F64], s2: \passbyvalue\ S2Member[F64, F64])

use @FFI_Test_3_int32_t_int32_t_double[\passbyvalue\ S3Member[I32, I32, F64]](s1: \passbyvalue\ S3Member[I32, I32, F64], s2: \passbyvalue\ S3Member[I32, I32, F64])
use @FFI_Test_3_float_float_int64_t[\passbyvalue\ S3Member[F32, F32, I64]](s1: \passbyvalue\ S3Member[F32, F32, I64], s2: \passbyvalue\ S3Member[F32, F32, I64])
use @FFI_Test_3_int32_t_float_int64_t[\passbyvalue\ S3Member[I32, F32, I64]](s1: \passbyvalue\ S3Member[I32, F32, I64], s2: \passbyvalue\ S3Member[I32, F32, I64])
use @FFI_Test_3_double_int32_t_int32_t[\passbyvalue\ S3Member[F64, I32, I32]](s1: \passbyvalue\ S3Member[F64, I32, I32], s2: \passbyvalue\ S3Member[F64, I32, I32])
use @FFI_Test_3_float_float_float[\passbyvalue\ S3Member[F32, F32, F32]](s1: \passbyvalue\ S3Member[F32, F32, F32], s2: \passbyvalue\ S3Member[F32, F32, F32])
use @FFI_Test_3_double_double_double[\passbyvalue\ S3Member[F64, F64, F64]](s1: \passbyvalue\ S3Member[F64, F64, F64], s2: \passbyvalue\ S3Member[F64, F64, F64])

use @FFI_Test_4_int32_t_int32_t_int32_t_int32_t[\passbyvalue\ S4Member[I32, I32, I32, I32]](s1: \passbyvalue\ S4Member[I32, I32, I32, I32], s2: \passbyvalue\ S4Member[I32, I32, I32, I32])
use @FFI_Test_4_int64_t_int64_t_int64_t_int64_t[\passbyvalue\ S4Member[I64, I64, I64, I64]](s1: \passbyvalue\ S4Member[I64, I64, I64, I64], s2: \passbyvalue\ S4Member[I64, I64, I64, I64])
use @FFI_Test_4_float_float_float_float[\passbyvalue\ S4Member[F32, F32, F32, F32]](s1: \passbyvalue\ S4Member[F32, F32, F32, F32], s2: \passbyvalue\ S4Member[F32, F32, F32, F32])
use @FFI_Test_4_double_double_double_double[\passbyvalue\ S4Member[F64, F64, F64, F64]](s1: \passbyvalue\ S4Member[F64, F64, F64, F64], s2: \passbyvalue\ S4Member[F64, F64, F64, F64])

use @FFI_Test_2_12_int32_t_int32_t[\passbyvalue\ S2Member[I32, I32]](s1: \passbyvalue\ S2Member[I32, I32], s2: \passbyvalue\ S2Member[I32, I32],
  s3: \passbyvalue\ S2Member[I32, I32], s4: \passbyvalue\ S2Member[I32, I32],
  s5: \passbyvalue\ S2Member[I32, I32], s6: \passbyvalue\ S2Member[I32, I32],
  s7: \passbyvalue\ S2Member[I32, I32], s8: \passbyvalue\ S2Member[I32, I32],
  s9: \passbyvalue\ S2Member[I32, I32], s10: \passbyvalue\ S2Member[I32, I32],
  s11: \passbyvalue\ S2Member[I32, I32], s12: \passbyvalue\ S2Member[I32, I32])

use @FFI_Test_2_12_int64_t_int64_t[\passbyvalue\ S2Member[I64, I64]](s1: \passbyvalue\ S2Member[I64, I64], s2: \passbyvalue\ S2Member[I64, I64],
  s3: \passbyvalue\ S2Member[I64, I64], s4: \passbyvalue\ S2Member[I64, I64],
  s5: \passbyvalue\ S2Member[I64, I64], s6: \passbyvalue\ S2Member[I64, I64],
  s7: \passbyvalue\ S2Member[I64, I64], s8: \passbyvalue\ S2Member[I64, I64],
  s9: \passbyvalue\ S2Member[I64, I64], s10: \passbyvalue\ S2Member[I64, I64],
  s11: \passbyvalue\ S2Member[I64, I64], s12: \passbyvalue\ S2Member[I64, I64])

use @FFI_Test_2_12_float_float[\passbyvalue\ S2Member[F32, F32]](s1: \passbyvalue\ S2Member[F32, F32], s2: \passbyvalue\ S2Member[F32, F32],
  s3: \passbyvalue\ S2Member[F32, F32], s4: \passbyvalue\ S2Member[F32, F32],
  s5: \passbyvalue\ S2Member[F32, F32], s6: \passbyvalue\ S2Member[F32, F32],
  s7: \passbyvalue\ S2Member[F32, F32], s8: \passbyvalue\ S2Member[F32, F32],
  s9: \passbyvalue\ S2Member[F32, F32], s10: \passbyvalue\ S2Member[F32, F32],
  s11: \passbyvalue\ S2Member[F32, F32], s12: \passbyvalue\ S2Member[F32, F32])

use @FFI_Test_2_12_double_double[\passbyvalue\ S2Member[F64, F64]](s1: \passbyvalue\ S2Member[F64, F64], s2: \passbyvalue\ S2Member[F64, F64],
  s3: \passbyvalue\ S2Member[F64, F64], s4: \passbyvalue\ S2Member[F64, F64],
  s5: \passbyvalue\ S2Member[F64, F64], s6: \passbyvalue\ S2Member[F64, F64],
  s7: \passbyvalue\ S2Member[F64, F64], s8: \passbyvalue\ S2Member[F64, F64],
  s9: \passbyvalue\ S2Member[F64, F64], s10: \passbyvalue\ S2Member[F64, F64],
  s11: \passbyvalue\ S2Member[F64, F64], s12: \passbyvalue\ S2Member[F64, F64])

struct S1Member[T1]
  var a: T1

  new create(a': T1^) =>
    a = a'

struct S2Member[T1, T2]
  var a: T1
  var b: T2

  new create(a': T1^, b': T2^) =>
    a = a'
    b = b'

struct S3Member[T1, T2, T3]
  var a: T1
  var b: T2
  var c: T3

  new create(a': T1^, b': T2^, c': T3^) =>
    a = a'
    b = b'
    c = c'

struct S4Member[T1, T2, T3, T4]
  var a: T1
  var b: T2
  var c: T3
  var d: T4

  new create(a': T1^, b': T2^, c': T3^, d': T4^) =>
    a = a'
    b = b'
    c = c'
    d = d'

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

    test_1_member_struct[I8](S1Member[I8](1), S1Member[I8](2),
      @{(s1: \passbyvalue\ S1Member[I8], s2: \passbyvalue\ S1Member[I8]): \passbyvalue\ S1Member[I8] =>
        @FFI_Test_1_int8_t(s1, s2)
      })
    test_1_member_struct[I16](S1Member[I16](1), S1Member[I16](2),
      @{(s1: \passbyvalue\ S1Member[I16], s2: \passbyvalue\ S1Member[I16]): \passbyvalue\ S1Member[I16] =>
        @FFI_Test_1_int16_t(s1, s2)
      })
    test_1_member_struct[I32](S1Member[I32](1), S1Member[I32](2),
      @{(s1: \passbyvalue\ S1Member[I32], s2: \passbyvalue\ S1Member[I32]): \passbyvalue\ S1Member[I32] =>
        @FFI_Test_1_int32_t(s1, s2)
      })
    test_1_member_struct[I64](S1Member[I64](1), S1Member[I64](2),
      @{(s1: \passbyvalue\ S1Member[I64], s2: \passbyvalue\ S1Member[I64]): \passbyvalue\ S1Member[I64] =>
        @FFI_Test_1_int64_t(s1, s2)
      })
    test_1_member_struct[F32](S1Member[F32](1), S1Member[F32](2),
      @{(s1: \passbyvalue\ S1Member[F32], s2: \passbyvalue\ S1Member[F32]): \passbyvalue\ S1Member[F32] =>
        @FFI_Test_1_float(s1, s2)
      })
    test_1_member_struct[F64](S1Member[F64](1), S1Member[F64](2),
      @{(s1: \passbyvalue\ S1Member[F64], s2: \passbyvalue\ S1Member[F64]): \passbyvalue\ S1Member[F64] =>
        @FFI_Test_1_double(s1, s2)
      })


    test_2_member_struct[I8, I8](S2Member[I8, I8](1, 1), S2Member[I8, I8](2, 2),
      @{(s1: \passbyvalue\ S2Member[I8, I8], s2: \passbyvalue\ S2Member[I8, I8]): \passbyvalue\ S2Member[I8, I8] =>
        @FFI_Test_2_int8_t_int8_t(s1, s2)
      })
    test_2_member_struct[I8, I32](S2Member[I8, I32](1, 1), S2Member[I8, I32](2, 2),
      @{(s1: \passbyvalue\ S2Member[I8, I32], s2: \passbyvalue\ S2Member[I8, I32]): \passbyvalue\ S2Member[I8, I32] =>
        @FFI_Test_2_int8_t_int32_t(s1, s2)
      })
    test_2_member_struct[I32, F32](S2Member[I32, F32](1, 1), S2Member[I32, F32](2, 2),
      @{(s1: \passbyvalue\ S2Member[I32, F32], s2: \passbyvalue\ S2Member[I32, F32]): \passbyvalue\ S2Member[I32, F32] =>
        @FFI_Test_2_int32_t_float(s1, s2)
      })
    test_2_member_struct[I64, F64](S2Member[I64, F64](1, 1), S2Member[I64, F64](2, 2),
      @{(s1: \passbyvalue\ S2Member[I64, F64], s2: \passbyvalue\ S2Member[I64, F64]): \passbyvalue\ S2Member[I64, F64] =>
        @FFI_Test_2_int64_t_double(s1, s2)
      })
    test_2_member_struct[I32, I32](S2Member[I32, I32](1, 1), S2Member[I32, I32](2, 2),
      @{(s1: \passbyvalue\ S2Member[I32, I32], s2: \passbyvalue\ S2Member[I32, I32]): \passbyvalue\ S2Member[I32, I32] =>
        @FFI_Test_2_int32_t_int32_t(s1, s2)
      })
    test_2_member_struct[I64, I64](S2Member[I64, I64](1, 1), S2Member[I64, I64](2, 2),
      @{(s1: \passbyvalue\ S2Member[I64, I64], s2: \passbyvalue\ S2Member[I64, I64]): \passbyvalue\ S2Member[I64, I64] =>
        @FFI_Test_2_int64_t_int64_t(s1, s2)
      })
    test_2_member_struct[F32, F32](S2Member[F32, F32](1, 1), S2Member[F32, F32](2, 2),
      @{(s1: \passbyvalue\ S2Member[F32, F32], s2: \passbyvalue\ S2Member[F32, F32]): \passbyvalue\ S2Member[F32, F32] =>
        @FFI_Test_2_float_float(s1, s2)
      })
    test_2_member_struct[F64, F64](S2Member[F64, F64](1, 1), S2Member[F64, F64](2, 2),
      @{(s1: \passbyvalue\ S2Member[F64, F64], s2: \passbyvalue\ S2Member[F64, F64]): \passbyvalue\ S2Member[F64, F64] =>
        @FFI_Test_2_double_double(s1, s2)
      })

     test_3_member_struct[I32, I32, F64](S3Member[I32, I32, F64](1, 1, 1), S3Member[I32, I32, F64](2, 2, 2),
      @{(s1: \passbyvalue\ S3Member[I32, I32, F64], s2: \passbyvalue\ S3Member[I32, I32, F64]): \passbyvalue\ S3Member[I32, I32, F64] =>
        @FFI_Test_3_int32_t_int32_t_double(s1, s2)
      })
     test_3_member_struct[F32, F32, I64](S3Member[F32, F32, I64](1, 1, 1), S3Member[F32, F32, I64](2, 2, 2),
      @{(s1: \passbyvalue\ S3Member[F32, F32, I64], s2: \passbyvalue\ S3Member[F32, F32, I64]): \passbyvalue\ S3Member[F32, F32, I64] =>
        @FFI_Test_3_float_float_int64_t(s1, s2)
      })
    test_3_member_struct[I32, F32, I64](S3Member[I32, F32, I64](1, 1, 1), S3Member[I32, F32, I64](2, 2, 2),
      @{(s1: \passbyvalue\ S3Member[I32, F32, I64], s2: \passbyvalue\ S3Member[I32, F32, I64]): \passbyvalue\ S3Member[I32, F32, I64] =>
        @FFI_Test_3_int32_t_float_int64_t(s1, s2)
      })
    test_3_member_struct[F64, I32, I32](S3Member[F64, I32, I32](1, 1, 1), S3Member[F64, I32, I32](2, 2, 2),
      @{(s1: \passbyvalue\ S3Member[F64, I32, I32], s2: \passbyvalue\ S3Member[F64, I32, I32]): \passbyvalue\ S3Member[F64, I32, I32] =>
        @FFI_Test_3_double_int32_t_int32_t(s1, s2)
      })
    test_3_member_struct[F32, F32, F32](S3Member[F32, F32, F32](1, 1, 1), S3Member[F32, F32, F32](2, 2, 2),
      @{(s1: \passbyvalue\ S3Member[F32, F32, F32], s2: \passbyvalue\ S3Member[F32, F32, F32]): \passbyvalue\ S3Member[F32, F32, F32] =>
        @FFI_Test_3_float_float_float(s1, s2)
      })
    test_3_member_struct[F64, F64, F64](S3Member[F64, F64, F64](1, 1, 1), S3Member[F64, F64, F64](2, 2, 2),
      @{(s1: \passbyvalue\ S3Member[F64, F64, F64], s2: \passbyvalue\ S3Member[F64, F64, F64]): \passbyvalue\ S3Member[F64, F64, F64] =>
        @FFI_Test_3_double_double_double(s1, s2)
      })

    test_4_member_struct[I32, I32, I32, I32](S4Member[I32, I32, I32, I32](1, 1, 1, 1), S4Member[I32, I32, I32, I32](2, 2, 2, 2),
      @{(s1: \passbyvalue\ S4Member[I32, I32, I32, I32], s2: \passbyvalue\ S4Member[I32, I32, I32, I32]): \passbyvalue\ S4Member[I32, I32, I32, I32] =>
        @FFI_Test_4_int32_t_int32_t_int32_t_int32_t(s1, s2)
      })
    test_4_member_struct[I64, I64, I64, I64](S4Member[I64, I64, I64, I64](1, 1, 1, 1), S4Member[I64, I64, I64, I64](2, 2, 2, 2),
      @{(s1: \passbyvalue\ S4Member[I64, I64, I64, I64], s2: \passbyvalue\ S4Member[I64, I64, I64, I64]): \passbyvalue\ S4Member[I64, I64, I64, I64] =>
        @FFI_Test_4_int64_t_int64_t_int64_t_int64_t(s1, s2)
      })
    test_4_member_struct[F32, F32, F32, F32](S4Member[F32, F32, F32, F32](1, 1, 1, 1), S4Member[F32, F32, F32, F32](2, 2, 2, 2),
      @{(s1: \passbyvalue\ S4Member[F32, F32, F32, F32], s2: \passbyvalue\ S4Member[F32, F32, F32, F32]): \passbyvalue\ S4Member[F32, F32, F32, F32] =>
        @FFI_Test_4_float_float_float_float(s1, s2)
      })
    test_4_member_struct[F64, F64, F64, F64](S4Member[F64, F64, F64, F64](1, 1, 1, 1), S4Member[F64, F64, F64, F64](2, 2, 2, 2),
      @{(s1: \passbyvalue\ S4Member[F64, F64, F64, F64], s2: \passbyvalue\ S4Member[F64, F64, F64, F64]): \passbyvalue\ S4Member[F64, F64, F64, F64] =>
        @FFI_Test_4_double_double_double_double(s1, s2)
      })

    test_2_member_struct_12_params[I32, I32](S2Member[I32, I32](1, 1), S2Member[I32, I32](2, 2),
      S2Member[I32, I32](3, 3), S2Member[I32, I32](4, 4),
      S2Member[I32, I32](5, 5), S2Member[I32, I32](6, 6),
      S2Member[I32, I32](7, 7), S2Member[I32, I32](8, 8),
      S2Member[I32, I32](9, 9), S2Member[I32, I32](10, 10),
      S2Member[I32, I32](11, 11), S2Member[I32, I32](12, 12),
      @{(s1: \passbyvalue\ S2Member[I32, I32], s2: \passbyvalue\ S2Member[I32, I32],
        s3: \passbyvalue\ S2Member[I32, I32], s4: \passbyvalue\ S2Member[I32, I32],
        s5: \passbyvalue\ S2Member[I32, I32], s6: \passbyvalue\ S2Member[I32, I32],
        s7: \passbyvalue\ S2Member[I32, I32], s8: \passbyvalue\ S2Member[I32, I32],
        s9: \passbyvalue\ S2Member[I32, I32], s10: \passbyvalue\ S2Member[I32, I32],
        s11: \passbyvalue\ S2Member[I32, I32], s12: \passbyvalue\ S2Member[I32, I32]
      ): \passbyvalue\ S2Member[I32, I32] =>
        @FFI_Test_2_12_int32_t_int32_t(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)
      })

    test_2_member_struct_12_params[I64, I64](S2Member[I64, I64](1, 1), S2Member[I64, I64](2, 2),
      S2Member[I64, I64](3, 3), S2Member[I64, I64](4, 4),
      S2Member[I64, I64](5, 5), S2Member[I64, I64](6, 6),
      S2Member[I64, I64](7, 7), S2Member[I64, I64](8, 8),
      S2Member[I64, I64](9, 9), S2Member[I64, I64](10, 10),
      S2Member[I64, I64](11, 11), S2Member[I64, I64](12, 12),
      @{(s1: \passbyvalue\ S2Member[I64, I64], s2: \passbyvalue\ S2Member[I64, I64],
        s3: \passbyvalue\ S2Member[I64, I64], s4: \passbyvalue\ S2Member[I64, I64],
        s5: \passbyvalue\ S2Member[I64, I64], s6: \passbyvalue\ S2Member[I64, I64],
        s7: \passbyvalue\ S2Member[I64, I64], s8: \passbyvalue\ S2Member[I64, I64],
        s9: \passbyvalue\ S2Member[I64, I64], s10: \passbyvalue\ S2Member[I64, I64],
        s11: \passbyvalue\ S2Member[I64, I64], s12: \passbyvalue\ S2Member[I64, I64]
      ): \passbyvalue\ S2Member[I64, I64] =>
        @FFI_Test_2_12_int64_t_int64_t(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)
      })

   /*test_2_member_struct_12_params[F32, F32](S2Member[F32, F32](1, 1), S2Member[F32, F32](2, 2),
      S2Member[F32, F32](3, 3), S2Member[F32, F32](4, 4),
      S2Member[F32, F32](5, 5), S2Member[F32, F32](6, 6),
      S2Member[F32, F32](7, 7), S2Member[F32, F32](8, 8),
      S2Member[F32, F32](9, 9), S2Member[F32, F32](10, 10),
      S2Member[F32, F32](11, 11), S2Member[F32, F32](12, 12),
      @{(s1: \passbyvalue\ S2Member[F32, F32], s2: \passbyvalue\ S2Member[F32, F32],
        s3: \passbyvalue\ S2Member[F32, F32], s4: \passbyvalue\ S2Member[F32, F32],
        s5: \passbyvalue\ S2Member[F32, F32], s6: \passbyvalue\ S2Member[F32, F32],
        s7: \passbyvalue\ S2Member[F32, F32], s8: \passbyvalue\ S2Member[F32, F32],
        s9: \passbyvalue\ S2Member[F32, F32], s10: \passbyvalue\ S2Member[F32, F32],
        s11: \passbyvalue\ S2Member[F32, F32], s12: \passbyvalue\ S2Member[F32, F32]
      ): \passbyvalue\ S2Member[F32, F32] =>
        @FFI_Test_2_12_float_float(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)
      })

    test_2_member_struct_12_params[F64, F64](S2Member[F64, F64](1, 1), S2Member[F64, F64](2, 2),
      S2Member[F64, F64](3, 3), S2Member[F64, F64](4, 4),
      S2Member[F64, F64](5, 5), S2Member[F64, F64](6, 6),
      S2Member[F64, F64](7, 7), S2Member[F64, F64](8, 8),
      S2Member[F64, F64](9, 9), S2Member[F64, F64](10, 10),
      S2Member[F64, F64](11, 11), S2Member[F64, F64](12, 12),
      @{(s1: \passbyvalue\ S2Member[F64, F64], s2: \passbyvalue\ S2Member[F64, F64],
        s3: \passbyvalue\ S2Member[F64, F64], s4: \passbyvalue\ S2Member[F64, F64],
        s5: \passbyvalue\ S2Member[F64, F64], s6: \passbyvalue\ S2Member[F64, F64],
        s7: \passbyvalue\ S2Member[F64, F64], s8: \passbyvalue\ S2Member[F64, F64],
        s9: \passbyvalue\ S2Member[F64, F64], s10: \passbyvalue\ S2Member[F64, F64],
        s11: \passbyvalue\ S2Member[F64, F64], s12: \passbyvalue\ S2Member[F64, F64]
      ): \passbyvalue\ S2Member[F64, F64] =>
        @FFI_Test_2_12_double_double(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)
      })*/

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

  fun test_1_member_struct[T1: Real[T1] val](s1: S1Member[T1], s2: S1Member[T1],
    ffi_function:
    @{(\passbyvalue\ S1Member[T1], \passbyvalue\ S1Member[T1]): \passbyvalue\ S1Member[T1]}) =>

    let old_s1_a = s1.a

    let old_s2_a = s2.a

    let s_ret = ffi_function(s1, s2)

    if s1.a != old_s1_a then
      @exit(1)
    end

    if s2.a != old_s2_a then
      @exit(2)
    end

    if s_ret.a != (old_s1_a + old_s2_a) then
      @exit(3)
    end

  fun test_2_member_struct[T1: Real[T1] val, T2: Real[T2] val](s1: S2Member[T1, T2], s2: S2Member[T1, T2],
    ffi_function:
    @{(\passbyvalue\ S2Member[T1, T2], \passbyvalue\ S2Member[T1, T2]): \passbyvalue\ S2Member[T1, T2]}) =>

    let old_s1_a = s1.a
    let old_s1_b = s1.b

    let old_s2_a = s2.a
    let old_s2_b = s2.b

    let s_ret = ffi_function(s1, s2)

    if (s1.a != old_s1_a) or (s1.b != old_s1_b) then
      @exit(1)
    end

    if (s2.a != old_s2_a) or (s2.b != old_s2_b) then
      @exit(2)
    end

    if (s_ret.a != (old_s1_a + old_s2_a)) or (s_ret.b != (old_s1_b + old_s2_b)) then
      @exit(3)
    end

fun test_3_member_struct[T1: Real[T1] val, T2: Real[T2] val, T3: Real[T3] val]
    (s1: S3Member[T1, T2, T3], s2: S3Member[T1, T2, T3], ffi_function:
    @{(\passbyvalue\ S3Member[T1, T2, T3], \passbyvalue\ S3Member[T1, T2, T3]): \passbyvalue\ S3Member[T1, T2, T3]}) =>

    let old_s1_a = s1.a
    let old_s1_b = s1.b
    let old_s1_c = s1.c

    let old_s2_a = s2.a
    let old_s2_b = s2.b
    let old_s2_c = s2.c

    let s_ret = ffi_function(s1, s2)

    if (s1.a != old_s1_a) or (s1.b != old_s1_b) or (s1.c != old_s1_c) then
      @exit(1)
    end

    if (s2.a != old_s2_a) or (s2.b != old_s2_b) or (s2.c != old_s2_c) then
      @exit(2)
    end

    if (s_ret.a != (old_s1_a + old_s2_a)) or (s_ret.b != (old_s1_b + old_s2_b)) or
       (s_ret.c != (old_s1_c + old_s2_c)) then
      @exit(3)
    end

fun test_4_member_struct[T1: Real[T1] val, T2: Real[T2] val, T3: Real[T3] val, T4: Real[T4] val]
    (s1: S4Member[T1, T2, T3, T4], s2: S4Member[T1, T2, T3, T4], ffi_function:
    @{(\passbyvalue\ S4Member[T1, T2, T3, T4], \passbyvalue\ S4Member[T1, T2, T3, T4]): \passbyvalue\ S4Member[T1, T2, T3, T4]}) =>

    let old_s1_a = s1.a
    let old_s1_b = s1.b
    let old_s1_c = s1.c
    let old_s1_d = s1.d

    let old_s2_a = s2.a
    let old_s2_b = s2.b
    let old_s2_c = s2.c
    let old_s2_d = s2.d

    let s_ret = ffi_function(s1, s2)

    if (s1.a != old_s1_a) or (s1.b != old_s1_b) or
       (s1.c != old_s1_c) or (s1.d != old_s1_d) then
      @exit(1)
    end

    if (s2.a != old_s2_a) or (s2.b != old_s2_b) or
       (s2.c != old_s2_c) or (s2.d != old_s2_d) then
      @exit(2)
    end

    if (s_ret.a != (old_s1_a + old_s2_a)) or (s_ret.b != (old_s1_b + old_s2_b)) or
       (s_ret.c != (old_s1_c + old_s2_c)) or (s_ret.d != (old_s1_d + old_s2_d)) then
      @exit(3)
    end

  fun test_2_member_struct_12_params[T1: Real[T1] val, T2: Real[T2] val](
    s1: S2Member[T1, T2], s2: S2Member[T1, T2],
    s3: S2Member[T1, T2], s4: S2Member[T1, T2],
    s5: S2Member[T1, T2], s6: S2Member[T1, T2],
    s7: S2Member[T1, T2], s8: S2Member[T1, T2],
    s9: S2Member[T1, T2], s10: S2Member[T1, T2],
    s11: S2Member[T1, T2], s12: S2Member[T1, T2],
    ffi_function:
    @{(\passbyvalue\ S2Member[T1, T2], \passbyvalue\ S2Member[T1, T2],
       \passbyvalue\ S2Member[T1, T2], \passbyvalue\ S2Member[T1, T2],
       \passbyvalue\ S2Member[T1, T2], \passbyvalue\ S2Member[T1, T2],
       \passbyvalue\ S2Member[T1, T2], \passbyvalue\ S2Member[T1, T2],
       \passbyvalue\ S2Member[T1, T2], \passbyvalue\ S2Member[T1, T2],
       \passbyvalue\ S2Member[T1, T2], \passbyvalue\ S2Member[T1, T2]
       ): \passbyvalue\ S2Member[T1, T2]}) =>

    let old_s1_a = s1.a
    let old_s1_b = s1.b

    let old_s2_a = s2.a
    let old_s2_b = s2.b

    let old_s3_a = s3.a
    let old_s3_b = s3.b

    let old_s4_a = s4.a
    let old_s4_b = s4.b

    let old_s5_a = s5.a
    let old_s5_b = s5.b

    let old_s6_a = s6.a
    let old_s6_b = s6.b

    let old_s7_a = s7.a
    let old_s7_b = s7.b

    let old_s8_a = s8.a
    let old_s8_b = s8.b

    let old_s9_a = s9.a
    let old_s9_b = s9.b

    let old_s10_a = s10.a
    let old_s10_b = s10.b

    let old_s11_a = s11.a
    let old_s11_b = s11.b

    let old_s12_a = s12.a
    let old_s12_b = s12.b

    let s_ret = ffi_function(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)

    if (s1.a != old_s1_a) or (s1.b != old_s1_b) then
      @exit(1)
    end

    if (s2.a != old_s2_a) or (s2.b != old_s2_b) then
      @exit(2)
    end

    if (s3.a != old_s3_a) or (s3.b != old_s3_b) then
      @exit(2)
    end

    if (s4.a != old_s4_a) or (s4.b != old_s4_b) then
      @exit(2)
    end

    if (s5.a != old_s5_a) or (s5.b != old_s5_b) then
      @exit(2)
    end

    if (s6.a != old_s6_a) or (s6.b != old_s6_b) then
      @exit(2)
    end

    if (s7.a != old_s7_a) or (s7.b != old_s7_b) then
      @exit(2)
    end

    if (s8.a != old_s8_a) or (s8.b != old_s8_b) then
      @exit(2)
    end

    if (s9.a != old_s9_a) or (s9.b != old_s9_b) then
      @exit(2)
    end

    if (s10.a != old_s10_a) or (s10.b != old_s10_b) then
      @exit(2)
    end

    if (s11.a != old_s11_a) or (s11.b != old_s11_b) then
      @exit(2)
    end

    if (s12.a != old_s12_a) or (s12.b != old_s12_b) then
      @exit(2)
    end

    let expected_a = old_s1_a + old_s2_a + old_s3_a + old_s4_a +
                     old_s5_a + old_s6_a + old_s7_a + old_s8_a +
                     old_s9_a + old_s10_a + old_s11_a + old_s12_a

    let expected_b = old_s1_b + old_s2_b + old_s3_b + old_s4_b +
                     old_s5_b + old_s6_b + old_s7_b + old_s8_b +
                     old_s9_b + old_s10_b + old_s11_b + old_s12_b

    if (s_ret.a != expected_a) or (s_ret.b != expected_b) then
      @exit(3)
    end