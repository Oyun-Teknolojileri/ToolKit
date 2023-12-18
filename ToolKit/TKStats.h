#pragma once

#include "TKAssert.h"

namespace ToolKit
{
  class TK_API TKStats
  {
   public:

    // Vram Usage
    ///////////////////////////////////////////////////////

    static inline uint64 GetTotalVRAMUsageInBytes() { return m_totalVRAMUsageInBytes; }

    static inline uint64 GetTotalVRAMUsageInKB() { return m_totalVRAMUsageInBytes / 1024; }

    static inline uint64 GetTotalVRAMUsageInMB() { return m_totalVRAMUsageInBytes / (1024 * 1024); }

    static inline void AddVRAMUsageInBytes(uint64 bytes)
    {
      m_totalVRAMUsageInBytes += bytes;
    }

    static inline void RemoveVRAMUsageInBytes(uint64 bytes)
    {
      uint64 old               = m_totalVRAMUsageInBytes; 

      if (m_totalVRAMUsageInBytes < bytes)
      {
        TK_ASSERT_ONCE(false);
      }

      m_totalVRAMUsageInBytes -= bytes;
    }

    static inline void ResetVRAMUsage() { m_totalVRAMUsageInBytes = 0; }

    // Draw Call
    ///////////////////////////////////////////////////////

    static inline void AddDrawCall() { ++m_drawCallCount; }

    // NOTE: This function should be called regularly
    static inline void ResetDrawCallCounter() { m_drawCallCount = 0; }

    static inline uint64 GetDrawCallCount() { return m_drawCallCount; }

   private:
    static uint64 m_totalVRAMUsageInBytes;
    static uint64 m_drawCallCount;
  };
} // namespace ToolKit
