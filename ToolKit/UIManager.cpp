

#include "UIManager.h"

#include "ToolKit.h"

#include <iterator>
#include <vector>

#include "DebugNew.h"

namespace ToolKit
{

  UILayer::UILayer() { m_id = GetHandleManager()->GetNextHandle(); }

  UILayer::UILayer(const String& file) : UILayer()
  {
    m_scene = GetSceneManager()->Create<Scene>(file);
    m_scene->Load();
  }

  UILayer::UILayer(ScenePtr scene) : UILayer()
  {
    m_scene = scene;
    m_scene->Load();
  }

  UILayer::~UILayer() {}

  void UILayer::Init() {}

  void UILayer::Uninit() {}

  void UILayer::Update(float deltaTime) {}

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
            canvasPanel->ApplyRecursiveResizePolicy(width, height);
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

      float t         = 0.0f;
      if (RayBoxIntersection(ray, box, t))
      {
        return true;
      }
    }
    return false;
  }

  Camera* UIManager::GetUICamera() { return m_uiCamera; }

  void UIManager::SetUICamera(Camera* cam)
  {
    SafeDel(m_uiCamera);
    m_uiCamera = cam;
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
          Surface* surface        = static_cast<Surface*>(ntt);
          bool mouseOverPrev      = surface->m_mouseOver;

          surface->m_mouseOver    = CheckMouseOver(surface, e, vp);
          surface->m_mouseClicked = CheckMouseClick(surface, e, vp);

          if (ntt->GetType() == EntityType::Entity_Button)
          {
            Button* button        = static_cast<Button*>(ntt);
            MaterialPtr hoverMat  = button->GetHoverMaterialVal();
            MaterialPtr normalMat = button->GetButtonMaterialVal();
            if (surface->m_mouseOver && hoverMat)
            {
              button->SetMaterialVal(hoverMat);
            }
            else
            {
              button->SetMaterialVal(normalMat);
            }
          }

          if (surface->m_mouseOver)
          {
            if (surface->m_onMouseOver)
            {
              surface->m_onMouseOver(e, ntt);
            }
          }

          if (surface->m_mouseClicked)
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

  UIManager::UIManager()
  {
    m_uiCamera = new Camera();
    m_uiCamera->SetLens(-100.0f, 100.0f, -100.0f, 100.0f, 0.5f, 1000.0f);
    m_uiCamera->m_orthographicScale = 1.0f;
  }

  UIManager::~UIManager() { SafeDel(m_uiCamera); }

  void UIManager::UpdateLayers(float deltaTime, Viewport* viewport)
  {
    // Make sure camera covers the whole viewport.
    Vec2 vpSize                     = viewport->m_wndContentAreaSize;
    m_uiCamera->m_orthographicScale = 1.0f;
    m_uiCamera->SetLens(vpSize.x * -0.5f,
                        vpSize.x * 0.5f,
                        vpSize.y * 0.5f,
                        vpSize.y * -0.5f,
                        -100.0f,
                        100.0f);

    // Swap viewport camera with ui camera.
    ULongID attachmentSwap = NULL_HANDLE;
    viewport->SwapCamera(&m_uiCamera, attachmentSwap);

    // Update viewports with ui camera.
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

    viewport->SwapCamera(&m_uiCamera, attachmentSwap);
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
