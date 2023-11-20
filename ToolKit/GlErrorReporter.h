/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "TKOpenGL.h"
#include "Types.h"

namespace ToolKit
{

  class TK_API GlErrorReporter
  {
   public:
    // Override it as you see fit. This lambda will be called with the opengl
    // error message.
    static GlReportCallback Report;
  };

  TK_API void GLDebugMessageCallback(GLenum source,
                                     GLenum type,
                                     GLuint id,
                                     GLenum severity,
                                     GLsizei length,
                                     const GLchar* msg,
                                     const void* data);

  TK_API void InitGLErrorReport(GlReportCallback callback = nullptr);

  TK_API GLenum glCheckError_(const char* file, int line);

#ifdef TK_DEBUG
  #define TKCheckGL() glCheckError_(__FILE__, __LINE__)
#else
  #define TKCheckGL()
#endif

} // namespace ToolKit
