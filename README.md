# SpacePony

SpacePony is an experimental fork of the [Pony programming language](https://github.com/ponylang/ponyc). The goal of the fork is to improve the FFI capabilities and add more systems programming language features.

* [Quick start](#quick-start)
  * [Linux/Macos](#linuxmacos)
  * [Windows](#windows)
* [Breaking changes from original Pony](#breaking-changes-from-original-pony)
* [List of additions/changes](#list-of-additionschanges)
  * [Identify SpacePony](#identify-spacepony)
  * [Pointer](#pointer)
  * [addressof](#addressof)
  * [Constant values in generics](#constant-values-in-generics)
  * [CFixedSizedArray](#cfixedsizedarray)
  * [offsetof](#offsetof)
  * [sizeof](#sizeof)
  * [FFI pass by value parameters](#ffi-pass-by-value-parameters)
  * [Inline assembler support](#inline-assembler-support)
  * [Atomics](#atomics)
  * [Additional capability, nhb](#additional-capability-nhb)
  * [CTFE (Compile Time Function Execution)](#ctfe-compile-time-function-execution)
  * [Enums](#enums-sort-of)
  * [Property](#property)
  * [TypeTrait](#typetrait)
* [Future directions](#future-directions)
  * [Short term](#short-term)
  * [Long term (read never)](#long-term-read-never)
* [The original Pony README is here with instructions how to compile](#the-original-pony-readme-is-here-with-instructions-how-to-compile)

## Quick start

SpacePony currently doesn't have any official binary releases, however it is easy to clone this repo and build the compiler.

* First clone this repo in any subdirectory.

### Linux/Macos

By default SpacePony uses Clang as the compiler. Also Cmake and Python3 is needed.

* In the base directory type:
  * **make libs build_flags="-j6"** (this will build associated libraries such as LLVM, this will take a while. Adjust the -j6 option to the number of cores you want to use)
  * **make configure**
  * **make build**

### Windows

By default SpacePony uses the MSVC compiler. Also Cmake and Python3 is needed. Everything can be installed using MS Visual Studio. Open a Powershell terminal in order to run the make script, simplest is to open the "Developer PowerShell for VS 202x" where all the compiler stuff is added to the path.

* In the base directory type:
  * **.\make.ps1 libs** (this will build associated libraries such as LLVM, this will take a while)
  * **.\make.ps1 configure**
  * **.\make.ps1 build**

The compiled output including the ponyc compiler is located in the build/release directory.

Did I miss anything? This guide will tell you more [Building from source](BUILD.md)

## Breaking changes from original Pony

* While SpacePony is compatible with original Pony, it is inevitable that SpacePony and Pony will diverge more and more. Several keywords have been added which might be used in Pony, cannot be used in SpacePony. This is a list of breaking changes that you might need to change in order to adapt Pony source code SpacePony.

  * In the class `Iter` in the package `itertools`, the method `enum` was renamed to `enumerate` because `enum` is a reserved keyword in SpacePony.

## List of additions/changes

### Identify SpacePony

* Added a build flag `spacepony` in order to identify SpacePony.

  ```pony
  ifdef spacepony then
    env.out.print("This is spacepony")
  else
    env.out.print("This is not spacepony")
  end
  ```

### Pointer

* The Pointer type can be used **everywhere** not only as FFI function arguments.

* It can be initialized using from_usize(addr: USize) in order to initialize a Pointer with any desired number.

  ```pony
  var my_ptr = Pointer[None].from_usize(0x12345678)
  ```

* It is possible to obtain the Pointer value as a USize with the usize method. Useful for printing the pointer value for example.

  ```pony
  env.out.print("Address is " + Format.int[USize](ptr.usize(), FormatHex))
  ```

* The convert method is made public in order to make it possible to cast one pointer type to another.

  ```pony
  var ptr1: Pointer[MyStruct1] = ...
  var ptr2: Pointer[MyStruct2] = ptr1.convert[MyStruct2]()
  ```

* Added to_reftype method to convert the Pointer to the underlying reference type. This is equivalent to the apply method of the NullablePointer type.

  ```pony
  var ptr: Pointer[MyStruct] = ...
  var my_struct: MyStruct = MyStruct_
  try
    my_struct_: MyStruct = ptr.to_reftype()?
  else
    ...
  end
  ```

* Added to_reftype_no_check method to convert the Pointer to the underlying reference type without any null pointer check.

  ```pony
  var ptr: Pointer[MyStruct] = ...
  var my_struct: MyStruct = ptr.to_reftype_no_check()
  ```

* Added pointer_at_index method in order to obtain a pointer at a specific index. Note that this depends on the type of Pointer.

  ```pony
  var ptr: Pointer[U32] = ...
  var ptr2 = ptr.pointer_at_index(3) // Gives a new pointer with a byte offset of ptr + 4 * 3
  ```

* Added from_reftype constructor in order to create a pointer from a class or struct.

  ```pony
  var ptr: Pointer[MyStruct].from_reftype(my_struct)
  ```

* Added from_any constructor in order to create a pointer from anything of the desired type. This is like a cast assign.

  ```pony
  var ptr: Pointer[U32].from_any[MyStruct](my_struct)
  ```

* A class or a struct can be implicitly converted to a Pointer. A Pointer[None] will accept any class or struct type.

  ```pony
  struct S

  ...

  let s: S = S
  var ps: Pointer[S] = s
  var ps2: Pointer[None] = s

  ```

### addressof

* addressof can be used **everywhere**

* It is possible to use addressof of an FFI function. The type is a bare lambda. This is useful for populating bare lambdas in structs to emulate C++ abstract classes if it is necessary to initialize them in Pony.

  ```pony
  use @FFI_Func[None](x: I32, y: I32)

  ...

  var f = addressof @FFI_Func
    f(3, 7)
  ```

### Constant values in generics

* It is possible to use constants (literals) as type arguments in generics, both as type arguments for types and type arguments for functions.

* Much of the work for constants in generics was done in ponyta (https://github.com/lukecheeseman/ponyta).

  ```pony
  class TT[n: USize, name: String]
    fun get_sum(k: USize): USize =>
      n * k

    fun get_name_(): String =>
      name

    fun get_sum_const[k: USize](): USize =>
      n * k

  ...

  var tt: TT[3, "constant string"] = TT[3, "constant string"]
  ```
* Default value type arguments are supported, both in function type arguments and class/struct type arguments.

  ```pony
  fun get_sum_const[k: USize = 4](): USize =>
    n * k
  ```

### CFixedSizedArray

* Added CFixedSizedArray in order to be able to model C fixed sized arrays found in C.

  ```c
  struct S
  {
    size_t size;
    char buf[512];
  }
  ```

  Can be modelled in Pony as

  ```pony
  struct S
    var size: USize = 0
    embed buf: CFixedSizedArray[U8, 512] = CFixedSizedArray[U8, 512](0)
  ```
  make sure to use **embed** in order to expand the array in the struct. Using `var buf: CFixedSizedArray[U8, 512]` would be the same `char (*buf)[512]` and Pony would allocate the fixed sized array on the heap.

  It is possible to emulate a C flexible array member (https://en.wikipedia.org/wiki/Flexible_array_member) that is allocated in C. The Array class can be loaded using the CFixedSizedArray.

  ```pony
  struct S
    var size: USize = 0
    embed buf: CFixedSizedArray[U8, 1] = CFixedSizedArray[U8, 1](0)

  ...

  var s = @get_struct()

  ...

  var ar = Array.from_cpointer(s.buf.cpointer(), s.size)
  ```

### offsetof

* Added offsetof in order to get an offset of a member in a struct or class.

  ```pony
  struct S
    var x: USize
    var y: USize

  ...

  var s = S
  let off_y = offsetof s.y // Gets the offset of y
  let off_y_2 = offsetof S.y // It is possible to use the type directly as base
  ```


### sizeof

* Added sizeof operator to obtain the size of any type. Also works on nested types.

  ```pony
  struct S
    var x: USize
    var y: USize

  ...

  var s = S
  let sz_u32 = sizeof U32 // Any base type can be used
  let sz_s = sizeof s // Gets the size of the struct s
  let sz_s_2 = sizeof S // Type can be used directly as base in a dot expression
  let sz_s_y = sizeof s.y // Size of members can be used
  let sz_s_y_2_ = sizeof S.y // Also with the type directly as base
  ```

* Keep in mind that both sizeof and offsetof are not compile time constants, meaning they do not behave like a literal and they can unfortunately not be used as type values in generics. sizeof/offsetof are created during the code generation step becoming a compile time constant in the LLVM code and not before that. The reason for this is the the SpacePony compiler uses LLVM in order build target dependent aggregate types in the code generation pass which is one of the last passes. It is possible to make sizeof and offsetof into a literal but that would require using LLVM to build up the types in earlier passes.

### FFI pass by value parameters

* Structs and classes can be passed as value to FFI functions. Add the annotation `\byval\` before the type in the parameter declaration. Annotation was used because it could be easily added without intruding too much on the existing Pony syntax. It also can coexists with other annotations disregarding the order. 

* Note that this needs to be manually added for each CPU and OS target as there is currently no functionality in LLVM that lowers the parameters. For more information read [this](https://yorickpeterse.com/articles/the-mess-that-is-handling-structure-arguments-and-returns-in-llvm/) article. There are currently discussions to add an ABI lowering library to LLVM, [[RFC] An ABI lowering library for LLVM](https://discourse.llvm.org/t/rfc-an-abi-lowering-library-for-llvm/84495/23). If successful and this library is mature, SpacePony will transition to this library because it makes sense. Until then expect bugs as the lowering is tricky and full of corner cases.

* Currently supported targets:
  * x86-64 Linux, Windows
  * Aarch64 Linux, Macos
  * Arm32 EABI

* These are not supported or tested but should be easy to get working:
  * x86-32 Windows
  * x86-64 Macos, which should be System V ABI for x86-64 just as Linux.

  ```pony
  use @FFI_Func[\byval\ S](x: \byval\ S, y: \byval\ S)

  ...

  var s1 = S
  var s2 = S
  var ret = FFI_Func(s1, s2)
  ```

* Bare lambdas also accepts pass by value parameters.

  ```pony
  let my_lambda: @{(\byval\ S, \byval\ S): \byval\ S} =
      @{(x: \byval\ S, y: \byval\ S): \byval\ S =>
        var ret = S
        ret.x = x.x + y.x
        ret
      }

  ...

  var s1 = S
  var s2 = S
  var ret = my_lambda(s1, s2)
  ```

* Note pass by value in lambdas is currently essentially a double copy (only inside SpacePony). First the argument is copied to the stack and then it is copied to a heap allocated structure. Why? Because there is no escape analysis and the passed aggregate can be sent or stored somewhere and because of that it cannot be on the stack. There is room for future optimizations regarding this, similar to how Pony can allocate on stack rather than heap.

* Currently not supported is the C `const` qualifier. This might affect lowering for some targets and therefore it must taken into account in the future. To map the `const` qualifier in SpacePony, one possibility to have `let` be const in the C FFI. However, this doesn't cover everything as `embed` also might be const. Adding an annotation `\cconst\` to the type can cover this.

### Inline assembler support

* Added support for an LLVM like syntax for inline assembler.

* The syntax is `asm <assembler_string>, <conststraints>, [return_type](parameters...) end`. For more information regarding the LLVM constraints syntax please go to [Inline Assembler Expressions](https://llvm.org/docs/LangRef.html#inline-assembler-expressions)

  ```pony
  var res = asm
    "mov eax, $1\n
     mov ebx, $2\n
     mul ebx\n
     mov $0, eax\n",
    "=r,r,r,~{eax},~{ebx},~{edx}",
    [U32](x1, x2)
  end
  ```

* Multiple return values can be achieved by making the return type a tuple.

  ```pony
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
  ```

### Atomics

* Added an atomic library in the built in package. The atomic type was modelled after std::atomic in C++.

  ```pony
  var a: Atomic[U32] = Atomic[U32](10)

  var r = a + 1 // same as a.add_fetch(1)
  ```

### Additional capability, nhb

* Added the extra capability called **nhb**. The nhb capability makes it possible to mutably share a class among actors. This is intended for when it is possible to mutably share a class when the class has implemented some form of custom synchronization like atomics or mutexes. 

  ```pony
  class NhbClass
    var x: U64

  actor TA
    var _nc: NhbClass nhb

    new create(nc: NhbClass nhb) =>
      _nc = nc

    be beh(nh: NhbClass nhb) =>
      _nc = nh

  actor Main
    let _env: Env
    var _ta: TA
    var _ta2: TA
    var _nc: NhbClass nhb

    new create(env: Env) =>
      _env = env

      _nc = NhbClass

      _ta = TA(_nc)
      _ta2 = TA(_nc) // Yes you can give _nc to several actors

      var nc2: NhbClass nhb = NhbClass
      _ta.beh(nc2)
      _ta2.beh(nc2) // Yes, you can send that nhb class all you want, over and over again. Have fun.
    ```

### CTFE (Compile Time Function Execution)

* Added CTFE to the pony compiler. Currently it is only supported using a `comptime expression end` statement. The compile time evaluation is mandatory inside this expression and a failure to evaluate the expression at compile time will result that the compilation fails. This corresponds to `consteval` in C++.

* Example:
  ```pony
  var x1: ILong = comptime count_to(10) end

  ...

  fun count_to(r: ILong): ILong =>
    var tt: ILong = 0
 
     while tt < r do
       tt = tt + 1
     end

     tt
  ```
  This will be run at compile and be reduced to a single literal of type `Ilong`.


* The CTFE implementation is currently in its early stages and not all expressions and types are supported yet.

* The CTFE has taken some ideas and code from (https://github.com/lukecheeseman/ponyta) but most of the implementation is completely new. The `#postexpr` in ponyta will not be used at all and the `#` character can be used for future purposes instead.

* It is possible to go really far with CTFE and the goal is to support as many expressions and types as possible. CTFE might be added to more places than only `comptime expression end`, which means it is also possible to try running CTFE at key places in the code. It is also possible to use CTFE in order to build up string mixins, self generating code. The D language has string mixins which can be used as a tool for meta-programming.

* Note that the purpose of CTFE is not really optimizations but rather a guarantee that an expression can be evaluated at compile time. LLVM already does constant folding and can do the much of same job as CTFE. One important decision to add CTFE was to be able to have expressions in value type parameters in generics.

* With CTFE it is possible to generate value type literals from compile time expressions.

  ```pony
  class C1[n: I32]

  fun gen_C1[u: I32, v: I32]()
    C1[comptime u + v end]
  ```

  Having `comptime` inside type arguments might be too verbose and unaesthetic so it is possible to use `=` in front of the expression instead

  ```pony
  fun gen_C1[u: I32, v: I32]()
    C1[= u + v]
  ```

  Why having `=` in front of the expression and not the expression directly? Unfortunately it is a parsing technicality, the `=` is needed for making the parser selecting the correct rule that otherwise would ambiguous.

  One big problem with expressions in the type arguments is that there is no type check when they are used. Right now it just accept a type comparison as soon as an expression is encountered. The problem is that the type checks are done in passes prior to the reach pass where the CTFE is being run. Literals can be easily checked for equality, but not an expression that has not been reduced to a literal. Comparing an AST tree is too difficult, take the following example `C1[= a + b + c] is C1[= c + b + a]` which is potentially the same type but a different expression yields later in the reach pass the same result in the type argument. This was unresolved in ponyta and currently also unresolved in SpacePony. Hopefully there will be a solution to this in the future.

* It is possible to load and save file during compile time inside a `comptime` expression using `CompTime.load_file` and `CompTime.save_file`. What is loaded in compile time can be processed at compile time but also during runtime because the compiler can create constant object(s) of what is loaded. However, when saving data at compile time, the data must be processed at compile time for obvious reasons.

  ```pony
  comptime
    CompTime.save_file("123456".array(), CompTimeOutputDirectory, "output.txt")
  end
  ```

  ```pony
  var loaded_array = comptime
    CompTime.load_file(CompTimeWorkingDirectory, "input.txt") // This loads the data and creates a constant array object
                                                              // Further processing can be made at compile time in order to   
                                                              // make other constant objects based on the input data.
  end
  ```

  ```pony
  comptime
    var array = CompTime.load_file(CompTimeWorkingDirectory, "input.txt") // Load the data

    ... // do some processing

    CompTime.save_file(array, CompTimeOutputDirectory, "output.txt") // Save processed data
  end
  ```

### Enums (sort of)

* Added the possibility using enum like declarations inside a primitive. The Pony language doesn't have enums and the goto method is to use methods inside a primitive [like this](https://tutorial.ponylang.io/appendices/examples.html#enumeration-with-values). However, this adds a lot of boiler plate to just write an enum which is very tedious for large amounts of enums and there is no auto increment of the value. Adding a completely new enum type classification in SpacePony is a lot of work so instead a syntax that lowers the enumerations to methods inside the primitive was chosen. Unlike C/C++ enums, the enums must be give a type and there is no automatic type inference to the smallest possible type that fits the enumerations.

  ```pony
  primitive P
    enum I32
      enum1 = 44
      enum2
      enum3
    end
  ```

* Becomes after the lowering, equivalent to this.

  ```pony
  primitive P
    fun \property\ enum1(): I32 => 44
    fun \property\ enum2(): I32 => 44.add(1) // 45
    fun \property\ enum3(): I32 => 44.add(2) // 46
  ```

 * When there are many enums, this makes it more easier to write. C interfaces can sometimes have many enums, often as some return status values and the ability to copy and paste most of it makes it easier.

 * Any type can be used in enums, however for auto-incrementation the type must support the add method. If not every enum must be given a value.

 * Several enums per line are supported using `;` delimiter

   ```pony
   primitive P
     enum I32
       enum1 = 1; enum2; enum3
     end
   ```

 * By using lowering and using existing primitives for implementing enums was the path of least resistance. Adding "real" enums would require much more work in order to make it work with the existing type system. Also the Pony language already support enums using a union of types.

   ```pony
    primitive P1
    primitive P2
    primitive P3
    primitive P4

    type PUnion is (P1 | P2 | P3 | P4)
    ```

* The Pony type unions are ok for a moderate amount enums, both in terms of typing and code generation. This is usually the goto method when the is no interest what the underlying representation is, meaning no conversion to some integer needed. Under the hood a primitive is a pointer to a global aggregate, like a class but without any members. Since there are no members it can be made an immutable global. For large amounts of enums, there might be missed optimizations opportunities as random pointers cannot easily be converted to jump tables. In this particular case monotonous increasing enums might be better. Despite having implemented enums using lowering in primitives, a "real" enum in SpacePony isn't off the table.


### Property

* Added an `property` annotation for methods in order to remove the requirement to have parentheses for method calls that has no parameters and returns a value. This is especially useful for enums that are under the hood are methods but gets the look and feel as it was constant variables. It is similar what is available by default in the D language and C# has properties as well, descibed in the [C# programming guide](https://learn.microsoft.com/en-us/dotnet/csharp/programming-guide/classes-and-structs/properties)

  * *A property is a member that provides a flexible mechanism to read, write, or compute the value of a data field. Properties appear as public data members, but they're implemented as special methods called accessors.*

* The SpacePony enums inside primitives are automatically given the `property` annotation. Any method that with the right signature can have the 'propery' annotation.

  ```pony
  primitive P
    enum I32
      e1 // enums will have the 'property' annotation by default
      e2
      e3
    end

    fun \property\ m1(): I32 => 22 // A method can be a property member

  class C
    var x: I32

    new create(x': I32) =>
      x = x'

    fun \property\ get_x(): I32 => x // It is not limited to literals and can
                                     // return any run time calculation

  ...

  let x = P.e1   // derferencing a property method looks like accessing a member variable
  let y = P.m1

  let c1 = C(33)
  let z = c1.get_x

  ```

* A property can also be a partial. Just add `?` after the name.

  ```pony
  class C
    var x: I32

  new create(x': I32) =>
    x = x'

  fun \property\ partial_x(): I32 ? =>
    if x < 10 then
      error
    else
      x
    end

  ...

  var c = C(5)

  var c_x: I32 = 0
  try
    c_x = c2.partial_x?
  else
    c_x = 11
  end

  // c_x will have the value 11

  ```

* Currently only reading a `property` is supported. However, in the future also writing to a `property` will be added.


### TypeTrait

* TypeTrait is a primitive that has a set of methods that operates on types in order to identify certain type properties. The name is obviously influenced by <type_traits> in C++ and its functionality is similar. Previously Pony had `iftype` that checked if a type was a subtype of another type however this functionality was limited and couldn't easily be expanded. What was not possible with `iftype` was to check the underlying type, for example if type is a struct or a class which is possible with TypeTrait.

  ```pony
  if TypeTrait.is_struct[S]() then
    env.out.print("S is struct")
  else
    env.out.print("S is not struct")
  end
  ```

* Why was TypeTrait added? First `iftype` couldn't detect underlying types like classes or primitives. `iftype` also can't support several expressions like `iftype (A <: B) and (C <: D)`. Note that currently because of the Pony type system, all `if` branches will be type checked regardless of the compile time expression evaluation unlike in C++ and D that might skip the unused branch (or as it appears for the programmers). The ultimate goal is to create something similar to `if constexpr` in C++ and `static if` in D, however how the Pony compiler works this goal might be diffifcult.


## Future directions

### Short term

* Right now there is a class called FixedSizedArray which is copy of CFixedSizedArray but implemented as a class with a type descriptor. However, since it is already "spoiled" with a an extra class member, why not add a size field as well. In this case FixedSizedArray can be converted to a FixedCapacityArray instead. It is unclear what is the most useful, if FixedCapacityArray or FixedSizedArray are necessary at all in addition to CFixedSizedArray.

* Since there are suddenly several different arrays, a Slice class might be needed. This would be equivalent to std::span in C++. However, it is also possible to reuse the Array class for this purpose as it is possible to load the Array with outside raw pointers. This is possible because the Array class use garbage collected pointers which can co-exist with foreign raw pointers. The D language has chosen this approach where there is a merge between the slice and the dynamic array. Slices or reuse Array as both their pros and cons.

* Real asynchronous IO and not a POSIX like wrapper. An API that can be used for anything streaming like Files, HTTP, TCP. The API should also use the best available asynchronous OS API primitives.

* Implement a good and comprehensive reflection interface.

* Replace `use` for FFI definitions to `extern`. Right now the keyword `use` is reused for defining external C calls, with additional conditions trailing the definition. These conditions can easily get out of and become unreadable. For example

  ```pony
  use @pony_os_writev[USize](ev: AsioEventID, iov: Pointer[(Pointer[U8] tag, USize)] tag, iovcnt: I32) ? if not windows
  ```

  As different targets increases it will be `if not windows and not anothertarget and not yetanothertarget`. Instead choosing what should be defined should be chosen by `ifdef` similar to `#ifdef` in C. Instead it should be

  ```pony
  ifdef windows then
    extern(C) @_write[I32](fd: I32, buffer: Pointer[None], bytes_to_send: I32)
  end
  ```

  this allows for more complex conditional statements that are also readable. Notice how the example above uses `extern(C)` instead of `use`. The reason is that currently `use` cannot be extended. For example if supporting importing C++ functions, then `use` at the current form cannot be extended. It might be possible to extend `use` with `use(C++)` for example but how it can be extended remains an open question.

### Long term (read never)

* Move to MLIR backend

* GPU offload. The capability/actor model of Pony is very well suited for running parts of the actors in GPUs.

* Improve the Pony type system.


## The original Pony README is here with instructions how to compile

* [Pony README](README_old_Pony.md)
