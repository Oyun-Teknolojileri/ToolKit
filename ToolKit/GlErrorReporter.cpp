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

#include <iostream>
#include <sstream>

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

  GLenum glCheckError_(const char* file, int line)
  {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
      std::string error;
      switch (errorCode)
      {
      case GL_INVALID_ENUM:
        error = "INVALID_ENUM";
        break;
      case GL_INVALID_VALUE:
        error = "INVALID_VALUE";
        break;
      case GL_INVALID_OPERATION:
        error = "INVALID_OPERATION";
        break;
      case GL_STACK_OVERFLOW:
        error = "STACK_OVERFLOW";
        break;
      case GL_STACK_UNDERFLOW:
        error = "STACK_UNDERFLOW";
        break;
      case GL_OUT_OF_MEMORY:
        error = "OUT_OF_MEMORY";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        error = "INVALID_FRAMEBUFFER_OPERATION";
        break;
      }

      std::ostringstream oss;
      oss << error << " | " << file << " (" << line << ")" << std::endl;

      GlErrorReporter::Report(oss.str());
    }
    return errorCode;
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
