#pragma once

#include "GL/glew.h"
#include "ToolKit.h"

#include <functional>
#include <string>

class GlErrorReporter
{
 public:
  typedef std::function<void(const std::string&)> ReportCallback;
  static ReportCallback Report;
};

// Override it as you see fit. This lambda will be called with the opengl error
// message.
GlErrorReporter::ReportCallback GlErrorReporter::Report =
    [](const std::string& msg) -> void { ToolKit::GetLogger()->Log(msg); };

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

void InitGLErrorReport(GlErrorReporter::ReportCallback callback = nullptr)
{
#ifdef TK_GL_CORE_3_2
  if (glDebugMessageCallback != NULL)
  {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageCallback(&GLDebugMessageCallback, nullptr);

    glDebugMessageControl(GL_DONT_CARE,
                          GL_DONT_CARE,
                          GL_DONT_CARE,
                          0,
                          NULL,
                          GL_FALSE);

    glDebugMessageControl(GL_DEBUG_SOURCE_API,
                          GL_DEBUG_TYPE_ERROR,
                          GL_DEBUG_SEVERITY_HIGH,
                          0,
                          NULL,
                          GL_TRUE);
  }
#endif

  if (callback) 
  {
    GlErrorReporter::Report = callback;
  }
}