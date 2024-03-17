/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include <atomic>
#include <cassert>

namespace ToolKit
{

  template <std::atomic<bool>& flag>
  void assert_once(bool expr)
  {
    if (!flag && !expr)
    {                // check if the expression is false and the flag is false
      flag = true;   // set the flag to true
      assert(false); // break the application with assert
    }
  }

} // namespace ToolKit

#ifdef TK_DEBUG

  #define TK_ASSERT_ONCE(expr)                                                                                         \
    {                                                                                                                  \
      static std::atomic<bool> flag {false};                                                                           \
      ToolKit::assert_once<flag>(expr);                                                                                \
    }

#else
  #define TK_ASSERT_ONCE(expr) ((void) 0);
#endif