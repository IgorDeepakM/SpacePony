struct IntrusiveDListNode
  var _p: IntrusiveDListNode
  var _n: IntrusiveDListNode

  new create() =>
    let p = Pointer[IntrusiveDListNode].from_any[IntrusiveDListNode tag](this)
    var fake_ptr: IntrusiveDListNode = p.to_reftype_no_check()
    _p = fake_ptr
    _n = fake_ptr

  fun _next(): this->IntrusiveDListNode =>
    _n

  fun _prev(): this->IntrusiveDListNode =>
    _p

  fun ref _hook(pos: IntrusiveDListNode) =>
    _n = pos
    _p = pos._p
    pos._p._n = this
    pos._p = this

  fun ref _unhook() =>
    let next_node = _n
    let prev_node = _p
    prev_node._n = next_node
    prev_node._p = prev_node

trait val NodeOffset
  new val create()
  fun get_offset(): USize


class IntrusiveDList[A, OffsetPrimitive: NodeOffset #read]
  embed _head: IntrusiveDListNode

  new create() =>
    _head = IntrusiveDListNode

  new from(seq: Array[A^]) =>
    """
    Creates a list equivalent to the provided Array (both node number and order are preserved).

    ```pony
    let my_list = IntrusiveDList[T, TNode].from([T(1); T(2); T(3)])
    ```
    """
    _head = IntrusiveDListNode

    for value in seq.values() do
      push(consume value)
    end

  fun empty(): Bool =>
    _head._next() is _head

  fun _node_is_head(node: IntrusiveDListNode box): Bool =>
    node is _head

  fun _get_head_node(): this->IntrusiveDListNode =>
    _head

  fun _get_parent_from_member(node: this->IntrusiveDListNode): this->A =>
    let member_ptr = Pointer[IntrusiveDListNode].from_any[this->IntrusiveDListNode](node)
    let u8_ptr = member_ptr.convert[U8]()
    var signed_offset = OffsetPrimitive.get_offset().isize_unsafe()
    signed_offset = -signed_offset
    let fake_offset = signed_offset.usize_unsafe()
    var parent_ptr = u8_ptr.pointer_at_index(fake_offset)
    var parent_c = parent_ptr.convert[this->A]()
    parent_c.to_reftype_no_check()

  fun _get_member_from_parent(parent: A!): IntrusiveDListNode =>
    let parent_ptr = Pointer[A].from_any[A!](parent)
    let u8_ptr = parent_ptr.convert[U8]()
    let member_ptr = u8_ptr.pointer_at_index(OffsetPrimitive.get_offset())
    let member_c = member_ptr.convert[IntrusiveDListNode]()
    member_c.to_reftype_no_check()

  fun ref prepend_node(elem: A) =>
    var elem_node = _get_member_from_parent(elem)
    elem_node._hook(_head._next())

  fun ref append_node(elem: A) =>
    var elem_node = _get_member_from_parent(elem)
    elem_node._hook(_head)

  fun head(): this->A ? =>
    if not empty() then
      _get_parent_from_member(_head._next())
    else
      error
    end

  fun tail(): this->A ? =>
    if not empty() then
      _get_parent_from_member(_head._prev())
    else
      error
    end

  fun ref pop(): A! ? =>
    if not empty() then
      let elem = _get_parent_from_member(_head._prev())
      _head._prev()._unhook()
      elem
    else
      error
    end

  fun ref push(elem: A^) =>
    append_node(elem)

  fun ref remove_node(elem: A) =>
    let elem_node = _get_member_from_parent(elem)
    elem_node._unhook()

  fun apply(i: USize = 0): this->A ? =>
    if empty() then
      error
    end

    var curr_node = _head._next()
    var j = USize(0)

    while j < i do
      curr_node = curr_node._next()
      if _node_is_head(curr_node) then
        error
      end
      j = j + 1
    end

    _get_parent_from_member(curr_node)

  fun ref insert(pos: A, elem: A) =>
    let pos_node = _get_member_from_parent(pos)
    let elem_node = _get_member_from_parent(elem)
    elem_node._hook(pos_node)

  fun ref nodes(): IntrusiveDListIterator[A, OffsetPrimitive] =>
    IntrusiveDListIterator[A, OffsetPrimitive](this)

  fun ref rnodes(): IntrusiveDListReverseIterator[A, OffsetPrimitive] =>
    IntrusiveDListReverseIterator[A, OffsetPrimitive](this)


class IntrusiveDListIterator[A, OffsetPrimitive: NodeOffset #read] is Iterator[A]
  let _list: IntrusiveDList[A, OffsetPrimitive]
  var _pos: IntrusiveDListNode

  new create(list: IntrusiveDList[A, OffsetPrimitive]) =>
    _list = list
    _pos = list._get_head_node()._next()

  fun has_next(): Bool =>
    not _list._node_is_head(_pos)

  fun ref next(): A ? =>
    if _list._node_is_head(_pos) then
      error
    else
      let old_pos = _pos
      _pos = _pos._next()
      _list._get_parent_from_member(old_pos)
    end

class IntrusiveDListReverseIterator[A, OffsetPrimitive: NodeOffset #read] is Iterator[A]
  let _list: IntrusiveDList[A, OffsetPrimitive]
  var _pos: IntrusiveDListNode

  new create(list: IntrusiveDList[A, OffsetPrimitive]) =>
    _list = list
    _pos = list._get_head_node()._prev()

  fun has_next(): Bool =>
    not _list._node_is_head(_pos)

  fun ref next(): A ? =>
    if _list._node_is_head(_pos) then
      error
    else
      let old_pos = _pos
      _pos = _pos._prev()
      _list._get_parent_from_member(old_pos)
    end
