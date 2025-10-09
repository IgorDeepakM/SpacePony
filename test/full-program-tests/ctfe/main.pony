use @pony_exitcode[None](code: I32)
use @printf[I32](fmt: Pointer[U8] tag, ...)

struct E1
  var em: ILong = 1010

struct S1
  var x: U32 = 2020
  var y: U32 = 3030
  embed z: E1 = E1

actor Main
  new create(env: Env) =>
    test_literal_int(0)
    test_repeat(20)
    test_while(40)
    test_if_else(60)
    test_try(80)
    test_member_access(100)
    test_tuple(120)
    test_string_literal(140)


  fun test_literal_int(exit_add: I32) =>
    // Test shift with negative numbers
    var x1: I64 = comptime (-33) << 2 end
    var x2: I64 = (-33) << 2

    if x1 != x2 then
      @pony_exitcode(exit_add + 1)
    end

    x1 = comptime (-33) >> 2 end
    x2 = (-33) >> 2

    if x1 != x2 then
      @pony_exitcode(exit_add + 2)
    end

    // Multiplication with negative numbers
    x1 = comptime 33 * (-11) end
    x2 = 33 * (-11)

    if x1 != x2 then
      @pony_exitcode(exit_add + 3)
    end

    // Multiplication with negative numbers
    x1 = comptime 33 * (-11) end
    x2 = 33 * (-11)

    if x1 != x2 then
      @pony_exitcode(exit_add + 4)
    end

    // Division with negative numbers
    x1 = comptime 33 / (-2) end
    x2 = 33 / (-2)

    if x1 != x2 then
      @pony_exitcode(exit_add + 5)
    end

    var big1: I128 = comptime ((((((3 + (20 * (-7))) / 2) >> 2) * 0xffff) / -34) << 112) + 0x7fffffff end
    var big2: I128 = ((((((3 + (20 * (-7))) / 2) >> 2) * 0xffff) / -34) << 112) + 0x7fffffff

    if big1 != big2 then
      @pony_exitcode(exit_add + 6)
    end

  fun test_repeat(exit_add: I32) =>
    var x1: ILong = comptime standard_repeat(10) end
    var x2: ILong = standard_repeat(10)

    if (x1 != 10) or (x2 != 10) then
      @pony_exitcode(exit_add + 1)
    end

    x1 = comptime repeat_with_else(10) end
    x2 = repeat_with_else(10)

    if (x1 != 44) or (x2 != 44) then
      @pony_exitcode(exit_add + 2)
    end

  fun standard_repeat(r: ILong): ILong =>
    var tt: ILong = 0

    repeat
      tt = tt + 1
    until tt >= r end

    tt

  fun repeat_with_else(r: ILong): ILong =>
    var tt: ILong = 0

    repeat
      tt = tt + 1
      if tt >= r then
        continue
      else
        tt
      end
    until tt >= r else
      44
    end

  fun test_while(exit_add: I32) =>
    var x1: ILong = comptime standard_while(10) end
    var x2: ILong = standard_while(10)

    if (x1 != 10) or (x2 != 10) then
      @pony_exitcode(exit_add + 1)
    end

    x1 = comptime while_with_else(10) end
    x2 = while_with_else(10)

    if (x1 != 44) or (x2 != 44) then
      @pony_exitcode(exit_add + 2)
    end

    x1 = comptime while_with_break(10) end
    x2 = while_with_break(10)

    if (x1 != 5) or (x2 != 5) then
      @pony_exitcode(exit_add + 3)
    end

  fun standard_while(r: ILong): ILong =>
    var tt: ILong = 0

     while tt < r do
       tt = tt + 1
     end

     tt

  fun while_with_else(r: ILong): ILong =>
    var tt: ILong = r

    while tt < r do
      tt = tt + 1
    else
      44
    end

  fun while_with_break(r: ILong): ILong =>
    var tt: ILong = 0

    while tt < r do
      tt = tt + 1
      if tt >= (r / 2) then
        break tt
      end
    end

    tt

  fun test_if_else(exit_add: I32) =>
    var x1: ILong = comptime if_only(9) end
    var x2: ILong = if_only(9)

    if (x1 != 44) or (x2 != 44) then
      @pony_exitcode(exit_add + 1)
    end

    x1 = comptime if_else(10) end
    x2 = if_else(10)

    if (x1 != 44) or (x2 != 44) then
      @pony_exitcode(exit_add + 2)
    end

    x1 = comptime if_elseif_else(16) end
    x2 = if_elseif_else(16)

    if (x1 != 44) or (x2 != 44) then
      @pony_exitcode(exit_add + 3)
    end


  fun if_only(r: ILong): ILong =>
    var x: ILong = 0

    if r < 10 then
      x = 44
    end

    x

  fun if_else(r: ILong): ILong =>
    var x: ILong = 0

    if r < 10 then
      x = 33
    else
      x = 44
    end

    x

  fun if_elseif_else(r: ILong): ILong =>
    var x: ILong = 0

    if r < 10 then
      x = 33
    elseif r < 15 then
      x = 22
    else
      x = 44
    end

    x

  fun test_try(exit_add: I32) =>
    var x1: ILong = comptime
      try
        succ_or_fail(false)?
      else
        11
      end
    end

    var x2: ILong =
      try
        succ_or_fail(false)?
      else
        11
      end

    if (x1 != 44) or (x2 != 44) then
      @pony_exitcode(exit_add + 1)
    end

    x1 = comptime
      try
        succ_or_fail(true)?
      else
        55
      end
    end

    x2 =
      try
        succ_or_fail(true)?
      else
        55
      end

    if (x1 != 55) or (x2 != 55) then
      @pony_exitcode(exit_add + 2)
    end

  fun succ_or_fail(fail: Bool): ILong? =>
    if fail then
      error
    end

    44

  fun test_member_access(exit_add: I32) =>
    var x1: ILong = comptime member_access() end
    var x2: ILong = member_access()

    if (x1 != 33) or (x2 != 33) then
      @pony_exitcode(exit_add + 1)
    end

  fun member_access(): ILong =>
    var s1: S1 = S1

    if (s1.x != 2020) or (s1.y != 3030) or (s1.z.em != 1010) then
      return 0
    end

    s1.x = 11
    s1.z.em = 22

    s1.x.ilong() + s1.z.em

  fun test_tuple(exit_add: I32) =>
    var c1: Bool = comptime test_tuple_creation_assignment() end
    var c2: Bool = test_tuple_creation_assignment()

    if (c1 != true) or (c2 != true) then
      @pony_exitcode(exit_add + 1)
    end

    c1 = comptime test_nested_tuple_creation_assignment() end
    c2 = test_nested_tuple_creation_assignment()

    if (c1 != true) or (c2 != true) then
      @pony_exitcode(exit_add + 1)
    end

  fun test_tuple_creation_assignment(): Bool =>
    var x: (I32, I32, I64) = (1, 2, 3)

    var x1 = x._1
    var x2 = x._2
    var x3 = x._3

    if (x1 != 1) or (x2 != 2) or (x3 != 3) then
      return false
    end

    var x22: I32
    (var x11, x22, var x33: I64) = x

    if (x11 != 1) or (x22 != 2) or (x33 != 3) then
      return false
    end

    x = (11, 22, 33)

    (x11, _, x33) = x

    if (x11 != 11) or (x22 != 2) or (x33 != 33) then
      return false
    end

    true

  fun test_nested_tuple_creation_assignment(): Bool =>
    var x: (I32, (I32, I64)) = (1, (2, 3))

    var x1 = x._1
    var xt2 = x._2
    var x2 = xt2._1
    var x3 = xt2._2

    if (x1 != 1) or (x2 != 2) or (x3 != 3) then
      return false
    end

    var x22: I32
    (var x11, (x22, var x33: I64)) = x

    if (x11 != 1) or (x22 != 2) or (x33 != 3) then
      return false
    end

    x = (11, (22, 33))

    (x11, (_, x33)) = x

    if (x11 != 11) or (x22 != 2) or (x33 != 33) then
      return false
    end

    true

  fun test_string_literal(exit_add: I32) =>
    let c1: String = comptime test_string_literal_add("this", " that") end
    let c2: String = test_string_literal_add("this", " that")

    if (c1 != "this that") or (c2 != "this that") then
      @pony_exitcode(exit_add + 1)
    end

  fun test_string_literal_add(s1: String, s2: String): String =>
    s1 + s2