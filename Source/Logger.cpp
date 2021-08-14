#include "stdafx.h"
#include "Logger.h"
#include "ToolKit.h"
#include "DebugNew.h"

namespace ToolKit
{

  Logger Logger::m_logger;

  Logger::Logger()
  {
    m_logFile.open("Log.txt", std::ios::out);
    assert(m_logFile.good());
  }

  Logger::~Logger()
  {
    m_logFile.close();
  }

  Logger* Logger::GetInstance()
  {
    return &m_logger;
  }

  void Logger::Log(const std::string& message)
  {
#ifdef __EMSCRIPTEN__
    String emLog = message + "\n";
    printf(emLog.c_str());
#endif
    m_logFile << message << std::endl;
  }

}
