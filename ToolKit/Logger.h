/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

#include <fstream>

namespace ToolKit
{

#ifdef __clang__
  #define TK_LOG(format, ...) ToolKit::GetLogger()->WriteTKConsole(ToolKit::LogType::Memo, format, ##__VA_ARGS__)
  #define TK_WRN(format, ...) ToolKit::GetLogger()->WriteTKConsole(ToolKit::LogType::Warning, format, ##__VA_ARGS__)
  #define TK_ERR(format, ...) ToolKit::GetLogger()->WriteTKConsole(ToolKit::LogType::Error, format, ##__VA_ARGS__)
  #define TK_SUC(format, ...) ToolKit::GetLogger()->WriteTKConsole(ToolKit::LogType::Success, format, ##__VA_ARGS__)
#elif _MSC_VER
  #define TK_LOG(format, ...) ToolKit::GetLogger()->WriteTKConsole(ToolKit::LogType::Memo, format, __VA_ARGS__)
  #define TK_WRN(format, ...) ToolKit::GetLogger()->WriteTKConsole(ToolKit::LogType::Warning, format, __VA_ARGS__)
  #define TK_ERR(format, ...) ToolKit::GetLogger()->WriteTKConsole(ToolKit::LogType::Error, format, __VA_ARGS__)
  #define TK_SUC(format, ...) ToolKit::GetLogger()->WriteTKConsole(ToolKit::LogType::Success, format, __VA_ARGS__)
#endif

  enum class LogType
  {
    Memo,
    Error,
    Warning,
    Command,
    Success
  };

  typedef std::function<void(LogType, const String&)> ConsoleOutputFn;
  typedef std::function<void()> ClearConsoleFn;

  class TK_API Logger
  {
   public:
    Logger();
    ~Logger();
    void Log(const String& message);
    void Log(LogType logType, const char* msg, ...);
    void SetWriteConsoleFn(ConsoleOutputFn fn);
    void SetClearConsoleFn(ClearConsoleFn fn);
    void SetPlatformConsoleFn(ConsoleOutputFn fn);
    void ClearConsole();
    void WriteTKConsole(LogType logType, const char* msg, ...);
    void WritePlatformConsole(LogType logType, const char* msg, ...);

   private:
    std::ofstream m_logFile;
    ClearConsoleFn m_clearConsoleFn;
    ConsoleOutputFn m_writeConsoleFn    = nullptr;
    ConsoleOutputFn m_platfromConsoleFn = nullptr;
  };
} // namespace ToolKit
