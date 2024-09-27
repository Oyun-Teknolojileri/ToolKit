/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

namespace ToolKit
{

#include "Types.h"

  //  Gpu profile marker
  //////////////////////////////////////////

// GPU profile
#ifdef TK_GPU_PROFILE
  #include "TKOpenGL.h"
  #define PUSH_GPU_MARKER(msg) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, ##msg)
  #define POP_GPU_MARKER()     glPopDebugGroup()
#else
  #define PUSH_GPU_MARKER(msg)
  #define POP_GPU_MARKER()
#endif

  //  Profile timer
  //////////////////////////////////////////

  /** Global profile timer set. Timer macro access this set to display / hide timers. */
  static std::unordered_map<String, bool> g_profileTimerMap;

/**
 * Start point of a region based timer that measures time between two matching call.
 * Both call must be in same scope with the same Name.
 */
#define TKBeginTimer(Name)                                                                                             \
  static uint Name##_s_cnt = 0;                                                                                        \
  static bool Name##_i_l   = true;                                                                                     \
  if (Name##_i_l)                                                                                                      \
  {                                                                                                                    \
    if (g_profileTimerMap.find(#Name) == g_profileTimerMap.end())                                                      \
      g_profileTimerMap[#Name] = true;                                                                                 \
    Name##_i_l = false;                                                                                                \
  }                                                                                                                    \
  float Name##_t_start = GetElapsedMilliSeconds();

/** End point of a region based timer. */
#define TKEndTimer(Name)                                                                                               \
  float Name##_t_d         = GetElapsedMilliSeconds() - Name##_t_start;                                                \
  static float Name##_t_t  = 0.0f;                                                                                     \
  Name##_t_t              += Name##_t_d;                                                                               \
  if (g_profileTimerMap[#Name])                                                                                        \
  {                                                                                                                    \
    TK_LOG(#Name " avg t: %f -- t: %f", Name##_t_t / (float) ++Name##_s_cnt, Name##_t_d);                              \
  }

} // namespace ToolKit
