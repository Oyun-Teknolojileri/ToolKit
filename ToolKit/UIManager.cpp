
#include "ToolKit.h"
#include "UIManager.h"

#include <vector>
#include <iterator>


namespace ToolKit
{

  UILayer::UILayer()
  {
  }

  UILayer::~UILayer()
  {
  }

  Entity* UILayer::GetLayer(const String& layerName)
  {
    if (m_layout)
    {
      return m_layout->GetFirstEntityByName(layerName);
    }

    return nullptr;
  }

  void UILayer::Update(float deltaTime, Camera* cam, Viewport* vp)
  {
    m_cam = cam;
    m_viewport = vp;
    m_lastCamEntity = vp->GetCamera();
    vp->AttachCamera(cam->GetIdVal());

    // Add update camera (cam) to scene, AttachCamera - GetCamera
    // routines will search the in the scene.
    // Make sure this is the very first thing before accessing viewport
    // cameras.
    if (SceneManager* sceneMngr = GetSceneManager())
    {
      ScenePtr currScene = sceneMngr->GetCurrentScene();
      currScene->AddEntity(cam);
    }

    UpdateSurfaces(vp);
    vp->AttachCamera(m_lastCamEntity->GetIdVal());

    if (SceneManager* sceneMngr = GetSceneManager())
    {
      ScenePtr currScene = sceneMngr->GetCurrentScene();
      currScene->RemoveEntity(m_cam->GetIdVal());
    }
  }

  Entity* UILayer::FetchEntity(const String& entityName)
  {
    if (m_layout)
    {
      EntityRawPtrArray fetch = m_layout->Filter
      (
        [&entityName](Entity* e) -> bool
        {
          return e->GetNameVal() == entityName;
        }
      );

      if (!fetch.empty())
      {
        return fetch.front();
      }
    }
    return nullptr;
  }

  bool UILayer::CheckMouseClick(Surface* surface, Event* e, Viewport* vp)
  {
    if (CheckMouseOver(surface, e, vp))
    {
      MouseEvent* me = static_cast<MouseEvent*> (e);
      return me->m_action == EventAction::LeftClick;
    }

    return false;
  }

  bool UILayer::CheckMouseOver(Surface* surface, Event* e, Viewport* vp)
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

  void UILayer::UpdateSurfaces(Viewport* vp)
  {
    Entity* rootEntity = m_layout->GetFirstEntityByName(m_layerName);

    EntityRawPtrArray entities;
    GetChildren(rootEntity, entities);
    EventPool& events = Main::GetInstance()->m_eventPool;
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

  void UIManager::SetRootLayer(UILayer* newRootLayer)
  {
    m_rootLayer = newRootLayer;
  }

  void UIManager::AddChildLayer(UILayer* newChildLayer)
  {
    m_childLayers.push_back(newChildLayer);
  }

  void UIManager::RemoveChildLayer(String layerName)
  {
    UILayerPtrArray::iterator it;
    it = m_childLayers.begin();
    while (it != m_childLayers.end())
    {
      if ((*it)->m_layerName == layerName)
      {
        m_childLayers.erase(it);
        return;
      }
      else
      {
        it++;
      }
    }
  }

  void UIManager::RemoveAllChilds()
  {
    m_childLayers.clear();
  }

  /**
  * Returns current UILayer that UIManager has.
  */
  UILayerPtrArray UIManager::GetCurrentLayers()
  {
    UILayerPtrArray currentLayers;
    currentLayers.push_back(m_rootLayer);
    currentLayers.insert
    (
      currentLayers.end(),
      m_childLayers.begin(),
      m_childLayers.end()
    );
    return currentLayers;
  }

  void UIManager::UpdateLayers(float deltaTime, Viewport* vp)
  {
    if (m_rootLayer == nullptr)
    {
      return;
    }

    m_rootLayer->Update(deltaTime, m_rootLayer->m_cam, vp);

    if (m_childLayers.size() < 1)
    {
      return;
    }

    for (UILayer* layer : m_childLayers)
    {
      layer->Update(deltaTime, layer->m_cam, vp);
    }
  }

}  // namespace ToolKit
