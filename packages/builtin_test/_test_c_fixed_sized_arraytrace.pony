use "pony_test"

class iso _TestCFixedSizedArrayTrace is UnitTest
  """
  Test val trace optimisation
  """
  fun name(): String => "builtin/CFixedSizedArrayTrace"

  fun apply(h: TestHelper) =>
    _CFixedSizedArrayTrace.one(h)
    h.long_test(2_000_000_000) // 2 second timeout


actor _CFixedSizedArrayTrace

  be three(h: TestHelper, v: CFixedSizedArray[String, 3] iso) =>
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
      let v = recover CFixedSizedArray[String, 3].from_array([s1; s2; s3])? end
      _CFixedSizedArrayTrace.three(h, consume v)
    else
      h.fail()
    end

  be one(h: TestHelper) =>
    @pony_triggergc(@pony_ctx())
    let s1 = "wombat"
    let s2 = "aardvark"
    let s3 = "meerkat"
    _CFixedSizedArrayTrace.two(h, consume s1, consume s2, consume s3)
