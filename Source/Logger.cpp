#include "stdafx.h"
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

  void Logger::Log(const std::string& message)
  {
#ifdef __EMSCRIPTEN__
    String emLog = message + "\n";
    printf("%s", emLog.c_str());
#endif
    m_logFile << message << std::endl;
  }

}
