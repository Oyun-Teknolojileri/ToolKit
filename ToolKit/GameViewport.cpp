/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "GameViewport.h"

#include "Events.h"

namespace ToolKit
{
  GameViewport::GameViewport() { m_contentAreaLocation = Vec2(0.0f); }

  GameViewport::GameViewport(float width, float height) : Viewport(width, height) {}

  void GameViewport::Update(float dt)
  {
    for (Event* e : Main::GetInstance()->m_eventPool)
    {
      int coord[2] = {0};
      if (e->m_type == Event::EventType::Mouse)
      {
        MouseEvent* me = static_cast<MouseEvent*>(e);
        coord[0]       = me->absolute[0];
        coord[1]       = me->absolute[1];
      }
      else if (e->m_type == Event::EventType::Touch)
      {
        TouchEvent* te = static_cast<TouchEvent*>(e);
        coord[0]       = glm::iround(te->absolute[0] * m_wndContentAreaSize.x);
        coord[1]       = glm::iround(te->absolute[1] * m_wndContentAreaSize.y);
      }

      m_lastMousePosRelContentArea.x = coord[0];
      m_lastMousePosRelContentArea.y = coord[1];
    }
  }
} // namespace ToolKit