#include <gtest/gtest.h>
#include <platform.h>
#include <type/subtype.h>
#include "util.h"

#define TEST_COMPILE(src) DO(test_compile(src, "expr"))
#define TEST_ERROR(src) DO(test_error(src, "expr"))

#define TEST_ERRORS_1(src, err1) \
  { const char* errs[] = {err1, NULL}; \
    DO(test_expected_errors(src, "expr", errs)); }

class PropertyTest: public PassTest
{};


TEST_F(PropertyTest, PropertyWrongLocation)
{
  const char* src =
    "primitive \\property\\ P\n"
    "  fun m1(): I32 => 22\n";

  TEST_ERRORS_1(src, "a 'property' annotation can only be used on function definitions");
}

TEST_F(PropertyTest, ReadPropertyOk)
{
  const char* src =
    "class C\n"
    "  var x: I32 = 0\n"
    "  fun \\property\\ m1(): I32 => x";

  TEST_COMPILE(src);
}

TEST_F(PropertyTest, ErrorReadPropertyNoReturn)
{
  const char* src =
    "class C\n"
    "  fun \\property\\ m1() => None";
    
  TEST_ERRORS_1(src, "a read property method must have a return type");
}

TEST_F(PropertyTest, ErrorReadPropertyNoParameterAllowed)
{
  const char* src =
    "class C\n"
    "  var x: I32 = 0\n"
    "  fun \\property\\ m1(x': I32): I32 => x";

  TEST_ERRORS_1(src, "a write property method must have a name with at least one character "
                     "and then is appended with '_w'");
}

TEST_F(PropertyTest, WritePropertyOk)
{
  const char* src =
    "class C\n"
    "  var x: I32 = 0\n"
    "  fun \\property\\ ref m1_w(x': I32) => x = x'";

  TEST_COMPILE(src);
}

TEST_F(PropertyTest, ErrorWritePropertyNo_w_AtEnd)
{
  const char* src =
    "class C\n"
    "  var x: I32 = 0\n"
    "  fun \\property\\ ref m1(x': I32) => x = x'";

  TEST_ERRORS_1(src, "a write property method must have a name with at least one character "
                     "and then is appended with '_w'");
}

TEST_F(PropertyTest, ErrorWritePropertyNoParam)
{
  const char* src =
    "class C\n"
    "  fun \\property\\ ref m1_w(): I32 => None";

  TEST_ERRORS_1(src, "a read property method cannot have a name that ends with '_w'");
}

TEST_F(PropertyTest, ErrorWritePropertyShadowsVariable)
{
  const char* src =
    "class C\n"
    "  var x: I32 = 0\n"
    "  var m1: I32 = 0\n"
    "  fun \\property\\ ref m1_w(x': I32) => x = x'";

  TEST_ERRORS_1(src, "The name 'm1' is already used which shadows the write property 'm1_w'");
}

TEST_F(PropertyTest, BothWriteAndReadPropertyOk)
{
  const char* src =
    "class C\n"
    "  var x: I32 = 0\n"
    "  fun \\property\\ m1(): I32 => x\n"
    "  fun \\property\\ ref m1_w(x': I32) => x = x'";

  TEST_COMPILE(src);
}

TEST_F(PropertyTest, WritePropertyWithReturnValueOk)
{
  const char* src =
    "class C\n"
    "  var x: I32 = 0\n"
    "  fun \\property\\ ref m1_w(x': I32): I32 =>\n"
    "    x = x'\n";
    "    x";

  TEST_COMPILE(src);
}

TEST_F(PropertyTest, ErrorPropertyTooManyParameters)
{
  const char* src =
    "class C\n"
    "  var x: I32 = 0\n"
    "  fun \\property\\ ref m1_w(x': I32, y': I32) => x = x'";

  TEST_ERRORS_1(src, "a property method cannot have more than one parameters");
}
