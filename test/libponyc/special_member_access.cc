#include <gtest/gtest.h>
#include <platform.h>
#include <type/subtype.h>
#include "util.h"

#define TEST_COMPILE(src) DO(test_compile(src, "expr"))
#define TEST_ERROR(src) DO(test_error(src, "expr"))

#define TEST_ERRORS_1(src, err1) \
  { const char* errs[] = {err1, NULL}; \
    DO(test_expected_errors(src, "expr", errs)); }

class XOfOperatorTest: public PassTest
{};

TEST_F(XOfOperatorTest, OffsetOfUsingVariable)
{
  const char* src =
    "struct S\n"
    "  var a: U32 = 0\n"
    "  var b: U32 = 1\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    var s: S = S\n"
    "    s.a.offset_of\n"
    "    s.b.offset_of\n";

  TEST_COMPILE(src);
}

TEST_F(XOfOperatorTest, OffsetOfUsingType)
{
  const char* src =
    "struct S\n"
    "var a: U32 = 0\n"
    "var b: U32 = 1\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    S.a.offset_of\n"
    "    S.b.offset_of\n";

    TEST_COMPILE(src);
}

TEST_F(XOfOperatorTest, OffsetOfNestedUsingVariable)
{
  const char* src =
    "struct S1\n"
    "  var a: U32 = 0\n"
    "  var b: U32 = 1\n"
    "struct S2\n"
    "  var a: U32 = 0\n"
    "  embed s1: S1 = S1\n"
    "  var c: U32 = 1\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    var s2: S2 = S2\n"
    "    s2.s1.a.offset_of\n"
    "    s2.s1.b.offset_of\n";

    TEST_COMPILE(src);
}

TEST_F(XOfOperatorTest, OffsetOfNestedUsingType)
{
  const char* src =
    "struct S1\n"
    "  var a: U32 = 0\n"
    "  var b: U32 = 1\n"
    "struct S2\n"
    "  var a: U32 = 0\n"
    "  embed s1: S1 = S1\n"
    "  var c: U32 = 1\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    S2.s1.a.offset_of\n"
    "    S2.s1.b.offset_of\n";

    TEST_COMPILE(src);
}

TEST_F(XOfOperatorTest, OffsetOfFailNotField)
{
  const char* src =
    "struct S\n"
    "  var a: U32 = 0\n"
    "  var b: U32 = 1\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    var s: S = S\n"
    "    s.offset_of\n";

    TEST_ERRORS_1(src, "can't take the offset of a var local");
}

TEST_F(XOfOperatorTest, OffsetOfFailOnlyType)
{
  const char* src =
    "struct S\n"
    "  var a: U32 = 0\n"
    "  var b: U32 = 1\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    S.offset_of\n";

    TEST_ERRORS_1(src, "can only take the offset of a field");
}

TEST_F(XOfOperatorTest, SizeOfSuccess)
{
  const char* src =
    "struct S\n"
    "  var a: U32 = 0\n"
    "  let b: U32 = 1\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    var v: I32 = 0\n"
    "    let v2: I32 = 0\n"
    "    var s: S = S\n"
    "    S.size_of\n"
    "    s.size_of\n"
    "    S.a.size_of\n"
    "    s.b.size_of\n"
    "    v.size_of\n"
    "    v2.size_of\n"
    "    USize.size_of\n"
    "    F64.size_of\n";

  TEST_COMPILE(src);
}