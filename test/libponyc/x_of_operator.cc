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
    "    offsetof s.a\n"
    "    offsetof s.b\n";

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
    "    offsetof S.a\n"
    "    offsetof S.b\n";

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
    "    offsetof s2.s1.a\n"
    "    offsetof s2.s1.b\n";

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
    "    offsetof S2.s1.a\n"
    "    offsetof S2.s1.b\n";

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
    "    offsetof s\n";

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
    "    offsetof S\n";

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
    "    sizeof S\n"
    "    sizeof s\n"
    "    sizeof S.a\n"
    "    sizeof s.b\n"
    "    sizeof v\n"
    "    sizeof v2\n"
    "    sizeof USize\n"
    "    sizeof F64\n";

  TEST_COMPILE(src);
}