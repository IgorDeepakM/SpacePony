#include <gtest/gtest.h>
#include <platform.h>

#include "util.h"


#define TEST_COMPILE(src) DO(test_compile(src, "ir"))

#define TEST_ERROR(src, err) \
  { const char* errs[] = {err, NULL}; \
    DO(test_expected_errors(src, "ir", errs)); }

#define TEST_ERRORS_2(src, err1, err2) \
  { const char* errs[] = {err1, err2, NULL}; \
    DO(test_expected_errors(src, "ir", errs)); }


class IftypeTest : public PassTest
{};


TEST_F(IftypeTest, ThenClause_TypeConstraint)
{
  const char* src =
    "trait T\n"
    "class C is T\n"
    "  fun tag c() => None\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C](C)\n"

    "  fun foo[A: T](x: A) =>\n"
    "    iftype A <: C then\n"
    "      x.c()\n"
    "    end";

  TEST_COMPILE(src);
}


TEST_F(IftypeTest, ElseClause_NoTypeConstraint)
{
  const char* src =
    "trait T\n"
    "class C is T\n"
    "  fun tag c() => None\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C](C)\n"

    "  fun foo[A: T](x: A) =>\n"
    "    iftype A <: C then\n"
    "      None\n"
    "    else\n"
    "      x.c()\n"
    "    end";

  TEST_ERROR(src, "couldn't find 'c' in 'T'");
}


TEST_F(IftypeTest, ThenClause_CapConstraint)
{
  const char* src =
    "class C\n"
    "  fun c() => None\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C](C)\n"

    "  fun foo[A: C](x: A) =>\n"
    "    iftype A <: C box then\n"
    "      x.c()\n"
    "    end";

  TEST_COMPILE(src);
}


TEST_F(IftypeTest, ElseClause_NoCapConstraint)
{
  const char* src =
    "class C\n"
    "  fun c() => None\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C](C)\n"

    "  fun foo[A: C](x: A) =>\n"
    "    iftype A <: C box then\n"
    "      None\n"
    "    else\n"
    "      x.c()\n"
    "    end";

  TEST_ERROR(src, "receiver type is not a subtype of target type");
}


TEST_F(IftypeTest, Cond_InvalidType)
{
  const char* src =
    "trait T\n"
    "class C is T\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C]()\n"

    "  fun foo[A: T]() =>\n"
    "    iftype C <: A then\n"
    "      None\n"
    "    end";

  TEST_ERROR(src, "the subtype in an iftype condition must be a type parameter "
    "or a tuple of type parameters");
}


TEST_F(IftypeTest, TupleCond_Valid)
{
  const char* src =
    "trait T1\n"
    "trait T2\n"
    "class C1 is T1\n"
    "class C2 is T2\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C1, C2]()\n"

    "  fun foo[A: T1, B: T2]() =>\n"
    "    iftype (A, B) <: (C1, C2) then\n"
    "      None\n"
    "    end";

  TEST_COMPILE(src);
}


TEST_F(IftypeTest, TupleCond_InvalidType)
{
  const char* src =
    "trait T1\n"
    "trait T2\n"
    "class C1 is T1\n"
    "class C2 is T2\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C1, C2]()\n"

    "  fun foo[A: T1, B: T2]() =>\n"
    "    iftype (A, B) <: C1 then\n"
    "      None\n"
    "    end";

  TEST_ERROR(src, "iftype subtype is a tuple but supertype isn't");
}


TEST_F(IftypeTest, TupleCond_InvalidCardinality)
{
  const char* src =
    "trait T1\n"
    "trait T2\n"
    "class C1 is T1\n"
    "class C2 is T2\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C1, C2]()\n"

    "  fun foo[A: T1, B: T2]() =>\n"
    "    iftype (A, B) <: (C1, C2, C1) then\n"
    "      None\n"
    "    end";

  TEST_ERROR(src, "the subtype and the supertype in an iftype condition must "
    "have the same cardinality");
}

TEST_F(IftypeTest, UnconstrainedTypeparam)
{
  const char* src =
    "class C\n"
    "  fun tag c() => None\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C](C)\n"

    "  fun foo[A](x: A) =>\n"
    "    iftype A <: C then\n"
    "      x.c()\n"
    "    end";

  TEST_COMPILE(src);
}

TEST_F(IftypeTest, RecursiveConstraint)
{
  const char* src =
    "trait T[X: T[X] #any]\n"
    "  fun tag m(): X\n"

    "class val C is T[C]\n"
    "  fun tag m(): C => C.create()\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C](C)\n"

    "  fun foo[A](x': A) =>\n"
    "    var x = x'\n"
    "    iftype A <: T[A] then\n"
    "      x = x.m()\n"
    "    end";

  TEST_COMPILE(src);
}


TEST_F(IftypeTest, Tuple_MutuallyRecursiveConstraint)
{
  const char* src =
    "trait T[X]\n"
    "  fun tag m(): X\n"

    "class val C is T[D]\n"
    "  fun tag m(): D => D.create()\n"

    "class val D is T[C]\n"
    "  fun tag m(): C => C.create()\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C,D](C, D)\n"

    "  fun foo[A, B](x': A, y': B) =>\n"
    "    var x = x'\n"
    "    var y = y'\n"
    "    iftype (A, B) <: (T[B], T[A]) then\n"
    "      x = y.m()\n"
    "      y = x.m()\n"
    "    end";

  TEST_COMPILE(src);
}


TEST_F(IftypeTest, NestedCond)
{
  const char* src =
    "trait T\n"
    "class C is T\n"
    "  fun tag c() => None\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C](C)\n"

    "  fun foo[A](x: A) =>\n"
    "    iftype A <: T then\n"
    "      iftype A <: C then\n"
    "        x.c()\n"
    "      end\n"
    "    end";

  TEST_COMPILE(src);
}


TEST_F(IftypeTest, ReplaceTypeargs)
{
  const char* src =
    "class A\n"
    "  fun m() => None\n"
    "trait Cell[X]\n"
    "  fun get(): X\n"

    "actor Main\n"
    "  fun foo[X: Any #read](cell: Cell[X]) =>\n"
    "    iftype X <: A #read\n"
    "    then cell.get().m()\n"
    "    end\n"
    "  new create(env: Env) => None";
  TEST_COMPILE(src);
}

TEST_F(IftypeTest, InsideLambda)
{
  // From #3762
  const char* src =
    "primitive DoIt[T: (U8 | I8)]\n"
    "  fun apply(arg: T): T =>\n"
    "    let do_it = {(v: T): T =>\n"
    "      iftype T <: U8 then\n"
    "        v\n"
    "      else\n"
    "        v\n"
    "      end\n"
    "    }\n"
    "    do_it(arg)\n"

    "actor Main\n"
    "  new create(env: Env) => None";

    TEST_COMPILE(src);
}

TEST_F(IftypeTest, TestUnderlyingType)
{
  const char* src =
    "class C1\n"
    "struct S1\n"
    "primitive P1\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    foo[C1]()\n"
    "    foo[S1]()\n"
    "    foo[P1]()\n"

    "  fun foo[A: AnyNoCheck]() =>\n"
    "    iftype A <: class then\n"
    "      None\n"
    "    end\n"
    "    iftype A <: struct then\n"
    "      None\n"
    "    end\n"
    "    iftype A <: (primitive | struct) then\n"
    "      None\n"
    "    end";

  TEST_COMPILE(src);
}

TEST_F(IftypeTest, TestIfTypeMethodOk)
{
  const char* src =
    "primitive P[A]\n"
    "  iftype A <: F64 then\n"
    "    fun float_op(x : A) =>\n"
    "      None\n"
    "  elseif A <: F32 then\n"
    "    fun float_op2(x : A) =>\n"
    "      None\n"
    "  else\n"
    "    fun float_op3(x: A) =>\n"
    "      None\n"
    "    fun float_op4(x: A) =>\n"
    "      None\n"
    "  end\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    P[F64].float_op(0.5)\n"
    "    P[F32].float_op2(0.5)";

  TEST_COMPILE(src);
}

TEST_F(IftypeTest, TestIfTypeMethodFail)
{
  const char* src =
    "primitive P[A]\n"
    "  iftype A <: F64 then\n"
    "    fun float_op(x : A) =>\n"
    "      None\n"
    "  elseif A <: F32 then\n"
    "    fun float_op2(x : A) =>\n"
    "      None\n"
    "  else\n"
    "    fun float_op3(x: A) =>\n"
    "      None\n"
    "    fun float_op4(x: A) =>\n"
    "      None\n"
    "  end\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    P[F64].float_op3(0.5)\n"
    "    P[F32].float_op4(0.5)";

  TEST_ERRORS_2(src,
    "Method float_op3 in type P_F64_val_o not found because it was removed in an iftype expression",
    "Method float_op4 in type P_F32_val_o not found because it was removed in an iftype expression");
}

TEST_F(IftypeTest, TestIfTypeNestedMethodOk)
{
  const char* src =
    "primitive P[A:(Real[A] val & Number)]\n"
    "  iftype A <: (F32 | F64) then\n"
    "    iftype A <: F64 then\n"
    "      fun float_op(x : A) =>\n"
    "        None\n"
    "    elseif A <: F32 then\n"
    "      fun float_op2(x : A) =>\n"
    "        None\n"
    "    else\n"
    "      fun float_op3(x: A) =>\n"
    "        None\n"
    "      fun float_op4(x: A) =>\n"
    "        None\n"
    "    end\n"
    "  end\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    P[F64].float_op(0.5)\n"
    "    P[F32].float_op2(0.5)";

  TEST_COMPILE(src);
}

TEST_F(IftypeTest, TestIfTypeNestedMethodFail)
{
  const char* src =
    "primitive P[A:(Real[A] val & Number)]\n"
    "  iftype A <: (F32 | F64) then\n"
    "    iftype A <: F64 then\n"
    "      fun float_op(x : A) =>\n"
    "        None\n"
    "    elseif A <: F32 then\n"
    "      fun float_op2(x : A) =>\n"
    "        None\n"
    "    else\n"
    "      fun float_op3(x: A) =>\n"
    "        None\n"
    "      fun float_op4(x: A) =>\n"
    "        None\n"
    "    end\n"
    "  end\n"

    "actor Main\n"
    "  new create(env: Env) =>\n"
    "    P[F64].float_op3(0.5)\n"
    "    P[F32].float_op4(0.5)";

  TEST_ERRORS_2(src,
    "Method float_op3 in type P_F64_val_o not found because it was removed in an iftype expression",
    "Method float_op4 in type P_F32_val_o not found because it was removed in an iftype expression");
}