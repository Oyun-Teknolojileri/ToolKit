#pragma once

// CPU profile
#ifdef TK_CPU_PROFILE
  #define NOMINMAX
  #include "nvtx3.hpp"
  #undef WriteConsole
  #undef far
  #define PUSH_CPU_MARKER(msg) nvtxRangePushA(##msg)
  #define POP_CPU_MARKER()     nvtxRangePop()
  #define CPU_FUNC_RANGE()     NVTX3_FUNC_RANGE()
  #define CPU_MARK(msg)        nvtx3::mark(##msg)
#else
  #define PUSH_CPU_MARKER(msg)
  #define POP_CPU_MARKER()
  #define CPU_FUNC_RANGE()
  #define CPU_MARK(msg)
#endif

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
