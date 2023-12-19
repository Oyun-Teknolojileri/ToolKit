#pragma once

#include "TKAssert.h"

namespace ToolKit
{
  class TK_API TKStats
  {
   public:
    // Vram Usage
    ///////////////////////////////////////////////////////

    inline uint64 GetTotalVRAMUsageInBytes() { return m_totalVRAMUsageInBytes; }

    inline uint64 GetTotalVRAMUsageInKB() { return m_totalVRAMUsageInBytes / 1024; }

    inline uint64 GetTotalVRAMUsageInMB() { return m_totalVRAMUsageInBytes / (1024 * 1024); }

    inline void AddVRAMUsageInBytes(uint64 bytes) { m_totalVRAMUsageInBytes += bytes; }

    inline void RemoveVRAMUsageInBytes(uint64 bytes)
    {
      uint64 old = m_totalVRAMUsageInBytes;

      if (m_totalVRAMUsageInBytes < bytes)
      {
        TK_ASSERT_ONCE(false);
      }

      m_totalVRAMUsageInBytes -= bytes;
    }

    inline void ResetVRAMUsage() { m_totalVRAMUsageInBytes = 0; }

    // Draw Call
    ///////////////////////////////////////////////////////

    inline void AddDrawCall() { ++m_drawCallCount; }

    // NOTE: This function should be called regularly in a frame in order to get per-frame count
    inline void ResetDrawCallCounter() { m_drawCallCount = 0; }

    inline uint64 GetDrawCallCount() { return m_drawCallCount; }

    // Hardware Render Pass Counter
    ///////////////////////////////////////////////////////

    inline void AddHWRenderPass() { ++m_renderPassCount; }

    inline void RemoveHWRenderPass() { --m_renderPassCount; }

    // NOTE: This function should be called regularly in a frame in order to get per-frame count
    inline void ResetHWRenderPassCounter() { m_renderPassCount = 0; }

    inline uint64 GetHWRenderPassCount() { return m_renderPassCount; }

   private:
    uint64 m_totalVRAMUsageInBytes = 0;
    uint64 m_drawCallCount         = 0;
    uint64 m_renderPassCount       = 0;
  };
} // namespace ToolKit
