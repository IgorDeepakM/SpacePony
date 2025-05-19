use @pony_exitcode[None](code: I32)


actor Main
  new create(env: Env) =>

    let x1: U32 = 3
    let x2: U32 = 4
	 
    var res: U32 = 0
    ifdef x86 then
      res = asm
        "mov eax, $1\n
		     mov ebx, $2\n
		     mul ebx\n
		     mov $0, eax\n",
        "=r,r,r,~{eax},~{ebx},~{edx}",
        [U32](x1, x2)
      end
    elseif arm and lp64 then
      res = asm
        "mov x9, $1\n
         mov x10, $2\n
         umull x11, w9, w10\n
         mov $0, x11\n",
        "=r,r,r,~{x9},~{x10},~{x11}",
        [U32](x1, x2)
      end
    else
      // We can not test every CPU architecture and only the popular ones so if 
      // the target CPU is not present here just fake it with the correct result.
      res = 12
    end

    if res != 12 then
      @pony_exitcode(1)
      return
    end

    let y1: U32 = 7
    let y2: U32 = 2

    var quot: U32 = 0
    var rem: U32 = 0
    ifdef x86 then
      (quot, rem) = asm
        "mov eax, $2\n
         mov edx, 0\n
		     mov ebx, $3\n
		     div ebx\n
		     mov $0, eax\n
         mov $1, edx\n",
        "=r,=r,r,r,~{eax},~{ebx},~{edx}",
        [(U32, U32)](y1, y2)
      end
    elseif arm and lp64 then
      (quot, rem) = asm
        "mov x9, $2\n
         mov x10, $3\n
         udiv x11, x9, x10\n
         mov $0, x11\n
         msub x12, x11, x10, x9\n
         mov $1, x12\n",
        "=r,=r,r,r,~{x9},~{x10},~{x11},~{x12}",
        [(U32, U32)](y1, y2)
      end
    else
      // We can not test every CPU architecture and only the popular ones so if 
      // the target CPU is not present here just fake it with the correct result.
      (quot, rem) = (U32(3), U32(1))
    end

    if (quot != 3) or (rem != 1) then
      @pony_exitcode(2)
      return
    end