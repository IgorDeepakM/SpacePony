use @pony_exitcode[None](code: I32)

trait T
class C1 is T
class C2 is T

struct S1
primitive P1

primitive PFloat1[A]
  iftype A <: F64 then
    fun float_op(x : A) =>
      x.acos()
  elseif A <: F32 then
    fun float_op2(x : A) =>
      None
  else
    fun float_op3(x: A) =>
      None
    fun float_op4(x: A) =>
      None
  end

primitive PFloat2[A: (Real[A] val & Number)]
  iftype A <: FloatingPoint[A] then
    iftype A <: F64 then
      fun float_op(x: A): A =>
        x.acos()
    elseif A <: F32 then
      fun float_op2(x: A) =>
        None
    else
      fun float_op3(x: A) =>
        None
      fun float_op4(x: A) =>
        None
    end
  end

actor Main
  new create(env: Env) =>
    if not foo[C2](C2) then
      @pony_exitcode(1)
    end

    if not is_class[C1]() then
      @pony_exitcode(2)
    end

    if not is_struct[S1]() then
      @pony_exitcode(3)
    end

    if not is_primitive[P1]() then
      @pony_exitcode(4)
    end

    if is_struct[C1]() then
      @pony_exitcode(5)
    end

    if is_primitive[C1]() then
      @pony_exitcode(6)
    end

    if is_class[S1]() then
      @pony_exitcode(7)
    end

    if is_primitive[S1]() then
      @pony_exitcode(8)
    end

    if is_class[P1]() then
      @pony_exitcode(9)
    end

    if is_struct[P1]() then
      @pony_exitcode(10)
    end

    if not is_class_or_primitive[C1]() then
      @pony_exitcode(11)
    end

    if not is_class_or_primitive[S1]() then
      @pony_exitcode(12)
    end

    if is_class_or_primitive[P1]() then
      @pony_exitcode(13)
    end

    PFloat1[F64].float_op(0.5)
    PFloat2[F32].float_op2(0.5)

  fun foo[A: T](x: A): Bool =>
    iftype A <: C1 then
      return false
    elseif A <: C2 then
      return true
    end

    false

  fun is_class[A: AnyNoCheck](): Bool =>
    iftype A <: class then
      true
    else
      false
    end

  fun is_struct[A: AnyNoCheck](): Bool =>
    iftype A <: struct then
      true
    else
      false
    end

  fun is_primitive[A: AnyNoCheck](): Bool =>
    iftype A <: primitive then
      true
    else
      false
    end

  fun is_class_or_primitive[A: AnyNoCheck](): Bool =>
    iftype A <: (class | struct) then
      true
    else
      false
    end