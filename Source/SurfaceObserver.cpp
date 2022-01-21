#include "stdafx.h"
#include "SurfaceObserver.h"
#include "ToolKit.h"
#include "Util.h"
#include "Viewport.h"

namespace ToolKit
{

  void SurfaceObserver::SetRoot(Entity* root)
  {
    m_root = root;
  }

  void SurfaceObserver::Update(float deltaTimeSec, Viewport* vp)
  {
    EntityRawPtrArray entities;
    GetChildren(m_root, entities);

    const EventPool& events = Main::GetInstance()->m_eventPool;
    if (entities.empty() || events.empty())
    {
      return;
    }

    for (Entity* ntt : entities)
    {
      // Process events.
      for (Event* e : events)
      {
        if (ntt->IsSurfaceInstance())
        {
          Surface* surface = static_cast<Surface*> (ntt);
          bool mouseOverPrev = surface->m_mouseOver;

          if (surface->m_mouseOver = CheckMouseOver(surface, e, vp))
          {
            if (surface->m_onMouseOver)
            {
              surface->m_onMouseOver(e, ntt);
            }
          }

          if (surface->m_mouseClicked = CheckMouseClick(surface, e, vp))
          {
            if (surface->m_onMouseClick)
            {
              surface->m_onMouseClick(e, ntt);
            }
          }

          if (!mouseOverPrev && surface->m_mouseOver)
          {
            if (surface->m_onMouseEnter)
            {
              surface->m_onMouseEnter(e, ntt);
            }
          }

          if (mouseOverPrev && !surface->m_mouseOver)
          {
            if (surface->m_onMouseExit)
            {
              surface->m_onMouseExit(e, ntt);
            }
          }
        }
      }
    }
  }

  bool SurfaceObserver::CheckMouseClick(Surface* surface, Event* e, Viewport* vp)
  {
    if (CheckMouseOver(surface, e, vp))
    {
      MouseEvent* me = static_cast<MouseEvent*> (e);
      return me->m_action == EventAction::LeftClick;
    }

    return false;
  }

  bool SurfaceObserver::CheckMouseOver(Surface* surface, Event* e, Viewport* vp)
  {
    if (e->m_type == Event::EventType::Mouse)
    {
      BoundingBox box = surface->GetAABB(true);
      Ray ray = vp->RayFromMousePosition();

      float t = 0.0f;
      if (RayBoxIntersection(ray, box, t))
      {
        return true;
      }
    }

    return false;
  }

}
