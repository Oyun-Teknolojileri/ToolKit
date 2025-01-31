/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Viewport.h"

namespace ToolKit
{

  class TK_API GameViewport : public Viewport
  {
   public:
    GameViewport();
    GameViewport(float width, float height);

    void Update(float dt) override;
  };

  typedef std::shared_ptr<GameViewport> GameViewportPtr;

} // namespace ToolKit
