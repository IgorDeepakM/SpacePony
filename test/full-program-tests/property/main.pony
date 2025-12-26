use @pony_exitcode[None](code: I32)

primitive P
  enum I32
    e1
    e2
    e3
  end
  
  fun \property\ m1(): I32 => 22

class C
  var x: I32

  new create(x': I32) =>
    x = x'

  fun \property\ get_x(): I32 => x

  fun \property\ partial_x(): I32 ? =>
    if x < 10 then
      error
    else
      x
    end

  fun \property\ xp(): I32 => x
  fun \property\ ref xp_w(x': I32): I32 => x = x'

  fun \property\ ref xpp_w(x': I32)? =>
    if x' < 10 then
      error
    else
      x = x'
    end

class C2
  var c: C = C(11)

actor Main
    let _env: Env
  
  new create(env: Env) =>
    _env = env

    if (P.e1 != 0) or (P.e2 != 1) or (P.e3 != 2) or (P.m1 != 22) then
      @pony_exitcode(1)
    end

    var x = P.e2

    var r: I32 = match x
      | P.e1 => 11
      | P.e2 => 22
      | P.e3 => 33
      else
        100
      end

    if r != 22 then
      @pony_exitcode(2)
    end

    var c1 = C(44)

    if c1.get_x != 44 then
      @pony_exitcode(3)
    end

    var c2 = C(5)

    var c_x: I32 = 0
    try
      c_x = c2.partial_x?
    end

    if c_x != 0 then
      @pony_exitcode(4)
    end

    var c3 = C(55)
    c_x = 0
    try
      c_x = c3.partial_x?
    end

    if c_x != 55 then
      @pony_exitcode(5)
    end

    if c3.xp != 55 then
      @pony_exitcode(6)
    end

    c3.xp = 111

    if c3.x != 111 then
      @pony_exitcode(7)
    end

    try
      c3.xpp? = 222
    else
      @pony_exitcode(8)
    end

    if c3.x != 222 then
      @pony_exitcode(9)
    end

    try
      c3.xpp? = 5
    end

    if c3.x != 222 then
      @pony_exitcode(11)
    end

    var c4: C2 = C2

    c4.c.xp = 333

    if c4.c.x != 333 then
      @pony_exitcode(12)
    end
