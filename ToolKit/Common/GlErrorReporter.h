#pragma once

#include "GL/glew.h"
#include "ToolKit.h"

#include <functional>
#include <string>

class GlErrorReporter
{
 public:
  typedef std::function<void(const std::string&)> ReportFnPtr;
  static ReportFnPtr Report;
};

// Override it as you see fit. This lambda will be called with the opengl error
// message.
GlErrorReporter::ReportFnPtr GlErrorReporter::Report =
    [](const std::string& msg) -> void { ToolKit::GetLogger()->Log(msg); };

// https://gist.github.com/liam-middlebrook/c52b069e4be2d87a6d2f
void GLDebugMessageCallback(GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar* msg,
                            const void* data)
{
}
