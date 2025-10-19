#pragma once

#include "../common/platform.h"

#if defined(PLATFORM_IS_WINDOWS)
  #include <__msvc_int128.hpp>
  using CtfeI128Type = std::_Signed128;
  using CtfeU128Type = std::_Unsigned128;
#else
  using CtfeI128Type = __int128;
  using CtfeU128Type = unsigned __int128;
#endif
