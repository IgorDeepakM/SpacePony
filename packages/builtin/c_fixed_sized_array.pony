struct CFixedSizedArray[A: AnyNoCheck, _size: USize]
  """
  Contiguous, fixed sized memory to store elements of type A.
  """

  new create(fill_with: A^) =>
    """
    Create a CFixedSizedArray, filled with a gived value
    """
    let p = cpointer().convert[A]()
    var i: USize = 0
    while i < _size do
        p._update(i = i + 1, fill_with)
    end

  new init(from: Iterator[A^]) ? =>
    """
    Create a CFixedSizedArray, initialised from the given sequence.
    """
    let p = cpointer().convert[A]()
    var i: USize = 0
    while i < _size do
      p._update(i = i + 1, from.next()?)
    end

  new uninitialized() =>
    """
    Creates an CFixedSizedArray with unitialized memory
    """
    None

  fun cpointer(): Pointer[A] =>
    """
    Return the underlying C-style pointer.
    """
    compile_intrinsic

   fun apply(i: USize): this->A ? =>
    """
    Get the i-th element, raising an error if the index is out of bounds.
    """
    if i < _size then
      let p = cpointer().convert[this->A]()
      p._apply(i)
    else
      error
    end

  fun ref update(i: USize, value: A): A^ ? =>
    """
    Change the i-th element, raising an error if the index is out of bounds.
    """
    if i < _size then
      let p = cpointer().convert[A]()
      p._update(i, consume value)
    else
      error
    end

  fun size(): USize =>
    """
    The number of elements in the array.
    """
    _size


  /*new duplicate(from: A^) =>
    """
    Create a vector, initiliased to the given value
    """
    var i: USize = 0
    while i < _size do
      _update(i = i + 1, from)
    end

  new generate(f: {ref(USize): A^ ?} ref) ? =>
    """
    Create a vector initiliased using a generator function
    """
    var i: USize = 0
    while i < _size do
      _update(i, f(i = i + 1))
    end

  new undefined[B: (A & Real[B] val & Number) = A]() =>
    """
    Create a vector of len elements, populating them with random memory. This
    is only allowed for a vector of numbers.
    """
    true

  fun filter(f: {val(box->A): Bool} val): Array[this->A!] =>
    let elems = Array[this->A]
    var i: USize = 0
    while i < _size do
      let elem = _apply(i = i + 1)
      if f(elem) then
        elems.push(elem)
      end
    end
    elems

  fun reduce[B = box->A](f: {(B, box->A): B^} val, start: B): B^ =>
    var i: USize = 0
    var acc = consume start
    while i < _size do
      acc = f(consume acc, _apply(i = i + 1))
    end
    acc

  fun map[B = box->A](f: {(box->A): B^} val): Vector[B, # _size] ref ? =>
    Vector[B, #_size].generate(
      lambda (i: USize)(f, me: Vector[A, #_size] box = this): B^ ? =>
        f(me.apply(i))
      end)

  fun copy_to(dst: Seq[this->A!]) ? =>
    var i: USize = 0
    while i < _size do
      dst.update(i, _apply(i = i + 1))
    end
    //dst.generate(lambda ref(i: USize)(src=this): A ? => src(i) end)

  fun string(f: {val(box->A!): String} val): String ref^ =>
    let array = Array[String]
    for value in values() do
      array.push(f(value))
    end
    String.append("{").append(", ".join(array)).append("}")

  fun size(): USize =>
    """
    Return the parameterised size, this lets us treat the Vector as a ReadSeq
    """
    _size

  fun apply(i: USize): this->A ? =>
    """
    Get the i-th element, raising an error if the index is out of bounds.
    """
    if i < _size then
      _apply(i)
    else
      error
    end

  fun ref update(i: USize, value: A): A^ ? =>
    """
    Change the i-th element, raising an error if the index is out of bounds.
    """
    if i < _size then
      _update(i, consume value)
    else
      error
    end*/

fun keys(): CFixedSizedArrayKeys[A, _size, this->CFixedSizedArray[A, _size]]^ =>
  """
  Return an iterator over the indices in the array.
  """
  CFixedSizedArrayKeys[A, _size, this->CFixedSizedArray[A, _size]](this)

fun values(): CFixedSizedArrayValues[A, _size, this->CFixedSizedArray[A, _size]]^ =>
  """
  Return an iterator over the values in the array.
  """
  CFixedSizedArrayValues[A, _size, this->CFixedSizedArray[A, _size]](this)

fun pairs(): CFixedSizedArrayPairs[A, _size, this->CFixedSizedArray[A, _size]]^ =>
  """
  Return an iterator over the (index, value) pairs in the array.
  """
  CFixedSizedArrayPairs[A, _size, this->CFixedSizedArray[A, _size]](this)


class CFixedSizedArrayKeys[A, _size: USize, B: CFixedSizedArray[A, _size] #read] is Iterator[USize]
  let _array: B
  var _i: USize

  new create(array: B) =>
    _array = array
    _i = 0

  fun has_next(): Bool =>
    _i < _array.size()

  fun ref next(): USize =>
    if _i < _array.size() then
      _i = _i + 1
    else
      _i
    end


class CFixedSizedArrayValues[A, _size: USize, B: CFixedSizedArray[A, _size] #read] is Iterator[B->A]
  let _array: B
  var _i: USize

  new create(array: B) =>
    _array = array
    _i = 0

  fun has_next(): Bool =>
    _i < _array.size()

  fun ref next(): B->A ? =>
    _array(_i = _i + 1)?


class CFixedSizedArrayPairs[A, _size: USize, B: CFixedSizedArray[A, _size] #read] is Iterator[(USize, B->A)]
  let _array: B
  var _i: USize

  new create(array: B) =>
    _array = array
    _i = 0

  fun has_next(): Bool =>
    _i < _array.size()

  fun ref next(): (USize, B->A) ? =>
    (_i, _array(_i = _i + 1)?)
