
use @pony_exitcode[None](code: I32)

struct TT
  var x1: I8 = 33
  var alignas(4 * 4) x2: U32 = 44

struct TT2[A]
  var x1: I8 = 33
  var alignas(4 * sizeof A) x2: U32 = 44

actor Main
  new create(env: Env) =>

  let tt: TT = TT

  if (tt.x1 != 33) or (tt.x2 != 44) then
    @pony_exitcode(1)
    return
  end

  if (offsetof tt.x2) != 16 then
    @pony_exitcode(2)
    return
  end

  if (sizeof tt) != 32 then
    @pony_exitcode(3)
    return
  end

  let tt2: TT2[U32] = TT2[U32]

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