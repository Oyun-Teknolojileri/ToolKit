/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "UIManager.h"

#include "Canvas.h"
#include "MathUtil.h"
#include "Scene.h"
#include "ToolKit.h"

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

  void UILayer::ResizeUI(const Vec2& size)
  {
    // Sanity checks.
    if (m_scene == nullptr)
    {
      return;
    }

    // Apply sizing only when the resolution has changed.
    if (VecAllEqual<Vec2>(m_size, size))
    {
      return;
    }

    m_size                    = size;

    const EntityPtrArray& arr = m_scene->GetEntities();
    for (EntityPtr ntt : arr)
    {
      if (Canvas* canvasPanel = ntt->As<Canvas>())
      {
        // Resize only root canvases.
        if (canvasPanel->m_node->m_parent == nullptr)
        {
          canvasPanel->ApplyRecursiveResizePolicy(m_size.x, m_size.y);
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

  CameraPtr UIManager::GetUICamera() { return m_uiCamera; }

  void UIManager::SetUICamera(CameraPtr cam) { m_uiCamera = cam; }

  void UIManager::UpdateSurfaces(Viewport* vp, const UILayerPtr& layer)
  {
    EventPool& events = Main::GetInstance()->m_eventPool;
    if (events.empty())
    {
      return;
    }

    const EntityPtrArray& entities = layer->m_scene->AccessEntityArray();
    for (EntityPtr ntt : entities)
    {
      // Process events.
      for (Event* e : events)
      {
        if (!ntt->IsA<Surface>())
        {
          continue;
        }
        Surface* surface        = ntt->As<Surface>();
        bool mouseOverPrev      = surface->m_mouseOver;

        surface->m_mouseOver    = CheckMouseOver(surface, e, vp);
        surface->m_mouseClicked = CheckMouseClick(surface, e, vp);

        if (ntt->IsA<Button>())
        {
          Button* button        = ntt->As<Button>();
          MaterialPtr hoverMat  = button->GetHoverMaterialVal();
          MaterialPtr normalMat = button->GetButtonMaterialVal();
          button->SetMaterialVal(surface->m_mouseOver && hoverMat ? hoverMat : normalMat);
        }

        if (surface->m_mouseOver && surface->m_onMouseOver)
        {
          surface->m_onMouseOver(e, ntt);
        }

        if (surface->m_mouseClicked && surface->m_onMouseClick)
        {
          surface->m_onMouseClick(e, ntt);
        }

        if (!mouseOverPrev && surface->m_mouseOver && surface->m_onMouseEnter)
        {
          surface->m_onMouseEnter(e, ntt);
        }

        if (mouseOverPrev && !surface->m_mouseOver && surface->m_onMouseExit)
        {
          surface->m_onMouseExit(e, ntt);
        }
      }
    }
  }

  UIManager::UIManager()
  {
    m_uiCamera = MakeNewPtr<Camera>();
    m_uiCamera->SetLens(-100.0f, 100.0f, -100.0f, 100.0f, 0.5f, 1000.0f);
    m_uiCamera->m_orthographicScale = 1.0f;
  }

  UIManager::~UIManager() {}

  void UIManager::UpdateLayers(float deltaTime, Viewport* viewport)
  {
    GetUIManager()->ResizeLayers(viewport);

    // Swap viewport camera with ui camera.
    ULongID attachmentSwap = NULL_HANDLE;
    viewport->SwapCamera(m_uiCamera, attachmentSwap);

    // Update viewports with ui camera.
    for (auto& viewLayerArray : m_viewportIdLayerArrayMap)
    {
      if (viewLayerArray.first == viewport->m_viewportId)
      {
        for (const UILayerPtr& layer : viewLayerArray.second)
        {
          // Check potential events than updates.
          UpdateSurfaces(viewport, layer);
          layer->Update(deltaTime);
        }
      }
    }

    viewport->SwapCamera(m_uiCamera, attachmentSwap);
  }

  void UIManager::ResizeLayers(Viewport* viewport)
  {
    // Make sure camera covers the whole viewport.
    Vec2 vpSize                     = viewport->m_wndContentAreaSize;
    m_uiCamera->m_orthographicScale = 1.0f;
    m_uiCamera->SetLens(vpSize.x * -0.5f, vpSize.x * 0.5f, vpSize.y * -0.5f, vpSize.y * 0.5f, -100.0f, 100.0f);

    // Adjust camera location to match lower left corners.
    m_uiCamera->m_node->SetTranslation(Vec3(vpSize.x * 0.5f, vpSize.y * 0.5f, 1.0f));

    // Update viewports with ui camera.
    for (auto& viewLayerArray : m_viewportIdLayerArrayMap)
    {
      if (viewLayerArray.first == viewport->m_viewportId)
      {
        for (UILayerPtr layer : viewLayerArray.second)
        {
          // Check potential events than updates.
          layer->ResizeUI(vpSize);
        }
      }
    }
  }

  void UIManager::GetLayers(ULongID viewportId, UILayerPtrArray& layers)
  {
    auto res = m_viewportIdLayerArrayMap.find(viewportId);
    if (res != m_viewportIdLayerArrayMap.end())
    {
      layers = res->second;
    }
  }

  void UIManager::AddLayer(ULongID viewportId, const UILayerPtr& layer)
  {
    if (Exist(viewportId, layer->m_id) == -1)
    {
      m_viewportIdLayerArrayMap[viewportId].push_back(layer);
    }
  }

  UILayerPtr UIManager::RemoveLayer(ULongID viewportId, ULongID layerId)
  {
    UILayerPtr layer = nullptr;
    int indx         = Exist(viewportId, layerId);
    if (indx != -1)
    {
      UILayerPtrArray& layers = m_viewportIdLayerArrayMap[viewportId];
      layer                   = layers[indx];
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

    UILayerPtrArray& layers = vlArray->second;
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
      for (UILayerPtr layer : vpLayerArray.second)
      {
        layer->Uninit();
      }
    }

    m_viewportIdLayerArrayMap.clear();
  }

} // namespace ToolKit
