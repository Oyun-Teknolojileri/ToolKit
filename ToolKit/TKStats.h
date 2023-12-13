#pragma once

#include "TKAssert.h"

namespace ToolKit
{
  class TK_API TKStats
  {
   public:
    static inline uint64 GetTotalVRAMUsageInBytes() { return m_totalVRAMUsageInBytes; }

    static inline uint64 GetTotalVRAMUsageInKB() { return m_totalVRAMUsageInBytes / 1024; }

    static inline uint64 GetTotalVRAMUsageInMB() { return m_totalVRAMUsageInBytes / (1024 * 1024); }

    static inline void AddVRAMUsageInBytes(uint64 bytes)
    {
      volatile int yy           = 5;
      m_totalVRAMUsageInBytes += bytes;
      volatile int y           = 5;
    }

    static inline void RemoveVRAMUsageInBytes(uint64 bytes)
    {
      uint64 old               = m_totalVRAMUsageInBytes; 
      m_totalVRAMUsageInBytes -= bytes;
      if (old < m_totalVRAMUsageInBytes)
      {
        TK_ASSERT_ONCE(false);
      }
    }

    static inline void ResetVRAMUsage() { m_totalVRAMUsageInBytes = 0; }

   private:
    static uint64 m_totalVRAMUsageInBytes;
  };
} // namespace ToolKit
