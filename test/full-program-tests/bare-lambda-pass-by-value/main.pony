use "lib:bare-lambda-pass-by-value-additional"

use @pony_exitcode[None](code: I32)

use @call_via_C_clobber_small[None](lambda: @{(\byval\ S1Small, \byval\ S1Small)},
  x: \byval\ S1Small, y: \byval\ S1Small)
use @call_via_C_clobber_large[None](lambda: @{(\byval\ S1Large, \byval\ S1Large)},
  x: \byval\ S1Large, y: \byval\ S1Large)
use @call_via_C_add_small[\byval\ S1Small](
  lambda: @{(\byval\ S1Small, \byval\ S1Small): \byval\ S1Small},
  x: \byval\ S1Small, y: \byval\ S1Small)
use @call_via_C_add_large[\byval\ S1Large](
  lambda: @{(\byval\ S1Large, \byval\ S1Large): \byval\ S1Large},
  x: \byval\ S1Large, y: \byval\ S1Large)

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

    ret = test_return_tuples[40]()
    if ret != 0 then
      @pony_exitcode(ret)
      return
    end

  fun test_small_struct[ret_add: I32](): I32 =>
    let lambda_clobber: @{(\byval\ S1Small, \byval\ S1Small)} =
      @{(x: \byval\ S1Small, y: \byval\ S1Small) =>
        for i in x.ar.keys() do
          try
            x.ar.update(i, 1234)?
            y.ar.update(i, 1234)?
          end
        end
      }

    let lambda: @{(\byval\ S1Small, \byval\ S1Small): \byval\ S1Small} =
      @{(x: \byval\ S1Small, y: \byval\ S1Small): \byval\ S1Small =>
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
    let lambda_clobber: @{(\byval\ S1Large, \byval\ S1Large)} =
      @{(x: \byval\ S1Large, y: \byval\ S1Large) =>
        for i in x.ar.keys() do
          try
            x.ar.update(i, 1234)?
            y.ar.update(i, 1234)?
          end
        end
      }

    let lambda: @{(\byval\ S1Large, \byval\ S1Large): \byval\ S1Large} =
      @{(x: \byval\ S1Large, y: \byval\ S1Large): \byval\ S1Large =>
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

  fun test_return_tuples[ret_add: I32](): I32 =>

    // This should be lowered on most CPUs
    let l1: @{(): (Bool, Bool)} = @{(): (Bool, Bool) => (false, true) }

    (let a, let b) = l1()

    if (a != false) or (b != true) then
      return 1 + ret_add
    end

    // This should not be lowered on most CPUs
    let l2: @{(): (USize, I32, I8, ISize)} = @{(): (USize, I32, I8, ISize) => (11, 22, 33, 44) }

    (let a2, let b2, let c2, let d2) = l2()

    if (a2 != 11) or (b2 != 22) or (c2 != 33) or (d2 != 44) then
      return 2 + ret_add
    end

    0