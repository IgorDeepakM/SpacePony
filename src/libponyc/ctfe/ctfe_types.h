#pragma once

#include "../common/platform.h"


enum class CtfeValueType
{
  None,
  Bool,
  IntLiteral,
  TypedIntI8,
  TypedIntU8,
  TypedIntI16,
  TypedIntU16,
  TypedIntI32,
  TypedIntU32,
  TypedIntI64,
  TypedIntU64,
  TypedIntI128,
  TypedIntU128,
  TypedIntILong,
  TypedIntULong,
  TypedIntISize,
  TypedIntUSize,
  RealLiteral,
  String,
  StructRef,
  Tuple,
  StringLiteral,
  Pointer,
  TypeRef
};


#if defined(PLATFORM_IS_WINDOWS)
  #include <__msvc_int128.hpp>
  using CtfeI128Type = std::_Signed128;
  using CtfeU128Type = std::_Unsigned128;
#else
  using CtfeI128Type = __int128;
  using CtfeU128Type = unsigned __int128;
#endif
