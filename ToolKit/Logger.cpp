#include "Logger.h"
#include "ToolKit.h"
#include "DebugNew.h"

namespace ToolKit
{

  Logger::Logger()
  {
    m_logFile.open("Log.txt", std::ios::out);
  }

  Logger::~Logger()
  {
    m_logFile.close();
  }

  void Logger::Log(const String& message)
  {
#ifdef __EMSCRIPTEN__
    String emLog = message + "\n";
    printf("%s", emLog.c_str());
#endif
    m_logFile << message << std::endl;
  }

  void Logger::SetWriteConsoleFn(std::function<void(LogType, String)> fn)
  {
    m_writeConsoleFn = fn;
  }

  void Logger::WriteConsole(LogType logType, const char* msg, ...)
  {
    va_list args;
    va_start(args, msg);

    static char buff[2048];
    vsprintf(buff, msg, args);

    if (m_writeConsoleFn != nullptr)
    {
      m_writeConsoleFn(logType, String(buff));
    }

    va_end(args);
  }
}  // namespace ToolKit
