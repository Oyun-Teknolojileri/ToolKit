#pragma once

#include "Types.h"
#include <fstream>

namespace ToolKit
{
  enum class LogType
  {
    Memo,
    Error,
    Warning,
    Command
  };

  class TK_API Logger
  {
   public:
    Logger();
    ~Logger();
    void Log(const String& message);
    void SetWriteConsoleFn(std::function<void(LogType, String)> fn);
    void WriteConsole(LogType logType, const char* msg, ...);

   private:
    std::ofstream m_logFile;
    std::function<void(LogType, String)> m_writeConsoleFn;
  };
} // namespace ToolKit
