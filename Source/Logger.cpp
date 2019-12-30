#include "stdafx.h"
#include "Logger.h"
#include "ToolKit.h"

ToolKit::Logger ToolKit::Logger::m_logger;

ToolKit::Logger::Logger()
{
  m_logFile.open("Log.txt", std::ios::out);
  assert(m_logFile.good());
}

ToolKit::Logger::~Logger()
{
  m_logFile.close();
}

ToolKit::Logger* ToolKit::Logger::GetInstance()
{
  return &m_logger;
}

void ToolKit::Logger::Log(std::string message)
{
  m_logFile << message << std::endl;
}
