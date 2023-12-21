#include "RHI.h"

namespace ToolKit
{
  GLuint RHI::m_currentReadFramebufferID = -1; // max unsigned integer
  GLuint RHI::m_currentDrawFramebufferID = -1; // max unsigned integer
  GLuint RHI::m_currentFramebufferID     = -1; // max unsigned integer
} // namespace ToolKit
