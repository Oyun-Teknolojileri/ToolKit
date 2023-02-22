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

	typedef std::function<void(LogType, const String&)> ConsoleOutputFn;

  class TK_API Logger
  {
   public:
    Logger();
    ~Logger();
    void Log(const String& message);
    void SetWriteConsoleFn(ConsoleOutputFn fn);
    void SetPlatformConsoleFn(ConsoleOutputFn fn);
		void WriteConsole(LogType logType, const char* msg, ...);
		void WritePlatformConsole(LogType logType, const char* msg, ...);

   private:
    std::ofstream m_logFile;
    ConsoleOutputFn m_writeConsoleFn = nullptr;
    ConsoleOutputFn m_platfromConsoleFn = nullptr;
  };
} // namespace ToolKit
