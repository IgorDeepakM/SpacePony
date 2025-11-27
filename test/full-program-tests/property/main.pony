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