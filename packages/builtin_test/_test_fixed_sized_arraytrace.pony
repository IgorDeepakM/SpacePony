use "pony_test"

class iso _TestFixedSizedArrayTrace is UnitTest
  """
  Test val trace optimisation
  """
  fun name(): String => "builtin/FixedSizedArrayTrace"

  fun apply(h: TestHelper) =>
    _FixedSizedArrayTrace.one(h)
    h.long_test(2_000_000_000) // 2 second timeout


actor _FixedSizedArrayTrace

  be three(h: TestHelper, v: FixedSizedArray[String, 3] iso) =>
    @pony_triggergc(@pony_ctx())
    try
      h.assert_eq[String]("wombat", v(0)?)
      h.assert_eq[String]("aardvark", v(1)?)
      h.assert_eq[String]("meerkat", v(2)?)
    else
      h.fail()
    end
    h.complete(true)

  be two(h: TestHelper, s1: String, s2: String, s3: String) =>
    @pony_triggergc(@pony_ctx())
    try
      let v = recover FixedSizedArray[String, 3].from_iterator([s1; s2; s3].values())? end
      _FixedSizedArrayTrace.three(h, consume v)
    else
      h.fail()
    end

  be one(h: TestHelper) =>
    @pony_triggergc(@pony_ctx())
    let s1 = "wombat"
    let s2 = "aardvark"
    let s3 = "meerkat"
    _FixedSizedArrayTrace.two(h, consume s1, consume s2, consume s3)
