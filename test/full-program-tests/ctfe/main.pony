use @pony_exitcode[None](code: I32)
use @printf[I32](fmt: Pointer[U8] tag, ...)

actor Main
  new create(env: Env) =>
    test_literal_int(0)
    test_repeat(20)


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

     if x1 != x2 then
       @pony_exitcode(exit_add + 1)
     end

     x1 = comptime repeat_with_else(10) end
     x2 = repeat_with_else(10)

     if x1 != x2 then
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
	   