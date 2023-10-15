/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Logger.h"

#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{
  const uint64 buffLen = 4096;
  static char buff[buffLen];

  void OutputUtil(ConsoleOutputFn logFn, LogType logType, const char* msg, va_list args)
  {
    if (logFn == nullptr)
    {
      return;
    }

    vsprintf(buff, msg, args);

    logFn(logType, String(buff));
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

    vsprintf(buff, msg, args);

    m_logFile << logTypes[(int) logType] << buff << std::endl;

    if (m_writeConsoleFn != nullptr)
    {
      m_writeConsoleFn(logType, buff);
    }

    if (m_platfromConsoleFn != nullptr)
    {
      m_platfromConsoleFn(logType, buff);
    }

    va_end(args);
  }

  void Logger::SetWriteConsoleFn(ConsoleOutputFn fn) { m_writeConsoleFn = fn; }

  void Logger::SetClearConsoleFn(ClearConsoleFn fn) { m_clearConsoleFn = fn; }

  void Logger::SetPlatformConsoleFn(ConsoleOutputFn fn) { m_platfromConsoleFn = fn; }

  void Logger::ClearConsole() { m_clearConsoleFn(); }

  void Logger::WriteConsole(LogType logType, const char* msg, ...)
  {
    if (strlen(msg) >= buffLen)
    {
      m_writeConsoleFn(LogType::Warning, "maximum size for WriteConsole exceeded, cannot format.");
      m_writeConsoleFn(logType, msg);
      return;
    }
    va_list args;
    va_start(args, msg);

    OutputUtil(m_writeConsoleFn, logType, msg, args);

    va_end(args);
  }

  void Logger::WritePlatformConsole(LogType logType, const char* msg, ...)
  {
    if (strlen(msg) >= buffLen)
    {
      m_platfromConsoleFn(logType, "maximum size for WriteConsole exceeded, cannot format.");
      m_platfromConsoleFn(logType, msg);
      return;
    }
    va_list args;
    va_start(args, msg);

    OutputUtil(m_platfromConsoleFn, logType, msg, args);

    va_end(args);
  }

} // namespace ToolKit
