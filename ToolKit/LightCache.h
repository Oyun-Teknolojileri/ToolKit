#pragma once

#include "TKStats.h"
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

    inline void SetDrawCallVersion(uint16 version) { m_drawCallVersion = version; }

    inline int Add(LightPtr light)
    {
      int index = m_nextIndex;

      // While adding new light, if the light that is going to be discarded is going to be
      // rendered on the current draw call (has the same draw call version with this value), skip that light.
      int i     = 0;
      while (true)
      {
        i++;
        if (i == T)
        {
          return -1;
        }

        if (m_lightCache[index] == nullptr)
        {
          m_lightCache[index] = light;
          m_nextIndex         = (index + 1) % T;
          break;
        }
        else if (m_lightCache[index]->m_drawCallVersion != m_drawCallVersion)
        {
          m_lightCache[index] = light;
          m_nextIndex         = (index + 1) % T;
          break;
        }
        index = (index + 1) % T;
      }

      UpdateVersion();

      return index;
    }

    void Reset()
    {
      m_nextIndex = 0;
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
    LightPtr m_lightCache[T];
    int m_nextIndex            = 0;
    uint16 m_lightCacheVersion = 1;

    uint16 m_drawCallVersion   = 0; //<! Renderer sets this value before each draw call
  };

} // namespace ToolKit