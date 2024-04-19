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

    ~LightCache() { m_leastFreqUsedLightIndices.clear(); }

    inline void Add(Light* light)
    {
      uint lfuIndex = m_leastFreqUsedLightIndices.front();
      m_leastFreqUsedLightIndices.pop_front();

      m_lightCache[lfuIndex] = light;
      m_leastFreqUsedLightIndices.push_back(lfuIndex);

      UpdateVersion();
    }

    void Reset()
    {
      m_leastFreqUsedLightIndices.clear();
      for (uint i = 0; i < T; ++i)
      {
        m_leastFreqUsedLightIndices.push_back(i);
        m_lightCache[i] = nullptr;
      }
      m_lightCacheVersion++;
    }

    // returns the index when found, returns -1 when not found
    int Contains(const Light* light)
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

    Light** GetLights() { return m_lightCache; }

   private:
    Light* m_lightCache[T];
    std::deque<int> m_leastFreqUsedLightIndices;
    uint16_t m_lightCacheVersion = 1;
  };

} // namespace ToolKit