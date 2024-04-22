#pragma once

#include "Types.h"

#include <deque>

namespace ToolKit
{
  class Light;

  template <int T>
  class TK_API LightCache
  {
   public:
    LightCache() { Reset(); }

    ~LightCache() { Reset(); }

    inline void Add(LightPtr light)
    {
      int index                 = m_leastFreqUsedLightIndex;

      m_lightCache[index]       = light;
      m_leastFreqUsedLightIndex = (index + 1) % T;

      UpdateVersion();
    }

    void Reset()
    {
      m_leastFreqUsedLightIndex = 0;
      for (uint i = 0; i < T; ++i)
      {
        m_lightCache[i] = nullptr;
      }
      m_lightCacheVersion++;
    }

    // returns the index when found, returns -1 when not found
    int Contains(const LightPtr& light)
    {
      for (int i = 0; i < T; ++i)
      {
        if (m_lightCache[i] == light)
        {
          return i;
        }
      }

      return -1;
    }

    uint16_t GetVersion() const { return m_lightCacheVersion; }

    void UpdateVersion() { m_lightCacheVersion++; }

    LightPtr* GetLights() { return m_lightCache; }

   private:
    LightPtr m_lightCache[T];
    int m_leastFreqUsedLightIndex = 0;
    uint16_t m_lightCacheVersion  = 1;
  };

} // namespace ToolKit