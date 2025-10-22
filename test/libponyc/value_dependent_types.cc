#include <gtest/gtest.h>
#include <platform.h>
#include <type/subtype.h>
#include "util.h"

#define TEST_COMPILE(src) DO(test_compile(src, "expr"))
#define TEST_ERROR(src) DO(test_error(src, "expr"))

class VDTTest: public PassTest
{};

TEST_F(VDTTest, TraitWithParameterValue)
{
  const char* src =
    "trait T1[n: U32]";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, ParameterValueUsedInMethod)
{
  const char* src =
    "trait T1[n: U32]\n"
    "  fun get_n(): U32 =>\n"
    "    n";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, ParameterValueUsedInMethodWrongType)
{
  const char* src =
    "trait T1[n: U32]\n"
    "  fun get_n(): U64 =>\n"
    "    n";

  TEST_ERROR(src);
}

TEST_F(VDTTest, TestBoolOutsideU32Constraint)
{
  const char* src =
    "trait T1[n: U32]\n"
    "class C1 is T1[true]";

  TEST_ERROR(src);
}

TEST_F(VDTTest, ClassWithValueDependentTrait)
{
  const char* src =
    "trait T1[n: U32]\n"
    "class C1 is T1[4]";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, ClassWithParameterValue)
{
  const char* src =
    "class C1[n: U32]";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, ClassWithMatchingArgument)
{
  const char* src =
    "class C1[n: U32]\n"
    "class C2\n"
    "  let c: C1[2] = C1[2]";

  TEST_COMPILE(src);
}


TEST_F(VDTTest, ClassWithMismatchValueArgument)
{
  const char* src =
    "class C1[n: U32]\n"
    "class C2\n"
    "  let c: C1[47] = C1[2]";

  TEST_ERROR(src);
}

TEST_F(VDTTest, ClassWithDeclareTypeMismatchValueArgument)
{
  const char* src =
    "class C1[n: U32]\n"
    "class C2\n"
    "  let c: C1[true] = C1[2]";

  TEST_ERROR(src);
}

TEST_F(VDTTest, ClassWithArgumentTypeMismatchValueArgument)
{
  const char* src =
    "class C1[n: U32]\n"
    "class C2\n"
    "  let c: C1[2] = C1[true]";

  TEST_ERROR(src);
}

TEST_F(VDTTest, FunctionCallWithFieldArgumentTypeMismatchValueArgument)
{
  const char* src =
    "class C1[n: U32]\n"
    "class C2\n"
    "  let c: C1[let n = 2] = C1[let n = 2]"
    "  fun get(f: C1[let n = 1]) =>\n"
    "    true\n"
    "  new create() =>\n"
    "    get(c1)";

  TEST_ERROR(src);
}

TEST_F(VDTTest, FunctionCallWithFieldTypeValueArgument)
{
  const char* src =
    "class C1[n: U32]\n"
    "class C2\n"
    "  let c1: C1[1] = C1[1]"
    "  fun get(f: C1[1]) =>\n"
    "    true\n"
    "  new create() =>\n"
    "    get(c1)";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, FunctionCallWithSubtypeOfValueDependentTypeMismatch)
{
  const char* src =
    "trait C1[n: U32]\n"
    "class C2 is C1[3]\n"
    "class C3\n"
    "  let c2: C2 = C2\n"
    "\n"
    "  fun get(c1: C1[4]) =>\n"
    "    true\n"
    "\n"
    "  new create() =>\n"
    "    get(c2)\n";

  TEST_ERROR(src);
}

TEST_F(VDTTest, FunctionCallWithSubtypeOfValueDependentType)
{
  const char* src =
    "trait C1[n: U32]\n"
    "class C2 is C1[4]\n"
    "class C3\n"
    "  let c2: C2 = C2\n"
    "\n"
    "  fun get(c1: C1[4]) =>\n"
    "    true\n"
    "\n"
    "  new create() =>\n"
    "    get(c2)\n";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, ValueDependentTypeOfParameterTypeAgree)
{
  const char* src =
    "trait T1[A, n: A]\n"
    "class C1 is T1[U32, 4]\n"
    "class C2 is T1[Bool, true]\n";
  TEST_COMPILE(src);
}

TEST_F(VDTTest, ValueDependentTypeNotOfParameterType)
{
  const char* src =
    "trait T1[A, n: A]\n"
    "class C1 is T1[U32, true]\n"
    "class C2 is T1[Bool, 4]\n";
  TEST_ERROR(src);
}

TEST_F(VDTTest, ValueDependentTypeOfParameterType)
{
  const char* src =
    "trait T1[A: (U32 | U64), n: A]\n"
    "  fun get(): A => n\n"
    "\n"
    "class C1 is T1[U32, 4]\n"
    "\n"
    "class C2\n"
    "  let c : C1 = C1\n"
    "  new create() =>\n"
    "    let u : U32 = c.get()\n";
  TEST_COMPILE(src);
}

TEST_F(VDTTest, ValueDependentTypeOfParameterTypeBadReturn)
{
  const char* src =
    "trait T1[A: (U32 | U64), n: A]\n"
    "  fun get(): A => n\n"
    "\n"
    "class C1 is T1[U32, 4]\n"
    "\n"
    "actor A1\n"
    "  let c : C1 = C1\n"
    "\n"
    "  new create() =>\n"
    "    let u : U64 = c.get()\n";
  TEST_ERROR(src);
}

TEST_F(VDTTest, TypeForThisForValueDependentType)
{
  const char* src =
    "class C1[n: U32]\n"
    "  new create() => this\n";
  TEST_COMPILE(src);
}

TEST_F(VDTTest, CreateValueDependentInClassConstructor)
{
  const char* src =
    "class C1[n: U32]\n"
    "class C2\n"
    "  new create() =>\n"
    "    let c1: C1[4] = C1[4]\n";
  TEST_COMPILE(src);
}

TEST_F(VDTTest, CreateValueDependentMisimatchInClassConstructor)
{
  const char* src =
    "class C1[n: U32]\n"
    "class C2\n"
    "  new create() =>\n"
    "    let c1: C1[4] = C1[70]\n";
  TEST_ERROR(src);
}

TEST_F(VDTTest, CreateValueDependentTypeInActorConstructor)
{
  const char* src =
    "class C1[n: U32]\n"
    "  fun get(): U32 => n\n"
    "\n"
    "actor A\n"
    "  new create() =>\n"
    "    let c: C1[2] = C1[2]\n";
  TEST_COMPILE(src);
}

TEST_F(VDTTest, FunctionCallWithTypeMismatchValueArgument)
{
  const char* src =
    "class C1[n: U32]\n"
    "class C2\n"
    "  fun get(f: C1[2]) =>\n"
    "    true\n"
    "  new create() =>\n"
    "    let c1: C1[1] ref = C1[1]\n"
    "    get(c1)";

  TEST_ERROR(src);
}

TEST_F(VDTTest, FunctionCallWithTypeValueArgument)
{
  const char* src =
    "class C1[n: U32]\n"
    "class C2\n"
    "  fun get(f: C1[1]) =>\n"
    "    true\n"
    "  new create() =>\n"
    "    let c1: C1[1] ref = C1[1]\n"
    "    get(c1)";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, IsSubTypeClassTrait)
{
  const char* src =
    "trait T1[n: U32]\n"

    "class C1 is T1[4]\n"

    "interface Test\n"
    "  fun z(c1: C1, t1: T1[4], t2: T1[57])";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, IsSubTypeClassTraitWithGenericValueDependentType)
{
  const char* src =
    "trait T1[A: (U32 | U64), n: A]\n"

    "class C1 is T1[U32, 4]\n"

    "interface Test\n"
    "  fun z(c1: C1, t1: T1[U32, 4], t2: T1[U64, 4], t3: T1[U32, 78])";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, ArbitraryValueDependentType)
{
  const char* src =
    "class C1[n: U32]\n"

    "class C2\n"
    "  fun apply[n: U32](c1: C1[n]) : C1[n] =>\n"
    "    c1";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, MatchingValueDependentType)
{
  const char* src =
    "class C1[n: U32]\n"

    "class C2\n"
    "  fun apply[n: U32](c1: C1[n], c2: C1[n]) : C1[n] =>\n"
    "    c1";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, MatchingValueDependentTypeCallSuccess)
{
  const char* src =
    "class C1[n: U32]\n"

    "class C2\n"
    "  fun apply[n: U32](c1: C1[n], c2: C1[n]) : C1[n] =>\n"
    "    c1\n"

    "class C3\n"
    "  fun apply() =>\n"
    "    C2.apply[4](C1[4], C1[4])";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, MatchingValueDependentTypeCallFailure)
{
  const char* src =
    "class C1[n: U32]\n"

    "class C2\n"
    "  fun apply[n: U32](c1: C1[n], c2: C1[n]) : C1[n] =>\n"
    "    c1\n"

    "class C3\n"
    "  fun apply() =>\n"
    "    C2.apply[4](C1[4], C1[92])";

  TEST_ERROR(src);
}

TEST_F(VDTTest, DISABLED_BadParamFuctionReturnType)
{
  const char* src =
    "class C1[n: U32]\n"

    "class C2\n"
    "  fun apply[n: U32]() : C1[4] =>\n"
    "    C1[n]";

  TEST_ERROR(src);
}

TEST_F(VDTTest, ReificationWithReversedOrder)
{
  const char* src =
    "class C1[n: (U32 | U64), m: (U32 | U64)]\n"

    "class C2[n: (U32 | U64), m: (U32 | U64)]\n"
    "  let c: C1[m, n] = C1[m, n]\n";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, TestReifiedConstraint)
{
  const char* src =
    "class C1[A, n: A]\n"

    "class C2\n"
    "  let c: C1[U32, 2] = C1[U32, 2]";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, TestBadConstraint)
{
  const char* src =
    "class C1[n: U32, m: n]";

  TEST_ERROR(src);
}

TEST_F(VDTTest, TestMultipleTraitInstantiationWithMethod)
{
  const char* src =
    "trait T1[n: U32]\n"
    "  fun foo(): U32 => n\n"
    "class C1 is (T1[2] & T1[68])";

  TEST_ERROR(src);
}

TEST_F(VDTTest, TestNestedReachabilityCreatingTyperef)
{
  const char* src =
    "class C1[u:USize]\n"
    "  fun get() : USize =>\n"
    "    u\n"
    "class C2[n:USize]\n"
    "  let c1 : C1[n] = C1[n]\n"
    "  fun get() : USize =>\n"
    "    c1.get()\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    var c2 = C2[22]\n"
    "    let x = c2.get()\n";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, TestDefaultValueTypeParameters)
{
  const char* src =
    "class C1[n:USize = 5]\n"
    "  fun get() : USize =>\n"
    "    n\n"
    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    let c1 = C1\n"
    "    c1.get()\n"
    "    get()\n"
    "  fun get[u:USize = 8]() : USize =>\n"
    "    u\n";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, TestNestedReifications)
{
  const char* src =
    "class C1[n: U32]\n"
    "  fun apply(): U32 => n\n"
    "class C2\n"
    "  fun foo[m: U32](c: C1[m]): U32 => c()\n"
    "  fun bar[n: U32](c: C1[n]): U32 => foo[n](c)\n"
    "  new create() => bar[12](C1[12])";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, DISABLED_TestCompileTimeExpressionScope)
{
  const char* src =
    "class C1\n"
    "  new create() =>\n"
    "    let x: U32 = # (let y: U32; y + 2)\n"
    "    let z: U32 = y";

  TEST_ERROR(src);
}

// FIXME: these tests fails as the operations cannot be found due to builtin
// not being included -- fail in dot_or_tilde, these do not fail when
// run outside of the testing framework
TEST_F(VDTTest, DISABLED_IsSubTypeClassWithCompileConstantGenericValueDependentType)
{
  const char* src =
    "trait T1[A: (U32 | U64), n: A]\n"

    "class C1 is T1[U32, 4]\n"

    "interface Test\n"
    "  fun z(c1: C1, t1: T1[U32, #(1+3)])";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, DISABLED_ExpressionEqualityOfTypeArgs)
{
  const char* src =
    "class C1[n: U32]\n"

    "class C2\n"
    "  let c: C1[4] = C1[#(1 + 3)]";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, DISABLED_ValueTypeSumOfInputTypes)
{
  const char* src =
    "class C1[n: U32]\n"

    "class C2\n"
    "  fun apply[n: U32, m: U32]() =>\n"
    "    C1[#(n + m)]";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, DISABLED_ReturnTypeSumOfInputTypesCallSuccess)
{
  const char* src =
    "class C1[n: U32]\n"

    "class C2\n"
    "  fun apply(c1: C1[n], c2: C1[m]) : C1[n + m] =>\n"
    "    C1[n + m]\n"

    "class C3\n"
    "    let c: C1[6] = C2.create().apply(C1[4], C1[2])";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, DISABLED_ReturnTypeSumOfInputTypesCallError)
{
  const char* src =
    "class C1[n: U32]\n"

    "class C2\n"
    "  fun apply(c1: C1[n], c2: C1[m]) : C1[n + m] =>\n"
    "    C1[n + m]\n"

    "class C3\n"
    "    let c: C1[6] = C2.create().apply(C1[4], C1[92])";

  TEST_ERROR(src);
}

TEST_F(VDTTest, DISABLED_FBoundedPolymorphicClass)
{
  const char* src =
    "class Foo[n: Foo[n]]\n";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, VDTClassInheritsFromInterface)
{
  const char* src =
    "class Vector[A, size: USize] is Seq[A]";
  TEST_ERROR(src);
}

TEST_F(VDTTest, DISABLED_VDTTypeWithCompileTimeConstant)
{
  const char* src =
    "class C1[n: U32]\n"
    "  fun apply(): U32 => n\n"
    "  fun join[m: U32](): C1[#(n + m)] =>\n"
    "    C1[#(n + m)]\n"
    "class C2\n"
    "  new create() =>\n"
    "    let c1: C1[82] = C1[82]\n"
    "    let c2: C1[700] = c1.join[618]()";

  TEST_COMPILE(src);
}

TEST_F(VDTTest, DISABLED_VDTTypeWithCompileTimeConstantError)
{
  const char* src =
    "class C1[n: U32]\n"
    "  fun apply(): U32 => n\n"
    "  fun join[m: U32](): C1[#(n + m)] =>\n"
    "    C1[#(n + m)]\n"
    "class C2\n"
    "  new create() =>\n"
    "    let c1: C1[82] = C1[82]\n"
    "    let c2: C1[408] = c1.join[618]()";

  TEST_ERROR(src);
}

