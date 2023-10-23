#pragma once

// TODO make DEFINE macros for CPU profiles for both function range & push-pop

// CPU profile
#define NOMINMAX
#include "nvtx3.hpp"
#undef WriteConsole
#undef far

// GPU profile
#ifdef TK_GPU_PROFILE
  #include "TKOpenGL.h"
  #include "Types.h"
  #define PUSH_GPU_MARKER(msg) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, ##msg)
  #define POP_GPU_MARKER()     glPopDebugGroup()
#else
  #define PUSH_GPU_MARKER(msg)
  #define POP_GPU_MARKER()
#endif
