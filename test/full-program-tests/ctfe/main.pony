use "collections"

use @pony_exitcode[None](code: I32)
use @printf[I32](fmt: Pointer[U8] tag, ...)

use @memcmp[I32](dst: Pointer[None] tag, src: Pointer[None] tag, len: USize)
use @memset[Pointer[None]](dst: Pointer[None], set: U32, len: USize)
use @memmove[Pointer[None]](dst: Pointer[None], src: Pointer[None], len: USize)
use @memcpy[Pointer[None]](dst: Pointer[None], src: Pointer[None], len: USize)

struct E1
  var em: ILong = 1010

struct S1
  var x: U32 = 2020
  var y: U32 = 3030
  embed z: E1 = E1

struct S2
  var x: U32 = 2020
  var y: U32 = 3030
  var z: E1 = E1
  embed z2: E1 = E1

class G1[T1: (Int & Real[T1] val), T2: (Int & Real[T2] val)]
  var g1: T1
  var g2: T2

  new create(a: T1, b: T2) =>
    g1 = a
    g2 = b

  fun compute[c: T1](d: T2): T1 =>
    g1 + T1.from[T2](g2) + c + T1.from[T2](d)

actor Main
  new create(env: Env) =>
    test_literal_int(0)
    test_repeat(20)
    test_while(40)
    test_if_else(60)
    test_try(80)
    test_member_access(100)
    test_tuple(120)
    test_generics(140)
    test_pointer(160)
    test_ffi_calls(180)
    test_match(200)
    test_array(220)
    test_string(240)
    test_value_type_param_expr(260)
    test_constant_object(280)

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

    x1 = comptime while_but_as_for_loop(10) end
    x2 = while_but_as_for_loop(10)

    if (x1 != 9) or (x2 != 9) then
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

  fun while_but_as_for_loop(r: ILong): ILong =>
    var tt: ILong = 0

    for i in Range[ILong](1, r) do
      tt = tt + 1
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
      @pony_exitcode(exit_add + 2)
    end

    var d1 = comptime test_constant_tuple_creation() end
    var d2 = test_constant_tuple_creation()

    if (d1._1 != 11) or (d2._1 != 11) or
       (d1._2 != 33) or (d2._2 != 33) then
      @pony_exitcode(exit_add + 3)
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

  fun test_constant_tuple_creation(): (U32, U64) =>
    (11, 33)

  fun test_generics(exit_add: I32) =>
    let c1 = comptime test_method_typeargs[I32, U64](11, 22) end
    let c2 = test_method_typeargs[I32, U64](11, 22)

    if (c1 != 33) or (c2 != 33) then
      @pony_exitcode(exit_add + 1)
    end

    let d1 = comptime test_method_value_typeargs[11, 22]() end
    let d2 = test_method_value_typeargs[11, 22]()

    if (d1 != 33) or (d2 != 33) then
      @pony_exitcode(exit_add + 2)
    end

    let e1 = comptime test_generic_struct() end
    let e2 = test_generic_struct()

    if (e1 != 99) or (e2 != 99) then
      @pony_exitcode(exit_add + 3)
    end

  fun test_method_typeargs[T: (Int & Real[T] val), U: (Int & Real[U] val)](v: T, u: U): T =>
    v + T.from[U](u)

  fun test_method_value_typeargs[a: I32, b: I32](): I32 =>
    a + b

  fun test_generic_struct(): I32 =>
    var g1 = G1[I32, U8](11, 22)
    g1.compute[33](33)

  fun test_pointer(exit_add: I32) =>
    let c1 = comptime test_pointer_array() end
    let c2 = test_pointer_array()

    if (c1 != 45) or (c2 != 45) then
      @pony_exitcode(exit_add + 1)
    end

  fun test_pointer_array(): U32 =>
    var p = Pointer[U32].alloc(10)

    for i in Range(0, 10) do
      p.update(i, i.u32())
    end

    var sum: U32 = 0

    for i in Range(0, 10) do
      sum = sum + p.apply(i)
    end

    sum

  fun test_ffi_calls(exit_add: I32) =>
    let c1 = comptime test_memory_ffi_calls() end
    let c2 = test_memory_ffi_calls()

    if (c1 != 0) or (c2 != 0) then
      @pony_exitcode(exit_add + 1)
    end

  fun test_memory_ffi_calls(): I32 =>
    var p1 = Pointer[U8].alloc(10)
    var p2 = Pointer[U8].alloc(10)
    var p3 = Pointer[U8].alloc(10)
    var p4 = Pointer[U8].alloc(10)

    for i in Range(0, 10) do
      p1.update(i, 3)
    end

    @memset(p2, 3, 10)

    @memmove(p3, p2, 10)

    @memcpy(p4, p3, 10)

    @memcmp(p1, p4, 10)

  fun test_match(exit_add: I32) =>
    var c1 = comptime simple_match("T1000") end
    var c2 = simple_match("T1000")

    if (c1 != 2) or (c2 != 2) then
      @pony_exitcode(exit_add + 1)
    end

    c1 = comptime simple_match("T3000") end
    c2 = simple_match("T3000")

    if (c1 != 99) or (c2 != 99) then
      @pony_exitcode(exit_add + 2)
    end

    var d1 = comptime union_match(None) end
    var d2 = union_match(None)

    if (d1 != "none") or (d2 != "none") then
      @pony_exitcode(exit_add + 3)
    end

    d1 = comptime union_match(3) end
    d2 = union_match(3)

    if (d1 != "three") or (d2 != "three") then
      @pony_exitcode(exit_add + 4)
    end

    d1 = comptime union_match(4) end
    d2 = union_match(4)

    if (d1 != "other integer") or (d2 != "other integer") then
      @pony_exitcode(exit_add + 5)
    end

    d1 = comptime union_match("passthrough") end
    d2 = union_match("passthrough")

    if (d1 != "passthrough") or (d2 != "passthrough") then
      @pony_exitcode(exit_add + 6)
    end

    var e1 = comptime tuple_match(None, 33) end
    var e2 = tuple_match(None, 33)

    if (e1 != "none") or (e2 != "none") then
      @pony_exitcode(exit_add + 7)
    end

    e1 = comptime tuple_match("tt", 2) end
    e2 = tuple_match("tt", 2)

    if (e1 != "tt two") or (e2 != "tt two") then
      @pony_exitcode(exit_add + 8)
    end

    e1 = comptime tuple_match("tt", 3) end
    e2 = tuple_match("tt", 3)

    if (e1 != "tt three") or (e2 != "tt three") then
      @pony_exitcode(exit_add + 9)
    end

    e1 = comptime tuple_match("tt", 15) end
    e2 = tuple_match("tt", 15)

    if (e1 != "tt other big integer") or (e2 != "tt other big integer") then
      @pony_exitcode(exit_add + 10)
    end

    e1 = comptime tuple_match("tt", 8) end
    e2 = tuple_match("tt", 8)

    if (e1 != "tt other small integer") or (e2 != "tt other small integer") then
      @pony_exitcode(exit_add + 11)
    end

  fun simple_match(x: String): U32 =>
    match x
    | "T800" => 1
    | "T1000" => 2
    else
      99
    end

  fun union_match(x: (U32 | String | None)): String =>
    match x
      | None => "none"
      | 2 => "two"
      | 3 => "three"
      | let u: U32 => "other integer"
      | let s: String => s
    end

  fun tuple_match(x: (String | None), y: U32): String =>
    match (x, y)
    | (None, _) => "none"
    | (let s: String, 2) => s + " two"
    | (let s: String, 3) => s + " three"
    | (let s: String, let u: U32) if u > 14 => s + " other big integer"
    | (let s: String, _) => s + " other small integer"
    else
      "something else"
    end

  fun test_array(exit_add: I32) =>
    var c1 = comptime array_add() end
    var c2 = array_add()

    if (c1 != 30) or (c2 != 30) then
      @pony_exitcode(exit_add + 1)
    end

  fun array_add(): I32 =>
    let x: Array[I32] = [1; 2; 3; 4; 5]
    let y: Array[I32] = [1; 2; 3; 4; 5]

    var res: I32 = 0

    let iterator_x = x.values()
    let iterator_y = y.values()
    while iterator_x.has_next() and iterator_y.has_next() do
      var x_val: I32 = 0
      var y_val: I32 = 0
      try
        x_val = iterator_x.next()?
        y_val = iterator_y.next()?
        res = res + x_val + y_val
      else
        break 0
      end
    end

    res

  fun test_string(exit_add: I32) =>
    var c1 = comptime string_add("this", " that") end
    var c2 = string_add("this", " that")

    if (c1 != "this that") or (c2 != "this that") then
      @pony_exitcode(exit_add + 1)
    end

    var d1 = comptime string_split_join("1 2 3 4") end
    var d2 = string_split_join("1 2 3 4")

    if (d1 != "1234") or (d2 != "1234") then
      @pony_exitcode(exit_add + 2)
    end

  fun string_add(x: String, y: String): String =>
    x + y

  fun string_split_join(x: String): String =>
    let r = x.split(" ")
    "".join((consume r).values())

  fun test_value_type_param_expr(exit_add: I32) =>
    var c1 = comptime value_param_add_1[comptime 1 + 4 end, 3](5) end
    var c2 = value_param_add_1[comptime 1 + 4 end, 3](5)

    if (c1 != 23) or (c2 != 23) then
      @pony_exitcode(exit_add + 1)
    end

  fun value_param_add_1[x: I32, y: I32](z: I32): I32 =>
    value_param_add_2[= x * y, y](z)

  fun value_param_add_2[y: I32, z: I32](x: I32): I32 =>
    x + y + z

  fun test_constant_object(exit_add: I32) =>
    let c1: S2 val = comptime S2 end
    let c2: S2 val = S2

    if (c1.x != 2020) or (c2.x != 2020) or
       (c1.y != 3030) or (c2.y != 3030) or
       (c1.z.em != 1010) or (c2.z.em != 1010) or
       (c1.z2.em != 1010) or (c2.z2.em != 1010) then
      @pony_exitcode(exit_add + 1)
    end

    var d1 = comptime create_constant_struct() end
    var d2 = create_constant_struct()

    if (d1.x != 2020) or (d2.x != 2020) or
       (d1.y != 3030) or (d2.y != 3030)
       (c1.z.em != 1010) or (c2.z.em != 1010) or
       (c1.z2.em != 1010) or (c2.z2.em != 1010) then
      @pony_exitcode(exit_add + 2)
    end

    // Test that the costant object has been allocated and copied
    // and can be written to when assigned to a TK_REF
    d1.x = 5050
    d1.z.em = 7070
    d1.z2.em = 9090

    if (d1.x != 5050) or (d1.z.em != 7070) or (d1.z2.em != 9090) then
      @pony_exitcode(exit_add + 3)
    end

  fun create_constant_struct(): S2 =>
    S2