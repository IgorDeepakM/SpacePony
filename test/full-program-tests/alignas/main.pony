
use "collections"

use @pony_exitcode[None](code: I32)

struct TT
  var x1: I8 = 33
  var alignas(8 * 8) x2: U32 = 44

struct TT2[A]
  var x1: I8 = 33
  var alignas(4 * sizeof A) x2: U32 = 44

struct alignas(64) TT3
  var x1: I8 = 33
  var x2: U32 = 44

struct alignas(sizeof A * 64) TT4[A]
  var x1: I8 = 33
  var x2: U32 = 44

actor Main
  new create(env: Env) =>

  for i in Range(0, 100) do
    let tt: TT = TT

    // put a spacer in between to try throw off the alignment of the
    // allocator
    let spacer = CFixedSizedArray[U8, 55](0)

    if (tt.x1 != 33) or (tt.x2 != 44) then
      @pony_exitcode(1)
      return
    end

    if (offsetof tt.x2) != 64 then
      @pony_exitcode(2)
      return
    end

    if (sizeof tt) != 128 then
      @pony_exitcode(3)
      return
    end
  end

  for i in Range(0, 100) do
    let tt2: TT2[U32] = TT2[U32]

    let spacer = CFixedSizedArray[U8, 55](0)

    if (tt2.x1 != 33) or (tt2.x2 != 44) then
      @pony_exitcode(4)
      return
    end

    if (offsetof tt2.x2) != 16 then
      @pony_exitcode(5)
      return
    end

    if (sizeof tt2) != 32 then
      @pony_exitcode(6)
      return
    end
  end

  for i in Range(0, 100) do
    let tt3: TT3 = TT3

    let spacer = CFixedSizedArray[U8, 55](0)

    if (sizeof tt3) != 64 then
      @pony_exitcode(7)
      return
    end

    let p3 = Pointer[TT3].from_reftype(tt3)

    if (p3.usize() and (64 - 1)) != 0 then
      @pony_exitcode(8)
      return
    end
  end

  for i in Range(0, 100) do
    let tt4: TT4[I32] = TT4[I32]

    if (sizeof tt4) != 256 then
      @pony_exitcode(9)
      return
    end

    let p4 = Pointer[TT4[I32]].from_reftype(tt4)

    let spacer = CFixedSizedArray[U8, 55](0)

    if (p4.usize() and (256 - 1)) != 0 then
      @pony_exitcode(10)
      return
    end
  end