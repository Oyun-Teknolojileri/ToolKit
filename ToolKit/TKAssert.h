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

#define TK_ASSERT_ONCE(expr)                                                   \
  {                                                                            \
    static std::atomic<bool> flag {false};                                     \
    ToolKit::assert_once<flag>(expr);                                          \
  }