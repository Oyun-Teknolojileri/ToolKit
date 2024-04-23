/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "TKAssert.h"
#include "ToolKit.h"
#include "Types.h"

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

   public:
    float m_elapsedGpuRenderTime          = 0.0f;
    float m_elapsedCpuRenderTime          = 0.0f;
    uint m_lightCacheInvalidationPerFrame = 0;

   private:
    uint64 m_totalVRAMUsageInBytes = 0;
    uint64 m_drawCallCount         = 0;
    uint64 m_renderPassCount       = 0;
  };

  TK_API inline uint64 GetLightCacheInvalidationPerFrame()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      return tkStats->m_lightCacheInvalidationPerFrame;
    }
    else
    {
      return 0;
    }
  }

  TK_API inline uint64 GetTotalVRAMUsageInBytes()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      return tkStats->GetTotalVRAMUsageInBytes();
    }
    else
    {
      return 0;
    }
  }

  TK_API inline uint64 GetTotalVRAMUsageInKB()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      return tkStats->GetTotalVRAMUsageInKB();
    }
    else
    {
      return 0;
    }
  }

  TK_API inline uint64 GetTotalVRAMUsageInMB()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      return tkStats->GetTotalVRAMUsageInMB();
    }
    else
    {
      return 0;
    }
  }

  TK_API inline void AddVRAMUsageInBytes(uint64 bytes)
  {
    if (TKStats* tkStats = GetTKStats())
    {
      tkStats->AddVRAMUsageInBytes(bytes);
    }
  }

  TK_API inline void RemoveVRAMUsageInBytes(uint64 bytes)
  {
    if (TKStats* tkStats = GetTKStats())
    {
      tkStats->RemoveVRAMUsageInBytes(bytes);
    }
  }

  TK_API inline void ResetVRAMUsage()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      tkStats->ResetVRAMUsage();
    }
  }

  TK_API inline void AddDrawCall()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      tkStats->AddDrawCall();
    }
  }

  TK_API inline void ResetDrawCallCounter()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      tkStats->ResetDrawCallCounter();
    }
  }

  TK_API inline uint64 GetDrawCallCount()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      return tkStats->GetDrawCallCount();
    }
    else
    {
      return 0;
    }
  }

  TK_API inline void AddHWRenderPass()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      tkStats->AddHWRenderPass();
    }
  }

  TK_API inline void RemoveHWRenderPass()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      tkStats->RemoveHWRenderPass();
    }
  }

  TK_API inline void ResetLightCacheInvalidationPerFrame()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      tkStats->m_lightCacheInvalidationPerFrame = 0;
    }
  }

  TK_API inline void ResetHWRenderPassCounter()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      tkStats->ResetHWRenderPassCounter();
    }
  }

  TK_API inline uint64 GetHWRenderPassCount()
  {
    if (TKStats* tkStats = GetTKStats())
    {
      return tkStats->GetHWRenderPassCount();
    }
    else
    {
      return 0;
    }
  }

  TK_API inline void GetRenderTime(float& cpu, float& gpu)
  {
    if (TKStats* tkStats = GetTKStats())
    {
      cpu = tkStats->m_elapsedCpuRenderTime;
      gpu = tkStats->m_elapsedGpuRenderTime;
    }
    else
    {
      cpu = 1.0f;
      gpu = 1.0f;
    }
  }
} // namespace ToolKit
