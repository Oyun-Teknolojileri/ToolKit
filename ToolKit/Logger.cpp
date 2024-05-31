/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Logger.h"

#include "ToolKit.h"

namespace ToolKit
{
  const uint64 MessageBufferLength = 4096;
  static char MessageBuffer[MessageBufferLength];

  void OutputUtil(ConsoleOutputFn logFn, LogType logType, const char* msg, va_list args)
  {
    if (logFn == nullptr)
    {
      return;
    }

    vsprintf(MessageBuffer, msg, args);

    logFn(logType, String(MessageBuffer));
  }

  Logger::Logger() { m_logFile.open("Log.txt", std::ios::out); }

  Logger::~Logger() { m_logFile.close(); }

  void Logger::Log(const String& message)
  {
#ifdef __EMSCRIPTEN__
    String emLog = message + "\n";
    printf("%s", emLog.c_str());
#endif
    m_logFile << message << std::endl;
  }

  void Logger::Log(LogType logType, const char* msg, ...)
  {
    va_list args;
    va_start(args, msg);

    static const char* logTypes[] = {"[Memo]", "[Error]", "[Warning]", "[Command]"};

    vsprintf(MessageBuffer, msg, args);

    m_logFile << logTypes[(int) logType] << MessageBuffer << std::endl;

    if (m_writeConsoleFn != nullptr)
    {
      m_writeConsoleFn(logType, MessageBuffer);
    }

    if (m_platfromConsoleFn != nullptr)
    {
      m_platfromConsoleFn(logType, MessageBuffer);
    }

    va_end(args);
  }

  void Logger::SetWriteConsoleFn(ConsoleOutputFn fn) { m_writeConsoleFn = fn; }

  void Logger::SetClearConsoleFn(ClearConsoleFn fn) { m_clearConsoleFn = fn; }

  void Logger::SetPlatformConsoleFn(ConsoleOutputFn fn) { m_platfromConsoleFn = fn; }

  void Logger::ClearConsole() { m_clearConsoleFn(); }

  void Logger::WriteTKConsole(LogType logType, const char* msg, ...)
  {
    if (strlen(msg) >= MessageBufferLength)
    {
      if (m_platfromConsoleFn)
      {
        m_platfromConsoleFn(LogType::Warning, "Maximum size for WriteConsole exceeded, cannot format.");
        m_platfromConsoleFn(logType, msg);
      }
      return;
    }
    va_list args;
    va_start(args, msg);

    OutputUtil(m_writeConsoleFn, logType, msg, args);

    // Echo to platform console.
    if (m_platfromConsoleFn)
    {
      OutputUtil(m_platfromConsoleFn, logType, msg, args);
    }

    va_end(args);
  }

  void Logger::WritePlatformConsole(LogType logType, const char* msg, ...)
  {
    if (strlen(msg) >= MessageBufferLength)
    {
      if (m_platfromConsoleFn)
      {
        m_platfromConsoleFn(logType, "Maximum size for WriteConsole exceeded, cannot format.");
        m_platfromConsoleFn(logType, msg);
      }
      return;
    }
    va_list args;
    va_start(args, msg);

    OutputUtil(m_platfromConsoleFn, logType, msg, args);

    va_end(args);
  }

} // namespace ToolKit
