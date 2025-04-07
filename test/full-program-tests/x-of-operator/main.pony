use "lib:x-of-operator-additional"

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
    if (sizeof S1 < 12) or (sizeof S1 > 16) then
      @pony_exitcode(1)
      return
    end

    // This depends on how the target architecture aligns fields
    if (sizeof s1 < 12) or (sizeof s1 > 16) then
      @pony_exitcode(1)
      return
    end

    if sizeof S1.x != 4 then
      @pony_exitcode(2)
      return
    end

    if sizeof s1.y != 8 then
      @pony_exitcode(3)
      return
    end

    if sizeof xl != 8 then
      @pony_exitcode(4)
      return
    end

    if sizeof yl != 4 then
      @pony_exitcode(5)
      return
    end

    if sizeof I32 != 4 then
      @pony_exitcode(6)
      return
    end

    if sizeof F64 != 8 then
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