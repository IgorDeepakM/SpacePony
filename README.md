# SpacePony

SpacePony is an experimental fork of the [Pony programming language](https://github.com/ponylang/ponyc). The goal of the fork is to improve the FFI capabilities and add more systems programming language features.

## List of changes.

### Pointer

* The Pointer type can be used **everywhere** not only as FFI function arguments.
* It can be initialized using from_usize(addr: USize) in order to initialize a Pointer with any desired number.
* The convert method is made public in order to make it possible to cast one pointer type to another.
* Added to to_reftype method to convert the Pointer to the underlying reference type. This is equivalent to the apply method of the NullablePointer type.
* Added to to_reftype_no_check method to convert the Pointer to the underlying reference type without any null pointer check.

### addressof

* addressof can be used **everywhere**
* It is possible to use addressof of an FFI function.

### Constant values in generics

* It is possible to use constants (literals) as type arguments in generics, both as type arguments for types and type arguments for functions.
* Default value type arguments are supported.

### CFixedSizedArray

* Added CFixedSizedArray in order to be able to model C fixed sized arrays found in C.

### offsetof

* Added offsetof in order to get an offset of a member in a struct or class.

### sizeof

* Added sizeof operator to obtain the size of any type. Also works on nested types.

### Added FFI pass by value parameters.

* Structs and classes can be passed as value to FFI functions.

### Inline assembler supported

* Added support for an LLVM like syntax for inline assembler.

### Atomics

* Added an atomic library in the built in package.

### Added an extra capability, nhb.

* Added the extra cability called **nhb**. The nhb capability makes it possible to mutably share a class among actors. This is inteded when it is possible to mutably share a class when the class has implemented some form of custom synchronization like atomics or mutexes. 


## The original Pony README is here with instruction how to compile.

* [Pony README](README_old_Pony.md)
