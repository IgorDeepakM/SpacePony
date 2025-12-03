primitive TypeTrait
  """
  Type traits is a set of methods that enables checks on types for
  different properties. For example checking if a type is a class.
  """
  fun is_struct[A: AnyNoCheck](): Bool =>
    """
    Checks if a type is a struct
    """
    compile_intrinsic

  fun is_class[A: AnyNoCheck](): Bool =>
    """
    Checks if a type is a class
    """
    compile_intrinsic

  fun is_primitive[A: AnyNoCheck](): Bool =>
    """
    Checks if a type is a primitive
    """
    compile_intrinsic

  fun is_subtype[A: AnyNoCheck, B: AnyNoCheck](): Bool =>
    """
    Checks if type A is a subtype of type B. The capability of A must be a
    subcapability of the capability of B.
    """
    compile_intrinsic

  fun is_subtype_constraint[A: AnyNoCheck, B: AnyNoCheck](): Bool =>
    """
    Checks if type A is a subtype of type B. The capability of A must be a
    equal of the capability of B.
    """
    compile_intrinsic

  fun is_subtype_ignore_cap[A: AnyNoCheck, B: AnyNoCheck](): Bool =>
    """
    Checks if type A is a subtype of type B. The capability subtyping is ignored.
    """
    compile_intrinsic

  fun is_eqtype[A: AnyNoCheck, B: AnyNoCheck](): Bool =>
    """
    Checks if type A is a equal to type B.
    """
    compile_intrinsic

  fun is_bool[A: AnyNoCheck](): Bool =>
    """
    Checks if type A is of type Bool.
    """
    compile_intrinsic

  fun is_float[A: AnyNoCheck](): Bool =>
    """
    Checks if type A is a floating point type.
    """
    compile_intrinsic

  fun is_integer[A: AnyNoCheck](): Bool =>
    """
    Checks if type A is an integer type.
    """
    compile_intrinsic

  fun is_machine_word[A: AnyNoCheck](): Bool =>
    """
    Checks if type A is a machine word.
    """
    compile_intrinsic

  fun is_signed[A: AnyNoCheck](): Bool =>
    """
    Checks if type A is a signed integer.
    """
    compile_intrinsic