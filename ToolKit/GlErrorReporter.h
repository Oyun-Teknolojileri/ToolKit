#pragma once

#include "ToolKit.h"
#include "Types.h"
#include "gles2.h"

#include <functional>
#include <string>

namespace ToolKit
{

  class GlErrorReporter
  {
   public:
    // Override it as you see fit. This lambda will be called with the opengl
    // error message.
    static GlReportCallback Report;
  };

  void GLDebugMessageCallback(GLenum source,
                              GLenum type,
                              GLuint id,
                              GLenum severity,
                              GLsizei length,
                              const GLchar* msg,
                              const void* data);

  void InitGLErrorReport(GlReportCallback callback = nullptr);

} // namespace ToolKit
