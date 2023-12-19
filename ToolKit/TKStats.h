#pragma once

#include "TKAssert.h"

namespace ToolKit
{
  class TK_API TKStats
  {
   public:
    inline uint64 GetTotalVRAMUsageInBytes() { return m_totalVRAMUsageInBytes; }

    inline uint64 GetTotalVRAMUsageInKB() { return m_totalVRAMUsageInBytes / 1024; }

    inline uint64 GetTotalVRAMUsageInMB() { return m_totalVRAMUsageInBytes / (1024 * 1024); }

    inline void AddVRAMUsageInBytes(uint64 bytes)
    {
      m_totalVRAMUsageInBytes += bytes;
    }

    inline void RemoveVRAMUsageInBytes(uint64 bytes)
    {
      uint64 old               = m_totalVRAMUsageInBytes; 

      if (m_totalVRAMUsageInBytes < bytes)
      {
        TK_ASSERT_ONCE(false);
      }

      m_totalVRAMUsageInBytes -= bytes;
    }

    inline void ResetVRAMUsage() { m_totalVRAMUsageInBytes = 0; }

   private:
    uint64 m_totalVRAMUsageInBytes = 0;
  };
} // namespace ToolKit
