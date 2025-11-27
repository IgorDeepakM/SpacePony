use @pony_exitcode[None](code: I32)

primitive P1
  enum I32
    e1
    e2; e3 = 11 + 33
    e4
    e11 = 11
  end

  fun in_the_middle(): I32 => 111

  enum String
    se1 = "string1"
    se2 = "string2"
    se3 = "string3"
  end

actor Main
  new create(env: Env) =>
    if (P1.e1() != 0) or (P1.e2() != 1) or (P1.e3() != 44) or (P1.e4() != 45) or (P1.e11() != 11) then
      @pony_exitcode(1)
    end

    if (P1.se1() != "string1") or (P1.se2() != "string2") or (P1.se3() != "string3") then
      @pony_exitcode(2)
    end

