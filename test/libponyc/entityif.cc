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


class EntityIfTest : public PassTest
{};


TEST_F(EntityIfTest, TestEntityIfOk)
{
  const char* src =
    "primitive P[A]\n"
    "  entityif A <: F64 then\n"
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

TEST_F(EntityIfTest, TestEntityIfFail)
{
  const char* src =
    "primitive P[A]\n"
    "  entityif A <: F64 then\n"
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

  TEST_ERROR(src,
    "Method float_op3 in type P_F64_val_o not found because it was removed in an entityif expression");
}

TEST_F(EntityIfTest, TestEntityIfNestedOk)
{
  const char* src =
    "primitive P[A:(Real[A] val & Number)]\n"
    "  entityif A <: (F32 | F64) then\n"
    "    entityif A <: F64 then\n"
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

TEST_F(EntityIfTest, TestEntityIfNestedFail)
{
  const char* src =
    "primitive P[A:(Real[A] val & Number)]\n"
    "  entityif A <: (F32 | F64) then\n"
    "    entityif A <: F64 then\n"
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

  TEST_ERROR(src,
    "Method float_op3 in type P_F64_val_o not found because it was removed in an entityif expression");
}
