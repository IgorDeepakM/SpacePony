use "lib:data-structure-operations-additional"

use @FFI_Func1[I32](x: I32, y: I32)

use @pony_exitcode[None](code: I32)

struct S1
  var x: I32 = 0
  let y: F64 = 0.0

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
      @pony_exitcode(1)
      return
    end

    if S1.x.size_of != 4 then
      @pony_exitcode(2)
      return
    end

    if s1.y.size_of != 8 then
      @pony_exitcode(3)
      return
    end

    if xl.size_of != 8 then
      @pony_exitcode(4)
      return
    end

    if yl.size_of != 4 then
      @pony_exitcode(5)
      return
    end

    if I32.size_of != 4 then
      @pony_exitcode(6)
      return
    end

    if F64.size_of != 8 then
      @pony_exitcode(7)
      return
    end

    // Test addressof operator

    let ffi_func1_bare_lambda: @{(I32, I32): I32} = addressof @FFI_Func1
    let ret = ffi_func1_bare_lambda(3, 7)
    if ret != 10 then
      @pony_exitcode(8)
      return
    end