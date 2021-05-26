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

  void Logger::Log(String message)
  {
    m_logFile << message << std::endl;
  }

}
