/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

namespace ToolKit
{
  namespace Editor
  {
    enum class PublishPlatform
    {
      Web,
      Windows,
      Linux,
      Android
    };

    class PublishManager
    {
     public:
      void Publish(PublishPlatform platform);

     public:
      TexturePtr m_icon = nullptr;
      String m_appName {};
      bool m_deployAfterBuild = false;
      bool m_isDebugBuild     = false;
      int m_minSdk            = 27;
      int m_maxSdk            = 32;

      enum Oriantation
      {
        Undefined,
        Landscape,
        Portrait
      };

      Oriantation m_oriantation;
      bool m_isBuilding = false;
    };

  } // namespace Editor
} // namespace ToolKit
