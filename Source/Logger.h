#pragma once

#include <string>
#include "Types.h"
#include <fstream>

namespace ToolKit
{

  class TK_API Logger
  {
  public:
    Logger();
    ~Logger();
    void Log(const String& message);

  private:
    std::ofstream m_logFile;
  };

}
