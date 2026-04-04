struct box Optional[A: AnyNoCheck]
  """
  An Optional is a low level primitive that allows a value or not a value.
  Internally, if the value is a pointer to a primitive then it is stored as
  one word, where null indiciated unvalid value. If it is a machine word
  type other than pointer then it is stored as the type + bool telling if
  the value is valid. This is order to avoid boxing as it is possible to use
  (T | None) as an optional but it requires boxing for certain types. Also
  (T | None) doesn't work for structs.

  If A is a pointer Optional is a pointer
  If A is another type Optional is a { T, Bool }

  Optional is of value type and cannot be sent to another actor.

  This type is used for return values of partial functions.
  """
  new none() =>
    """
    Initializes the optional to none.
    """
    compile_intrinsic

  new create(init: A) =>
    """
    Initializes the optional to a value
    """
    compile_intrinsic

  fun is_none(): Bool =>
    """
    Returns true if the optional is empty
    """
    compile_intrinsic

  fun is_some(): Bool =>
    """
    Returns true if the optional has a valid value
    """
    not is_none()

  fun apply(): A ? =>
    """
    Gets the value from the optional. If the optional has no value then an
    error is thrown.
    """
    if is_none() then
      error
    else
      get_no_check()
    end

  fun get_no_check(): A =>
    """
    Gets the value from the optional. If the optional has no value then the
    return value is undefined
    """
    compile_intrinsic
