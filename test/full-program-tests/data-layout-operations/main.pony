use "collections"
use "lib:data-layout-operations-additional"

use @FFI_Func1[I32](x: I32, y: I32)

use @pony_exitcode[None](code: I32)

struct S1
  var x: I32 = 0
  let y: F64 = 0.0

struct TT
  var x1: I8 = 33
  var alignas(8 * 8) x2: U32 = 44

struct TT2[A]
  var x1: I8 = 33
  var alignas(4 * A.size_of) x2: U32 = 44

struct alignas(64) TT3
  var x1: I8 = 33
  var x2: U32 = 44

struct alignas(A.size_of * 64) TT4[A]
  var x1: I8 = 33
  var x2: U32 = 44

struct TT5
  embed alignas(128) ar: CFixedSizedArray[I8, 7] = CFixedSizedArray[I8, 7](0)
  embed ar2: CFixedSizedArray[I8, 7] = CFixedSizedArray[I8, 7](0)
  var tt: F64 = 0.0

actor Main
  new create(env: Env) =>

    // Test sizeof operator

    var xl: I64 = 0
    let yl: F32 = 0.0
    var s1: S1 = S1

    // This depends on how the target architecture aligns fields
    if (S1.size_of < 12) or (S1.size_of > 16) then
      @pony_exitcode(1)
      return
    end

    // This depends on how the target architecture aligns fields
    if (s1.size_of < 12) or (s1.size_of > 16) then
      @pony_exitcode(2)
      return
    end

    if S1.x.size_of != 4 then
      @pony_exitcode(3)
      return
    end

    if s1.y.size_of != 8 then
      @pony_exitcode(4)
      return
    end

    if xl.size_of != 8 then
      @pony_exitcode(5)
      return
    end

    if yl.size_of != 4 then
      @pony_exitcode(6)
      return
    end

    if I32.size_of != 4 then
      @pony_exitcode(7)
      return
    end

    if F64.size_of != 8 then
      @pony_exitcode(8)
      return
    end

    // Test addressof operator

    let ffi_func1_bare_lambda: @{(I32, I32): I32} = addressof @FFI_Func1
    let ret = ffi_func1_bare_lambda(3, 7)
    if ret != 10 then
      @pony_exitcode(9)
      return
    end

    // Test alignas

    for i in Range(0, 100) do
    let tt: TT = TT

    // put a spacer in between to try throw off the alignment of the
    // allocator
    let spacer = CFixedSizedArray[U8, 55](0)

    if (tt.x1 != 33) or (tt.x2 != 44) then
      @pony_exitcode(10)
      return
    end

    if (tt.x2.offset_of != 64) or (tt.x2.align_of != 64) then
      @pony_exitcode(11)
      return
    end

    if tt.size_of != 128 then
      @pony_exitcode(12)
      return
    end
  end

  for i in Range(0, 100) do
    let tt2: TT2[U32] = TT2[U32]

    let spacer = CFixedSizedArray[U8, 55](0)

    if (tt2.x1 != 33) or (tt2.x2 != 44) then
      @pony_exitcode(13)
      return
    end

    if (tt2.x2.offset_of != 16) or (tt2.x2.align_of != 16) then
      @pony_exitcode(14)
      return
    end

    if tt2.size_of != 32 then
      @pony_exitcode(15)
      return
    end
  end

  for i in Range(0, 100) do
    let tt3: TT3 = TT3

    let spacer = CFixedSizedArray[U8, 55](0)

    if tt3.size_of != 64 then
      @pony_exitcode(16)
      return
    end

    let p3 = Pointer[TT3].from_reftype(tt3)

    if (p3.usize() and (64 - 1)) != 0 then
      @pony_exitcode(17)
      return
    end
  end

  for i in Range(0, 100) do
    let tt4: TT4[I32] = TT4[I32]

    if tt4.size_of != 256 then
      @pony_exitcode(18)
      return
    end

    let p4 = Pointer[TT4[I32]].from_reftype(tt4)

    let spacer = CFixedSizedArray[U8, 55](0)

    if (p4.usize() and (256 - 1)) != 0 then
      @pony_exitcode(19)
      return
    end
  end

  // Test align_of

  // This depends on how the target architecture aligns fields
    if (S1.align_of < 4) or (S1.align_of > 8) then
      @pony_exitcode(20)
      return
    end

    // This depends on how the target architecture aligns fields
    if (s1.align_of < 4) or (s1.align_of > 8) then
      @pony_exitcode(21)
      return
    end

    if (S1.x.align_of < 4) or (S1.x.align_of > 8) then
      @pony_exitcode(22)
      return
    end

    if s1.y.align_of != 8 then
      @pony_exitcode(23)
      return
    end

    if xl.align_of != 8 then
      @pony_exitcode(24)
      return
    end

    if yl.align_of != 4 then
      @pony_exitcode(25)
      return
    end

    if I32.align_of != 4 then
      @pony_exitcode(26)
      return
    end

    if F64.align_of != 8 then
      @pony_exitcode(27)
      return
    end

    if TT5.align_of != 128 then
      @pony_exitcode(28)
      return
    end

    if TT5.ar.align_of != 128 then
      @pony_exitcode(29)
      return
    end

    if TT5.ar2.align_of != 1 then
      @pony_exitcode(30)
      return
    end