#include "GlErrorReporter.h"

namespace ToolKit
{

  GlReportCallback GlErrorReporter::Report = [](const std::string& msg) -> void
  { GetLogger()->Log(msg); };

  void InitGLErrorReport(GlReportCallback callback)
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
