/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include <Plugin.h>
#include <ToolKit.h>

namespace ToolKit
{

  class Game : public GamePlugin
  {
   public:
    void Init(Main* master) override;
    void Destroy() override;
    void Frame(float deltaTime) override;
    void OnLoad(XmlDocumentPtr state) override;
    void OnUnload(XmlDocumentPtr state) override;
  };

  extern Game* g_game;

} // namespace ToolKit

extern "C" TK_PLUGIN_API ToolKit::Game* TK_STDCAL GetInstance();