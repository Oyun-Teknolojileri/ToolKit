/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

#ifdef _WIN32 // Windows.
  #define TK_GAME_API __declspec(dllexport)
#else // Other OS.
  #define TK_GAME_API
#endif

namespace ToolKit
{

  enum class PluginType
  {
    Base,
    Game
  };

  class TK_API Plugin
  {
   public:
    Plugin() {}

    virtual ~Plugin() {}

    virtual PluginType GetType()          = 0;
    virtual void Init(class Main* master) = 0;
    virtual void Destroy()                = 0;
  };

  class TK_API GamePlugin : public Plugin
  {
   public:
    virtual void Frame(float deltaTime, class Viewport* viewport) = 0;

    PluginType GetType() { return PluginType::Game; }

   public:
    bool m_quit = false; // Set this flag true to stop gameplay.
  };

} // namespace ToolKit
