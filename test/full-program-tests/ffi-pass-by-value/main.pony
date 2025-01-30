use "lib:ffi-pass-by-value-additional"

use @pony_exitcode[None](code: I32)

use @FFI_clobber_small[None](x: \passbyvalue\ S1Small, y: \passbyvalue\ S1Small)
use @FFI_clobber_large[None](x: \passbyvalue\ S1Large, y: \passbyvalue\ S1Large)
use @FFI_add_small[\passbyvalue\ S1Small](x: \passbyvalue\ S1Small, y: \passbyvalue\ S1Small)
use @FFI_add_large[\passbyvalue\ S1Large](x: \passbyvalue\ S1Large, y: \passbyvalue\ S1Large)

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
    var ret = test_small_struct()
    if ret != 0 then
      @pony_exitcode(ret)
      return
    end

    ret = test_large_struct()
    if ret != 0 then
      @pony_exitcode(ret)
      return
    end

  fun test_small_struct(): I32 =>
    //============== Test small struct

    var x = S1Small(100)
    var y = S1Small(200)

    // Check that passing by value retains the data at call site
    @FFI_clobber_small(x, y)

    if not check_small(x, 100) then
      return 1
    end

    if not check_small(y, 200) then
      return 2
    end
    
    // Test return with new allocation
    let z = @FFI_add_small(x, y)

    if not check_small(z, 300) then
      return 3
    end

    // Test return with already allocated
    var z2 = S1Small(0)
    z2 = @FFI_add_small(x, y)

    if not check_small(z2, 300) then
      return 4
    end

    // Test return but without any assignment, shouldn't crash
    @FFI_add_small(x, y)

    0

  fun test_large_struct(): I32 =>
    //============== Test large struct

    var x = S1Large(100)
    var y = S1Large(200)

    // Check that passing by value retains the data at call site
    @FFI_clobber_large(x, y)

    if not check_large(x, 100) then
      return 100
    end

    if not check_large(y, 200) then
      return 200
    end
    
    // Test return with new allocation
    let z = @FFI_add_large(x, y)

    if not check_large(z, 300) then
      return 300
    end

    // Test return with already allocated
    var z2 = S1Large(0)
    z2 = @FFI_add_large(x, y)

    if not check_large(z2, 300) then
      return 400
    end

    // Test return but without any assignment, shouldn't crash
    @FFI_add_large(x, y)

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
