# SpacePony

SpacePony is an experimental fork of the [Pony programming language](https://github.com/ponylang/ponyc). The goal of the fork is to improve the FFI capabilities and add more systems programming language features.

## List of changes.

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

* Added from_any constructor in order to create a pointer from anything of the desired type. This is like a cast assign.

  ```pony
    var ptr: Pointer[U32].from_any[MyStruct](my_struct)
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

  It is possible to emulate a C flexible array member (https://en.wikipedia.org/wiki/Flexible_array_member) in Pony. The Array class can be loaded using the CFixedSizedArray.

  ```pony
  struct S
    var size: USize = 0
    embed buf: CFixedSizedArray[U8, 1] = CFixedSizedArray[U8, 1](0)

  ...

  var s = S

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

* Keep in mind that both sizeof and offsetof are not compile time constants, meaning they do not behave like a literal. They can unfortunately not be used as type values in generics. They created during the code generation step becoming a compile time constant in the LLVM code and not before that. The reason for this is the the SpacePony compiler use LLVM in order build target dependent aggregate types in the code generation pass which is one of the last passes. It is not impossible to make sizeof and offsetof into a literal but that require using LLVM to build up the types in earlier passes.

### Added FFI pass by value parameters.

* Structs and classes can be passed as value to FFI functions. Add the annotation `\passbyvalue\` before the type in the parameter declaration. Annotation was used because it could be easily added without intruding too much on the existing Pony syntax. It also can coexists with other annotations disregarding the order. 

* Note that this needs to be manually added for each CPU and OS target as there is currently no functionality in LLVM that lowers the parameters. For more information read [this](https://yorickpeterse.com/articles/the-mess-that-is-handling-structure-arguments-and-returns-in-llvm/) article. There are currently discussions to add an ABI lowering library to LLVM, [[RFC] An ABI lowering library for LLVM](https://discourse.llvm.org/t/rfc-an-abi-lowering-library-for-llvm/84495/23). If successful and this library is mature, SpacePony will transition to this library because it makes sense. Until then expect bugs as the lowering is tricky and full of corner cases.

* Currently supported targets:
  * x86-64 Linux, Windows
  * Aarch64 Linux, Macos
  * Arm32 EABI

* These are not supported or tested but should be easy to get working:
  * x86-32 Windows
  * x86-64 Macos, which should be System V ABI for x86-64 just as Linux.

  ```pony
  use @FFI_Func[\passbyvalue\ S](x: \passbyvalue\ S, y: \passbyvalue\ S)

  ...

  var s1 = S
  var s2 = S
  var ret = FFI_Func(s1, s2)
  ```

* Bare lambdas also accepts pass by value parameters.

  ```pony
  let my_lambda: @{(\passbyvalue\ S, \passbyvalue\ S): \passbyvalue\ S} =
      @{(x: \passbyvalue\ S, y: \passbyvalue\ S): \passbyvalue\ S =>
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

### Inline assembler supported

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

### Atomics

* Added an atomic library in the built in package. The atomic type was modelled after std::atomic in C++.

  ```pony
  var a: Atomic[U32] = Atomic[U32](10)

  var r = a + 1 // same as a.fetch_add(1)
  ```

### Added an extra capability, nhb.

* Added the extra cability called **nhb**. The nhb capability makes it possible to mutably share a class among actors. This is inteded when it is possible to mutably share a class when the class has implemented some form of custom synchronization like atomics or mutexes. 

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

### Added CTFE (Compile Time Function Execution)

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

* It is possible to go really far with CTFE and the goal is to support as many expressions and types as possible. CTFE will be added to more places than only `comptime expression end`. It is also possible for the CTFE to try evaulating at key places in the code. In the D language it is possible load files at compile time using `import("file.txt")` which can be used together with CTFE and similar functionality should be added to SpacePony as well.

* Note that the purpose of CTFE is not really optimizations but rather a guarantee that an expression can be evaluated at compile time. LLVM already does good constant folding and can do the much of same job as CTFE. One important decision to add CTFE was to be able to have expressions in value type parameters in generics.

## Future directions

### Short term

* Right now there is a class called FixedSizedArray which is copy of CFixedSizedArray but implemented as a class with a type descriptor. However, since it is already "spoiled" with a an extra class member, why not add a size field as well. In this case FixedSizedArray can be converted to a FixedCapacityArray instead. It is unclear what is the most useful, if FixedCapacityArray or FixedSizedArray are necessary at all in addition to CFixedSizedArray.

* Since there are suddenly several different arrays, a Slice class might be needed. This would be eqvivalent to std::span in C++. However, it is also possible to reuse the Array class for this purpose as it is possible to load the Array with outside raw pointers. This is possible because the Array class use garbage collected pointers which can co-exist with foreign raw pointers. The D language has chosen this approach where there is a merge between the slice and the dynamic array. Slices or reuse Array as both their pros and cons.

* Real asynchronous IO and not a POSIX like wrapper. An API that can be used for anything streaming like Files, HTTP, TCP. The API should also use the best available asynchrounous OS API primitives.

* Implement a good and comprehensive reflection interface.

### Long term (read never)

* Move to MLIR backend

* GPU offload. The capability/actor model of Pony is very well suited for running parts of the actors in GPUs.

* Improve the Pony type system.


## The original Pony README is here with instruction how to compile.

* [Pony README](README_old_Pony.md)
