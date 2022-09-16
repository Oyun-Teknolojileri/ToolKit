
#include "UIManager.h"

#include "ToolKit.h"

#include <iterator>
#include <vector>

namespace ToolKit
{

  UILayer::UILayer(ScenePtr scene)
  {
    m_scene = scene;
    m_id    = GetHandleManager()->GetNextHandle();
  }

  UILayer::~UILayer()
  {
  }

  void UILayer::Init()
  {
  }

  void UILayer::Uninit()
  {
  }

  void UILayer::Update(float deltaTime)
  {
  }

  void UILayer::ResizeUI(float width, float height)
  {
    if (m_scene == nullptr)
    {
      return;
    }

    const EntityRawPtrArray& arr = m_scene->GetEntities();
    for (Entity* ntt : arr)
    {
      if (ntt->GetType() == EntityType::Entity_Canvas)
      {
        Canvas* canvasPanel = static_cast<Canvas*>(ntt);

        // Apply sizing only when the resolution has changed.
        Vec2 size(width, height);
        if (!VecAllEqual<Vec2>(m_size, size))
        {
          m_size = size;

          // Resize only root canvases.
          if (canvasPanel->m_node->m_parent == nullptr)
          {
            canvasPanel->ApplyRecursivResizePolicy(width, height);
          }
        }
      }
    }
  }

  bool UIManager::CheckMouseClick(Surface* surface, Event* e, Viewport* vp)
  {
    if (CheckMouseOver(surface, e, vp))
    {
      MouseEvent* me = static_cast<MouseEvent*>(e);
      return me->m_action == EventAction::LeftClick;
    }

    return false;
  }

  bool UIManager::CheckMouseOver(Surface* surface, Event* e, Viewport* vp)
  {
    if (e->m_type == Event::EventType::Mouse)
    {
      BoundingBox box = surface->GetAABB(true);
      Ray ray         = vp->RayFromMousePosition();

      float t = 0.0f;
      if (RayBoxIntersection(ray, box, t))
      {
        return true;
      }
    }
    return false;
  }

  void UIManager::UpdateSurfaces(Viewport* vp, UILayer* layer)
  {
    EventPool& events = Main::GetInstance()->m_eventPool;
    if (events.empty())
    {
      return;
    }

    EntityRawPtrArray& entities = layer->m_scene->AccessEntityArray();
    for (Entity* ntt : entities)
    {
      // Process events.
      for (Event* e : events)
      {
        if (ntt->IsSurfaceInstance())
        {
          Surface* surface   = static_cast<Surface*>(ntt);
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

  void UIManager::UpdateLayers(float deltaTime, Viewport* viewport)
  {
    for (auto& viewLayerArray : m_viewportIdLayerArrayMap)
    {
      if (viewLayerArray.first == viewport->m_viewportId)
      {
        for (UILayer* layer : viewLayerArray.second)
        {
          // Check potential events than updates.
          UpdateSurfaces(viewport, layer);
          layer->Update(deltaTime);
        }
      }
    }
  }

  void UIManager::GetLayers(ULongID viewportId, UILayerRawPtrArray& layers)
  {
    auto res = m_viewportIdLayerArrayMap.find(viewportId);
    if (res != m_viewportIdLayerArrayMap.end())
    {
      layers = res->second;
    }
  }

  void UIManager::AddLayer(ULongID viewportId, UILayer* layer)
  {
    if (Exist(viewportId, layer->m_id) == -1)
    {
      m_viewportIdLayerArrayMap[viewportId].push_back(layer);
    }
  }

  UILayer* UIManager::RemoveLayer(ULongID viewportId, ULongID layerId)
  {
    UILayer* layer = nullptr;
    int indx       = Exist(viewportId, layerId);
    if (indx != -1)
    {
      UILayerRawPtrArray& layers = m_viewportIdLayerArrayMap[viewportId];
      layer                      = layers[indx];
      layers.erase(layers.begin() + indx);
    }

    return layer;
  }

  int UIManager::Exist(ULongID viewportId, ULongID layerId)
  {
    auto vlArray = m_viewportIdLayerArrayMap.find(viewportId);
    if (vlArray == m_viewportIdLayerArrayMap.end())
    {
      return -1;
    }

    UILayerRawPtrArray& layers = vlArray->second;
    for (size_t i = 0; i < layers.size(); i++)
    {
      if (layers[i]->m_id == layerId)
      {
        return (int) i;
      }
    }

    return -1;
  }

  void UIManager::DestroyLayers()
  {
    for (auto vpLayerArray : m_viewportIdLayerArrayMap)
    {
      for (UILayer* layer : vpLayerArray.second)
      {
        layer->Uninit();
        SafeDel(layer);
      }
    }
  }

} // namespace ToolKit
