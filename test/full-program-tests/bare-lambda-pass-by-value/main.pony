use "lib:bare-lambda-pass-by-value-additional"

use @pony_exitcode[None](code: I32)

use @call_via_C_clobber_small[None](lambda: @{(\passbyvalue\ S1Small, \passbyvalue\ S1Small)},
  x: \passbyvalue\ S1Small, y: \passbyvalue\ S1Small)
use @call_via_C_clobber_large[None](lambda: @{(\passbyvalue\ S1Large, \passbyvalue\ S1Large)},
  x: \passbyvalue\ S1Large, y: \passbyvalue\ S1Large)
use @call_via_C_add_small[\passbyvalue\ S1Small](
  lambda: @{(\passbyvalue\ S1Small, \passbyvalue\ S1Small): \passbyvalue\ S1Small},
  x: \passbyvalue\ S1Small, y: \passbyvalue\ S1Small)
use @call_via_C_add_large[\passbyvalue\ S1Large](
  lambda: @{(\passbyvalue\ S1Large, \passbyvalue\ S1Large): \passbyvalue\ S1Large},
  x: \passbyvalue\ S1Large, y: \passbyvalue\ S1Large)

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

    ret = test_large_struct[1000]()
    if ret != 0 then
      @pony_exitcode(ret)
      return
    end

  fun test_small_struct[ret_add: I32](): I32 =>
    let lambda_clobber: @{(\passbyvalue\ S1Small, \passbyvalue\ S1Small)} =
      @{(x: \passbyvalue\ S1Small, y: \passbyvalue\ S1Small) =>
        for i in x.ar.keys() do
          try
            x.ar.update(i, 1234)?
            y.ar.update(i, 1234)?
          end
        end
      }

    let lambda: @{(\passbyvalue\ S1Small, \passbyvalue\ S1Small): \passbyvalue\ S1Small} =
      @{(x: \passbyvalue\ S1Small, y: \passbyvalue\ S1Small): \passbyvalue\ S1Small =>
        var z = S1Small(0)
        for i in x.ar.keys() do
          try
            z.ar.update(i, x.ar(i)? + y.ar(i)?)?
          end
        end
        z
      }

    var x = S1Small(100)
    var y = S1Small(200)

    lambda_clobber(x, y)

    if not check_small(x, 100) then
      return 1 + ret_add
    end

    if not check_small(y, 200) then
      return 2 + ret_add
    end

    let z = lambda(x, y)

    if not check_small(z, 300) then
      return 3 + ret_add
    end

    var z2 = S1Small(0)
    z2 = lambda(x, y)
    if not check_small(z2, 300) then
      return 4 + ret_add
    end

    // test return without any assignment
    lambda(x, y)

    @call_via_C_clobber_small(lambda_clobber, x, y)

    if not check_small(x, 100) then
      return 5 + ret_add
    end

    if not check_small(y, 200) then
      return 6 + ret_add
    end

    z2 = @call_via_C_add_small(lambda, x, y)
    if not check_small(z2, 300) then
      return 7 + ret_add
    end

    0

  fun test_large_struct[ret_add: I32](): I32 =>
    let lambda_clobber: @{(\passbyvalue\ S1Large, \passbyvalue\ S1Large)} =
      @{(x: \passbyvalue\ S1Large, y: \passbyvalue\ S1Large) =>
        for i in x.ar.keys() do
          try
            x.ar.update(i, 1234)?
            y.ar.update(i, 1234)?
          end
        end
      }

    let lambda: @{(\passbyvalue\ S1Large, \passbyvalue\ S1Large): \passbyvalue\ S1Large} =
      @{(x: \passbyvalue\ S1Large, y: \passbyvalue\ S1Large): \passbyvalue\ S1Large =>
        var z = S1Large(0)
        for i in x.ar.keys() do
          try
            z.ar.update(i, x.ar(i)? + y.ar(i)?)?
          end
        end
        z
      }

    var x = S1Large(100)
    var y = S1Large(200)

    lambda_clobber(x, y)

    if not check_large(x, 100) then
      return 1 + ret_add
    end

    if not check_large(y, 200) then
      return 2 + ret_add
    end

    let z = lambda(x, y)

    if not check_large(z, 300) then
      return 3 + ret_add
    end

    var z2 = S1Large(0)
    z2 = lambda(x, y)
    if not check_large(z2, 300) then
      return 4 + ret_add
    end

    // test return without any assignment
    lambda(x, y)

    @call_via_C_clobber_large(lambda_clobber, x, y)

    if not check_large(x, 100) then
      return 5 + ret_add
    end

    if not check_large(y, 200) then
      return 6 + ret_add
    end

    z2 = @call_via_C_add_large(lambda, x, y)
    if not check_large(z2, 300) then
      return 7 + ret_add
    end

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
