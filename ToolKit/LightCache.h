#pragma once

#include "Light.h"
#include "Types.h"

namespace ToolKit
{
  template <int T>
  class TK_API LightCache
  {
    friend class Renderer;

   public:
    LightCache() { Reset(); }

    ~LightCache() { Reset(); }

    // Returns the index of light in cache
    inline int Add(LightPtr light, bool rendererCache = false)
    {
      int index = m_leastFreqUsedLightIndex;

      if (rendererCache && m_lightCache[index] != nullptr)
      {
        m_lightCache[index]->m_lightCacheIndex = -1;
      }

      m_lightCache[index]       = light;
      m_lightVersion[index]     = light->m_lightCacheVersion;
      m_leastFreqUsedLightIndex = (index + 1) % T;

      UpdateVersion();

      return index;
    }

    inline void AddToIndex(LightPtr light, int index, bool rendererCache = false)
    {
      if (m_leastFreqUsedLightIndex == index)
      {
        m_leastFreqUsedLightIndex++;
      }

      if (rendererCache && m_lightCache[index] != nullptr)
      {
        m_lightCache[index]->m_lightCacheIndex = -1;
      }

      m_lightCache[index]   = light;
      m_lightVersion[index] = light->m_lightCacheVersion;

      UpdateVersion();
    }

    inline void Reset()
    {
      m_leastFreqUsedLightIndex = 0;
      for (int i = 0; i < T; ++i)
      {
        m_lightCache[i]   = nullptr;
        m_lightVersion[i] = 0;
      }
      m_cacheVersion++;
    }

    // returns the index when found, returns -1 when not found
    inline int Contains(const LightPtr& light)
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

    inline uint16_t GetVersion() const { return m_cacheVersion; }

    inline void UpdateVersion() { m_cacheVersion++; }

    inline LightPtr* GetLights() { return m_lightCache; }

    inline void UpdateLightSlotVersion(int index)
    {
      if (m_lightCache[index] != nullptr)
      {
        m_lightVersion[index] = m_lightCache[index]->m_lightCacheVersion;
      }
    }

    inline uint16 GetLightSlotVersion(int index)
    {
      if (m_lightCache[index] != nullptr)
      {
        return m_lightVersion[index];
      }
      return 0;
    }

   private:
    LightPtr m_lightCache[T];
    uint16 m_lightVersion[T];
    uint16 m_cacheVersion         = 1;
    // std::deque<int> m_leastFreqUsedLightIndices; // We do not have Remove() function because we don't need it yet. So
    // this array is needless. Instead we can simply use an index.
    int m_leastFreqUsedLightIndex = 0;
  };

} // namespace ToolKit