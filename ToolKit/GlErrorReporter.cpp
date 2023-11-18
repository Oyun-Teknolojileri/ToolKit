/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "GlErrorReporter.h"

#include "Logger.h"
#include "TKOpenGL.h"
#include "ToolKit.h"

namespace ToolKit
{
  GlReportCallback GlErrorReporter::Report = [](const String& msg) -> void { GetLogger()->Log(msg); };

  void InitGLErrorReport(GlReportCallback callback)
  {
    if (glDebugMessageCallback != NULL)
    {
      glEnable(GL_DEBUG_OUTPUT);
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback(&GLDebugMessageCallback, nullptr);
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
      glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
    }

    if (callback)
    {
      GlErrorReporter::Report = callback;
    }
  }

  void GLDebugMessageCallback(GLenum source,
                              GLenum type,
                              GLuint id,
                              GLenum severity,
                              GLsizei length,
                              const GLchar* msg,
                              const void* data)
  {
    GlErrorReporter::Report(msg);
  }

} // namespace ToolKit
