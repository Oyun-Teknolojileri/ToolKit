#pragma once

#include "TKAssert.h"

namespace ToolKit
{
  class TKStats
  {
   public:
    static inline uint GetTotalVRAMUsageInBytes() { return m_totalVRAMUsageInBytes; }

    static inline uint GetTotalVRAMUsageInKB() { return m_totalVRAMUsageInBytes / 1024; }

    static inline uint GetTotalVRAMUsageInMB() { return m_totalVRAMUsageInBytes / (1024 * 1024); }

    static inline void AddVRAMUsageInBytes(uint bytes) { m_totalVRAMUsageInBytes += bytes; }

    static inline void RemoveVRAMUsageInBytes(uint bytes)
    {
      m_totalVRAMUsageInBytes -= bytes;
      if (m_totalVRAMUsageInBytes < 0)
      {
        m_totalVRAMUsageInBytes = 0;
        TK_ASSERT_ONCE(false);
      }
    }

    static inline void ResetVRAMUsage() { m_totalVRAMUsageInBytes = 0; }

   private:
    static uint m_totalVRAMUsageInBytes;
  };
} // namespace ToolKit
