#include "LightCache.h"

namespace ToolKit
{
  template <int T>
  LightCache<T>::LightCache()
  {
    Reset();
  }

  template <int T>
  LightCache<T>::~LightCache()
  {
    m_leastFreqUsedLightIndices.clear();
  }

  template <int T>
  inline void ToolKit::LightCache<T>::Add(Light* light)
  {
    uint lfuIndex = m_leastFreqUsedLightIndices.front();
    m_leastFreqUsedLightIndices.pop_front();

    m_lightCache[lfuIndex] = light;
    m_leastFreqUsedLightIndices.push_back(lfuIndex);

    UpdateVersion();
  }

  template <int T>
  void ToolKit::LightCache<T>::Reset()
  {
    m_leastFreqUsedLightIndices.clear();
    for (uint i = 0; i < T; ++i)
    {
      m_leastFreqUsedLightIndices.push_back(i);
      m_lightCache[i] = nullptr;
    }
    m_lightCacheVersion++;
  }

  template <int T>
  int ToolKit::LightCache<T>::Contains(const Light* light)
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

  template <int T>
  uint16_t LightCache<T>::GetVersion() const
  {
    return m_lightCacheVersion;
  }

  template <int T>
  void LightCache<T>::UpdateVersion()
  {
    m_lightCacheVersion++;
  }

  template <int T>
  Light** LightCache<T>::GetLights()
  {
    return m_lightCache;
  }
} // namespace ToolKit
