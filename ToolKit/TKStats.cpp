/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "TKStats.h"

#include "TKAssert.h"
#include "TKOpenGL.h"
#include "ToolKit.h"

namespace ToolKit
{

  void TKStats::BeginTimer(StringView name)
  {
    TimeArgs& args = m_profileTimerMap[name.data()];
    args.beginTime = GetElapsedMilliSeconds();
  }

  // Finalize a timer updates statistics.
  void TKStats::EndTimer(StringView name)
  {
    TimeArgs& args        = m_profileTimerMap[name.data()];
    args.elapsedTime      = GetElapsedMilliSeconds() - args.beginTime;
    args.accumulatedTime += args.elapsedTime;
    args.hitCount++;
  }

  void TKStats::RemoveVRAMUsageInBytes(uint64 bytes)
  {
    uint64 old = m_totalVRAMUsageInBytes;

    TK_ASSERT_ONCE(m_totalVRAMUsageInBytes >= bytes);

    m_totalVRAMUsageInBytes -= bytes;
  }

  namespace Stats
  {
    TK_API void SetGpuResourceLabel(StringView label, GpuResourceType resourceType, uint resourceId)
    {
      if (glLabelObjectEXT != nullptr && label.size() > 0)
      {
        String labelId = String(label) + "_" + std::to_string(resourceId);
        glLabelObjectEXT((GLenum) resourceType, (GLuint) resourceId, 0, labelId.c_str());
      }
    }

    void BeginGpuScope(StringView name)
    {
      if (glPushGroupMarkerEXT != nullptr)
      {
        glPushGroupMarkerEXT(-1, name.data());
      }
    }

    void EndGpuScope()
    {
      if (glPopGroupMarkerEXT != nullptr)
      {
        glPopGroupMarkerEXT();
      }
    }

    void BeginTimeScope(StringView name)
    {
      if (TKStats* tkStats = GetTKStats())
      {
        tkStats->BeginTimer(name);
      }
    }

    void EndTimeScope(StringView name)
    {
      if (TKStats* tkStats = GetTKStats())
      {
        tkStats->EndTimer(name);
      }
    }

    uint64 GetLightCacheInvalidationPerFrame()
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

    uint64 GetTotalVRAMUsageInBytes()
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

    uint64 GetTotalVRAMUsageInKB()
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

    uint64 GetTotalVRAMUsageInMB()
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

    void AddVRAMUsageInBytes(uint64 bytes)
    {
      if (TKStats* tkStats = GetTKStats())
      {
        tkStats->AddVRAMUsageInBytes(bytes);
      }
    }

    void RemoveVRAMUsageInBytes(uint64 bytes)
    {
      if (TKStats* tkStats = GetTKStats())
      {
        tkStats->RemoveVRAMUsageInBytes(bytes);
      }
    }

    void ResetVRAMUsage()
    {
      if (TKStats* tkStats = GetTKStats())
      {
        tkStats->ResetVRAMUsage();
      }
    }

    void AddDrawCall()
    {
      if (TKStats* tkStats = GetTKStats())
      {
        tkStats->AddDrawCall();
      }
    }

    void ResetDrawCallCounter()
    {
      if (TKStats* tkStats = GetTKStats())
      {
        tkStats->ResetDrawCallCounter();
      }
    }

    uint64 GetDrawCallCount()
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

    void AddHWRenderPass()
    {
      if (TKStats* tkStats = GetTKStats())
      {
        tkStats->AddHWRenderPass();
      }
    }

    void RemoveHWRenderPass()
    {
      if (TKStats* tkStats = GetTKStats())
      {
        tkStats->RemoveHWRenderPass();
      }
    }

    void ResetLightCacheInvalidationPerFrame()
    {
      if (TKStats* tkStats = GetTKStats())
      {
        tkStats->m_lightCacheInvalidationPerFrame = 0;
      }
    }

    void ResetHWRenderPassCounter()
    {
      if (TKStats* tkStats = GetTKStats())
      {
        tkStats->ResetHWRenderPassCounter();
      }
    }

    uint64 GetHWRenderPassCount()
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

    void GetRenderTime(float& cpu, float& gpu)
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

    void GetRenderTimeAvg(float& cpu, float& gpu)
    {
      if (TKStats* tkStats = GetTKStats())
      {
        cpu = tkStats->m_elapsedCpuRenderTimeAvg;
        gpu = tkStats->m_elapsedGpuRenderTimeAvg;
      }
      else
      {
        cpu = 1.0f;
        gpu = 1.0f;
      }
    }

  } // namespace Stats

} // namespace ToolKit
