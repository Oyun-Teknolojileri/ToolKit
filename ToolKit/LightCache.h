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
    LightCache();
    ~LightCache();
    void Add(Light* light);
    void Reset();

    // returns the index when found, returns -1 when not found
    int Contains(const Light* light);

    uint16_t GetVersion() const;
    void UpdateVersion();

    Light** GetLights();

   private:
    Light* m_lightCache[T];
    std::deque<int> m_leastFreqUsedLightIndices;
    uint16_t m_lightCacheVersion = 1;
  };

} // namespace ToolKit