use @pony_exitcode[None](code: I32)

primitive PFloat1[A]
  entityif A <: F64 then
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
  entityif A <: FloatingPoint[A] then
    entityif A <: F64 then
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
    PFloat1[F64].float_op(0.5)
    PFloat2[F32].float_op2(0.5)
