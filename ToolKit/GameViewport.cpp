#include "GameViewport.h"
#include "Events.h"

namespace ToolKit
{
  GameViewport::GameViewport() { m_contentAreaLocation = Vec2(0.0f); }

  GameViewport::GameViewport(float width, float height) : Viewport(width, height) {}

  void GameViewport::Update(float dt)
  {
    m_mouseOverContentArea = true;

    for (Event* e : Main::GetInstance()->m_eventPool)
    {
      if (e->m_type == Event::EventType::Mouse)
      {
        MouseEvent* me                 = static_cast<MouseEvent*>(e);
        m_lastMousePosRelContentArea.x = me->absolute[0];
        m_lastMousePosRelContentArea.y = me->absolute[1];
      }
    }
  }
} // namespace ToolKit