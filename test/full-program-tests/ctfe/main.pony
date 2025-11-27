use "collections"
use "files"

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

primitive Something

class Is1
  var x: I32

  new create(x': I32) =>
    x = x'

actor Main
  var _env: Env

  new create(env: Env) =>
    _env = env

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
    test_constant_array(300)
    test_floating_point(320)
    test_load_save_file(340)
    test_primitive_union(360)
    test_is(380)

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
       (d1.z.em != 1010) or (d2.z.em != 1010) or
       (d1.z2.em != 1010) or (d2.z2.em != 1010) then
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

  fun test_constant_array(exit_add: I32) =>
    let c1: Array[USize] val = comptime [1; 2; 3; 4; 5; 6; 7; 8; 9; 10; 11] end

    var i: USize = 1
    for c_elem in c1.values() do
      if c_elem != i then
        @pony_exitcode(exit_add + 1)
      end
      i = i + 1
    end

    let d1: Array[USize] = comptime [1; 2; 3; 4; 5; 6; 7; 8; 9; 10; 11] end

    i = 1
    for c_elem in d1.values() do
      if c_elem != i then
        @pony_exitcode(exit_add + 2)
      end
      i = i + 1
    end

    i = 0
    for c_elem in d1.values() do
      try
        d1.update(i, c_elem + 1000)?
      else
        @pony_exitcode(exit_add + 3)
      end
      i = i + 1
    end

    i = 1001
    for c_elem in d1.values() do
      if c_elem != i then
        @pony_exitcode(exit_add + 4)
      end
      i = i + 1
    end

    // Test homogeneous array
    let e1: Array[USize] val = comptime [33; 33; 33; 33; 33; 33; 33; 33; 33; 33; 33] end

    for c_elem in e1.values() do
      if c_elem != 33 then
        @pony_exitcode(exit_add + 5)
      end
    end

    var f1 = comptime big_generated_array(5000) end
    i = 0
    for c_elem in f1.values() do
      if c_elem.usize() != i then
        @pony_exitcode(exit_add + 6)
      end
      i = i + 1
    end

    var g1 = comptime [as (None | Something): None; Something; Something; None] end
    var g2 = [as (None | Something): None; Something; Something; None]

    let iterator_g1 = g1.values()
    let iterator_g2 = g2.values()
    var sz: USize = 0
    while iterator_g1.has_next() and iterator_g2.has_next() do
      try
        if not (iterator_g1.next()? is iterator_g2.next()?) then
          @pony_exitcode(exit_add + 7)
        end
      else
        @pony_exitcode(exit_add + 8)
      end

      sz = sz + 1
    end

    if sz != 4 then
      @pony_exitcode(exit_add + 9)
    end

    var h1 = comptime [as ((None | Something), U32): (None, 1); (Something, 2); (Something, 3); (None, 3)] end
    var h2 = [as ((None | Something), U32): (None, 1); (Something, 2); (Something, 3); (None, 3)]

    let iterator_h1 = h1.values()
    let iterator_h2 = h2.values()
    sz = 0
    while iterator_h1.has_next() and iterator_h2.has_next() do
      try
        if not (iterator_h1.next()? is iterator_h2.next()?) then
          @pony_exitcode(exit_add + 10)
        end
      else
        @pony_exitcode(exit_add + 11)
      end

     sz = sz + 1
    end

    if sz != 4 then
      @pony_exitcode(exit_add + 12)
    end

  fun big_generated_array(size: USize): Array[U32] val =>
    var ar: Array[U32] iso = Array[U32]

    for i in Range[U32](0, size.u32()) do
      ar.push(i)
    end

    ar

  fun test_floating_point(exit_add: I32) =>
    let x1 = floating_point_arithmetic_test()
    if not x1 then
      @pony_exitcode(exit_add + 1)
    end

    let x2 = floating_point_conversion_test()
    if x2 != 0 then
      @pony_exitcode(exit_add + 2 + x2)
    end


  fun floating_point_arithmetic_test(): Bool =>
    var x1: F64 = comptime (((-((3.0 + (4.0 * -6.0)) / 2.111)) * 33) / -34.2) % 6.1 end
    var x2: F64 = (((-((3.0 + (4.0 * -6.0)) / 2.111)) * 33) / -34.2) % 6.1

    if x1 != x2 then
      return false
    end

    true

  fun floating_point_conversion_test(): I32 =>
    var d2_1: F64 = comptime
      var t2_1: I128 = 0x7FFFFFFF_FFFFFFFF_FFFFFFFF_FFFFFFFF
      t2_1.f64()
    end

    var t1_1: I128 = 0x7FFFFFFF_FFFFFFFF_FFFFFFFF_FFFFFFFF
    var d1_1: F64 = t1_1.f64()

    if d1_1 != d2_1 then
      return 1
    end

    var d2_2: F32 = comptime
      var t2_2: I64 = 0x7FFFFFFF_FFFFFFFF
      t2_2.f32()
    end

    var t1_2: I64 = 0x7FFFFFFF_FFFFFFFF
    var d1_2: F32 = t1_2.f32()

    if d1_2 != d2_2 then
      return 2
    end

    var d2_3: I128 = comptime
      var t2_3: F64 = 123456789.123456789
      t2_3.i128()
    end

    var t1_3: F64 = 123456789.123456789
    var d1_3: I128 = t1_3.i128()

    if d1_3 != d2_3 then
      return 3
    end

    var d2_4: I64 = comptime
      var t2_4: F64 = 123456789.123456789
      t2_4.i64()
    end

    var t1_4: F64 = 123456789.123456789
    var d1_4: I64 = t1_4.i64()

    if d1_4 != d2_4 then
      return 4
    end

    var d2_5: U32 = comptime
      var t2_5: F64 = 123456789.123456789
      t2_5.u32()
    end

    var t1_5: F64 = 123456789.123456789
    var d1_5: U32 = t1_5.u32()

    if d1_5 != d2_5 then
      return 5
    end

    0

  fun test_load_save_file(exit_add: I32) =>
    comptime
      var s11 = String.from_array(CompTime.load_file(CompTimeWorkingDirectory, "ctfe_test_file.txt"))
      var s22: String = recover val s11.reverse() end
      CompTime.save_file(s22.array(), CompTimeOutputDirectory, "ctfe_test_file_2.txt")
    end

    // Read the file again in runtime to verify its contents.
    // The file that was created during comptime was written to the output
    // directory of the executable and therefore we need the path to executable
    // which is in the first command line argument.
    var p: String = String
    try
      p = _env.args(0)?
    end

    var dir = Path.dir(p)

    var file_path = "ctfe_test_file_2.txt"
    if dir != p then
      file_path = Path.join(dir, file_path)
    end

    var path = FilePath(FileAuth(_env.root), file_path)
    try
      with f = OpenFile(path) as File do
        var ar: Array[U8] val = f.read(10)

        if ar.size() != 10 then
          @pony_exitcode(exit_add + 1)
        end

        var s33: String = String.from_array(ar)

        if s33 != "9876543210" then
          @pony_exitcode(exit_add + 2)
        end
      end
    else
      @pony_exitcode(exit_add + 3)
    end

  fun test_primitive_union(exit_add: I32) =>
    var c1 = comptime union_type_return(0) end

    if match_union(c1) != 0 then
      @pony_exitcode(exit_add + 1)
    end

    c1 = comptime union_type_return(1) end

    if match_union(c1) != 1 then
      @pony_exitcode(exit_add + 2)
    end

    c1 = comptime union_type_return(33) end

    if match_union(c1) != 33 then
      @pony_exitcode(exit_add + 3)
    end

    var d1 = comptime match_union(union_type_return(0)) end

    if d1 != 0 then
      @pony_exitcode(exit_add + 4)
    end

    d1 = comptime match_union(union_type_return(1)) end

    if d1 != 1 then
      @pony_exitcode(exit_add + 5)
    end

    d1 = comptime match_union(union_type_return(33)) end

    if d1 != 33 then
      @pony_exitcode(exit_add + 6)
    end

  fun union_type_return(x: I32): (None | Something | I32) =>
    match x
    | 0 => None
    | 1 => Something
    else
      x
    end

  fun match_union(xx: (None | Something | I32)): I32 =>
    match xx
    | None => I32(0)
    | Something => I32(1)
    else
      try
        (xx as I32)
      else
        I32(0xffff)
      end
    end

  fun test_is(exit_add: I32) =>
    var d1: Bool = comptime
      var e1 = S2
      var e2 = e1
      e1 is e2
    end

    var c11 = S2
    var c22 = c11
    var d2: Bool = c11 is c22

    if d1 != d2 then
      @pony_exitcode(exit_add + 1)
    end

    d1 = comptime
      var e1 = S2
      var e2 = S2
      e1 is e2
    end

    var e11 = S2
    var e22 = S2
    d2 = e11 is e22

    if d1 != d2 then
      @pony_exitcode(exit_add + 2)
    end

    d1 = comptime
      var e1 = Is1(2)
      var e2 = Is1(2)
      e1 is e2
    end

    var e11_2 = Is1(2)
    var e22_2 = Is1(2)
    d2 = e11_2 is e22_2

    if d1 != d2 then
      @pony_exitcode(exit_add + 3)
    end

    d1 = comptime
      var e1 = Is1(1)
      var e2 = Is1(2)
      e1 is e2
    end

    var e11_3 = Is1(1)
    var e22_3 = Is1(2)
    d2 = e11_3 is e22_3

    if d1 != d2 then
      @pony_exitcode(exit_add + 4)
    end

    d1 = comptime
      var e1: (U32, I32) = (1, 2)
      var e2: (U32, I32) = (1, 2)
      e1 is e2
    end

     var e11_4: (U32, I32) = (1, 2)
     var e22_4: (U32, I32) = (1, 2)
     d2 = e11_4 is e22_4

    if d1 != d2 then
      @pony_exitcode(exit_add + 5)
    end

    d1 = comptime
      var e1:(U32, I32) = (2, 1)
      var e2:(U32, I32) = (1, 2)
      e1 is e2
    end

    var e11_5: (U32, I32) = (2, 1)
    var e22_5: (U32, I32) = (1, 2)
    d2 = e11_5 is e22_5

    if d1 != d2 then
      @pony_exitcode(exit_add + 6)
    end