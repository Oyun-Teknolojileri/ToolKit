#include "Logger.h"

#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

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

	void Logger::SetWriteConsoleFn(ConsoleOutputFn fn)
  {
    m_writeConsoleFn = fn;
  }
	
	void Logger::SetPlatformConsoleFn(ConsoleOutputFn fn)
	{
		m_platfromConsoleFn = fn;
	}

	void OutputUtil(ConsoleOutputFn logFn, LogType logType, const char* msg, ...)
	{
		if (logFn == nullptr)
		{
			return;
		}

		va_list args;
		va_start(args, msg);

		static char buff[2048];
		vsprintf(buff, msg, args);

		logFn(logType, String(buff));

		va_end(args);
	}

  void Logger::WriteConsole(LogType logType, const char* msg, ...)
  {
		OutputUtil(m_writeConsoleFn, logType, msg);
  }

	void Logger::WritePlatformConsole(LogType logType, const char* msg, ...)
	{
		OutputUtil(m_platfromConsoleFn, logType, msg);
	}

} // namespace ToolKit
