primitive MemoryOrderNotAtomic
primitive MemoryOrderUnordered
primitive MemoryOrderMonotonic
primitive MemoryOrderAcquire
primitive MemoryOrderRelease
primitive MemoryOrderAcquireRelease
primitive MemoryOrderSequentiallyConsistent

type AtomicMemoryOrder is
  ( MemoryOrderNotAtomic | MemoryOrderUnordered | MemoryOrderMonotonic | MemoryOrderAcquire |
    MemoryOrderRelease | MemoryOrderAcquireRelease | MemoryOrderSequentiallyConsistent)

struct Atomic[A: Integer[A] val]
  """
  This is a Pony version of a partial implementation similar to the std::atomic template
  found in C++ STL. https://en.cppreference.com/w/cpp/atomic/atomic
  """
  var _val: A

  new create(v: A) =>
    """
    Initializes the underlying object with desired value. The initialization is not atomic.
    """
    _val = v

  fun load[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](): A =>
    """
    Atomically loads the current value.
    """
    compile_intrinsic

  fun ref store[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A) =>
    """
    Atomically replaces the current value with a desired value.
    Memory is affected according to the value of order.
    """
    compile_intrinsic

  fun ref exchange[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically replaces the underlying value with a desired value (a read-modify-write operation).
    Memory is affected according to the value of order. Returns the atomic value BEFORE the exchange.
    """
    compile_intrinsic

  fun ref fetch_add[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically replaces the current value with the result of arithmetic addition of the atomic variable and v.
    That is, it performs atomic post-increment. The operation is a read-modify-write operation.
    Memory is affected according to the value of order. Returns the atomic value BEFORE the operation.
    """
    compile_intrinsic

  fun ref fetch_sub[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically replaces the current value with the result of arithmetic subtraction of the atomic variable and v.
    That is, it performs atomic post-increment. The operation is a read-modify-write operation.
    Memory is affected according to the value of order. Returns the atomic value BEFORE the operation.
    """
    compile_intrinsic

  fun ref fetch_and[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically replaces the current value with the result of bitwise AND of the the atomic value and v.
    The operation is read-modify-write operation. Memory is affected according to the value of order.
    Returns the atomic value BEFORE the operation.
    """
    compile_intrinsic

  fun ref fetch_nand[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically replaces the current value with the result of bitwise NAND of the the atomic value and v.
    The operation is read-modify-write operation. Memory is affected according to the value of order.
    Returns the atomic value BEFORE the operation.
    """
    compile_intrinsic

  fun ref fetch_or[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically replaces the current value with the result of bitwise OR of the the atomic value and v.
    The operation is read-modify-write operation. Memory is affected according to the value of order.
    Returns the atomic value BEFORE the operation.
    """
    compile_intrinsic

  fun ref fetch_xor[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically replaces the current value with the result of bitwise XOR of the the atomic value and v.
    The operation is read-modify-write operation. Memory is affected according to the value of order.
    Returns the atomic value BEFORE the operation.
    """
    compile_intrinsic

  fun ref fetch_min[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    atomically performs min operation between the argument and the value of the atomic object
    and obtains the value held previously
    """
    compile_intrinsic

  fun ref fetch_max[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    atomically performs max operation between the argument and the value of the atomic object
    and obtains the value held previously
    """
    compile_intrinsic

  fun ref add_fetch[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically adds v to the atomic value. The operation is read-modify-write operation.
    Returns the atomic value AFTER the operation.
    """
    fetch_add[M](v) + v

  fun ref sub_fetch[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically subtracts v to the atomic value. The operation is read-modify-write operation.
    Returns the atomic value AFTER the operation. Equivalent to fetch_add(v).
    """
    fetch_sub[M](v) - v

  fun ref and_fetch[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically replaces the current value with the result of a bitwise AND operation involving the
    previous value and v. The operation is read-modify-write operation. Returns the atomic value
    AFTER the operation
    """
    fetch_and[M](v) and v

  fun ref nand_fetch[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically replaces the current value with the result of a bitwise NAND operation involving the
    previous value and v. The operation is read-modify-write operation. Returns the atomic value
    AFTER the operation
    """
    not (fetch_nand[M](v) and v)

  fun ref or_fetch[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically replaces the current value with the result of a bitwise OR operation involving the
    previous value and v. The operation is read-modify-write operation. Returns the atomic value
    AFTER the operation
    """
    fetch_or[M](v) or v

  fun ref xor_fetch[M: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent](v: A): A =>
    """
    Atomically replaces the current value with the result of an bitwise XOR operation involving the
    previous value and v. The operation is read-modify-write operation. Returns the atomic value
    AFTER the operation
    """
    fetch_xor[M](v) xor v

  fun ref add(v: A): A =>
    """
    Atomically adds v to the atomic value. The operation is read-modify-write operation.
    Returns the atomic value AFTER the operation. Equivalent to add_fetch(v).
    """
    add_fetch(v)

  fun ref sub(v: A): A =>
    """
    Atomically subtracts v to the atomic value. The operation is read-modify-write operation.
    Returns the atomic value AFTER the operation. Equivalent to sub_fetch(v).
    """
    sub_fetch(v)

  fun ref op_and(v: A): A =>
    """
    Atomically performs a bitwise AND with the atomic value and v.
    The operation is read-modify-write operation.
    Returns the atomic value AFTER the operation. Equivalent to and_fetch(v).
    """
    and_fetch(v)

  fun ref op_or(v: A): A =>
    """
    Atomically performs a bitwise OR with the atomic value and v.
    The operation is read-modify-write operation.
    Returns the atomic value AFTER the operation. Equivalent to or_fetch(v).
    """
    or_fetch(v)

  fun ref op_xor(v: A): A =>
    """
    Atomically performs a bitwise XOR with the atomic value and v.
    The operation is read-modify-write operation.
    Returns the atomic value AFTER the operation. Equivalent to xor_fetch(v).
    """
    xor_fetch(v)

  fun ref compare_exchange_weak[
    OrderSuccess: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent,
    OrderFail: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent]
    (expected: Pointer[A], desired: A): Bool =>
    """
    Atomically compares the atomic value with that of expected. If those are bitwise-equal, replaces the former with desired
    (performs read-modify-write operation). Otherwise, loads the actual atomic value stored into expected (performs load operation).

    bool_compare_exchange_weak is allowed to fail spuriously, that is, acts as if atomic value != expected even if they are equal.
    When a compare-and-exchange is in a loop, compare_exchange_weak will yield better performance on some platforms.

    Params:
    expected = The expected value
    desired = The new desired value

    Returns:
    true if the underlying atomic value was successfully changed, false otherwise.
    """
    compile_intrinsic

  fun ref compare_exchange_strong[
    OrderSuccess: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent,
    OrderFail: AtomicMemoryOrder = MemoryOrderSequentiallyConsistent]
    (expected: Pointer[A], desired: A): Bool =>
    """
    Atomically compares the atomic value with that of expected. If those are bitwise-equal, replaces the former with desired
    (performs read-modify-write operation). Otherwise, loads the actual atomic value stored into expected (performs load operation).

    Params:
    expected = The expected value
    desired = The new desired value

    Returns:
    true if the underlying atomic value was successfully changed, false otherwise.
    """
    compile_intrinsic