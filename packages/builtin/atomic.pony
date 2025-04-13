primitive AtomicOrdering
  fun not_atomic(): U32 => 0
  fun unordered(): U32 => 1
  fun monotonic(): U32 => 2
  fun acquire(): U32 => 4
  fun release(): U32 => 5
  fun acquire_release(): U32 => 6
  fun sequentially_consistent(): U32 => 7

struct Atomic[A: Real[A] val]
  """
  Atomic type
  """
  var _val: A

  new create(v: A) =>
    """
    create a new atomic value with initial value of val
    """
    _val = v

  fun load(): A =>
    compile_intrinsic

  fun ref store(v: A) =>
    compile_intrinsic

  fun ref fetch_add(v: A): A =>
    compile_intrinsic

  fun ref fetch_sub(v: A): A =>
    compile_intrinsic

  fun ref fetch_and(v: A): A =>
    compile_intrinsic

  fun ref fetch_nand(v: A): A =>
    compile_intrinsic

  fun ref fetch_or(v: A): A =>
    compile_intrinsic

  fun ref fetch_xor(v: A): A =>
    compile_intrinsic

  fun ref add_fetch(v: A): A =>
    fetch_add(v) + v

  fun ref sub_fetch(v: A): A =>
    fetch_sub(v) - v

  fun ref add(v: A): A =>
    add_fetch(v)

  fun ref sub(v: A): A =>
    sub_fetch(v)