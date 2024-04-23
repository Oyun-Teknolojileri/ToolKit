#pragma once

#include "TKStats.h"
#include "Types.h"

namespace ToolKit
{
  class Light;

  template <int CACHE_SIZE>
  class TK_API LightCache
  {
   public:
    LightCache() { Reset(); }

    ~LightCache() { Reset(); }

    inline void SetDrawCallVersion(uint16 version) { m_drawCallVersion = version; }

    inline int Add(LightPtr light)
    {
      int index = m_nextIndex;

      // If the light intended for removal is scheduled to be rendered in the current
      // draw call (possesses the same draw call version as this value), it should be omitted.
      int i     = 0;
      while (true)
      {
        i++;
        if (i == CACHE_SIZE)
        {
          return -1;
        }

        if (m_lightCache[index] == nullptr)
        {
          m_lightCache[index] = light;
          m_nextIndex         = (index + 1) % CACHE_SIZE;
          break;
        }
        else if (m_lightCache[index]->m_drawCallVersion != m_drawCallVersion)
        {
          m_lightCache[index] = light;
          m_nextIndex         = (index + 1) % CACHE_SIZE;
          break;
        }
        index = (index + 1) % CACHE_SIZE;
      }

      UpdateVersion();

      return index;
    }

    void Reset()
    {
      m_nextIndex = 0;
      for (uint i = 0; i < CACHE_SIZE; ++i)
      {
        m_lightCache[i] = nullptr;
      }
      m_lightCacheVersion++;
    }

    // returns the index when found, returns -1 when not found
    int Contains(const LightPtr& light)
    {
      for (int i = 0; i < CACHE_SIZE; ++i)
      {
        if (m_lightCache[i] == light)
        {
          return i;
        }
      }

      return -1;
    }

    uint16_t GetVersion() const { return m_lightCacheVersion; }

    void UpdateVersion()
    {
      if (TKStats* stats = GetTKStats())
      {
        stats->m_lightCacheInvalidationPerFrame++;
      }
      m_lightCacheVersion++;
    }

    LightPtr* GetLights() { return m_lightCache; }

   private:
    LightPtr m_lightCache[CACHE_SIZE];
    int m_nextIndex            = 0;
    uint16 m_lightCacheVersion = 1;

    uint16 m_drawCallVersion   = 0; //<! Renderer sets this value before each draw call
  };

} // namespace ToolKit