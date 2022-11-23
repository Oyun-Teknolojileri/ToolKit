#include "Viewport.h"

#include "Camera.h"
#include "DirectionComponent.h"
#include "Node.h"
#include "Primative.h"
#include "Renderer.h"
#include "ToolKit.h"
#include "Util.h"

#include <algorithm>

#include "DebugNew.h"

namespace ToolKit
{

  Camera* ViewportBase::GetCamera() const
  {
    if (m_attachedCamera != NULL_HANDLE)
    {
      if (ScenePtr currScene = GetSceneManager()->GetCurrentScene())
      {
        if (Camera* cam =
                static_cast<Camera*>(currScene->GetEntity(m_attachedCamera)))
        {
          assert(cam->GetType() == EntityType::Entity_Camera);
          return cam;
        }
      }
    }

    return m_camera;
  }

  void ViewportBase::SetCamera(Camera* cam)
  {
    SafeDel(m_camera);
    m_camera         = cam;
    m_attachedCamera = cam->GetIdVal();
  }

  ViewportBase::ViewportBase()
  {
    m_camera     = new Camera();
    m_viewportId = GetHandleManager()->GetNextHandle();
  }

  ViewportBase::~ViewportBase()
  {
    SafeDel(m_camera);
  }

  Viewport::Viewport()
  {
  }

  Viewport::Viewport(float width, float height)
      : m_wndContentAreaSize(width, height)
  {
    GetCamera()->SetLens(glm::quarter_pi<float>(), width / height);
    ResetViewportImage(GetRenderTargetSettings());
  }

  Viewport::~Viewport()
  {
  }

  void Viewport::OnResizeContentArea(float width, float height)
  {
    m_wndContentAreaSize.x = width;
    m_wndContentAreaSize.y = height;
    ResetViewportImage(GetRenderTargetSettings());
  }

  void Viewport::AdjustZoom(float delta)
  {
    Camera* cam = GetCamera();
    cam->m_node->Translate(Vec3(0.0f, 0.0f, -delta),
                           TransformationSpace::TS_LOCAL);

    if (cam->IsOrtographic())
    {
      float zoom               = cam->m_orthographicScale - delta;
      cam->m_orthographicScale = glm::max(zoom, 0.01f);
    }
  }

  RenderTargetSettigs Viewport::GetRenderTargetSettings()
  {
    return RenderTargetSettigs();
  }

  void Viewport::ResetViewportImage(const RenderTargetSettigs& settings)
  {
    if (m_framebuffer == nullptr)
    {
      m_framebuffer = std::make_shared<Framebuffer>();
    }

    m_framebuffer->UnInit();
    m_framebuffer->Init({(uint) m_wndContentAreaSize.x,
                         (uint) m_wndContentAreaSize.y,
                         settings.Msaa,
                         false,
                         true});

    m_renderTarget = std::make_shared<RenderTarget>(
        (uint) m_wndContentAreaSize.x, (uint) m_wndContentAreaSize.y, settings);
    m_renderTarget->Init();

    if (m_renderTarget->m_initiated)
    {
      m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                   m_renderTarget);
    }
  }

  Ray Viewport::RayFromMousePosition()
  {
    Vec2 ssp = GetLastMousePosScreenSpace();
    return RayFromScreenSpacePoint(ssp);
  }

  Ray Viewport::RayFromScreenSpacePoint(const Vec2& pnt)
  {
    Vec2 mcInVs = TransformScreenToViewportSpace(pnt);

    Ray ray;
    ray.position = TransformViewportToWorldSpace(mcInVs);

    Camera* cam = GetCamera();
    if (cam->IsOrtographic())
    {
      ray.direction = cam->GetComponent<DirectionComponent>()->GetDirection();
    }
    else
    {
      ray.direction =
          glm::normalize(ray.position - cam->m_node->GetTranslation(
                                            TransformationSpace::TS_WORLD));
    }

    return ray;
  }

  Vec3 Viewport::GetLastMousePosWorldSpace()
  {
    return TransformViewportToWorldSpace(GetLastMousePosViewportSpace());
  }

  Vec2 Viewport::GetLastMousePosViewportSpace()
  {
    Vec2 screenPoint = m_lastMousePosRelContentArea;
    screenPoint.y    = m_wndContentAreaSize.y - screenPoint.y;

    return screenPoint;
  }

  Vec2 Viewport::GetLastMousePosScreenSpace()
  {
    Vec2 screenPoint = GetLastMousePosViewportSpace();
    screenPoint.y    = m_wndContentAreaSize.y - screenPoint.y;

    return m_contentAreaLocation + screenPoint;
  }

  Vec3 Viewport::TransformViewportToWorldSpace(const Vec2& pnt)
  {
    return ViewportSpaceToWorldSpace(pnt, GetCamera(), m_wndContentAreaSize);
  }

  Vec2 Viewport::TransformWorldSpaceToScreenSpace(const Vec3& pnt)
  {
    return WorldSpaceToScreenSpace(
        pnt, GetCamera(), m_contentAreaLocation, m_wndContentAreaSize);
  }

  Vec2 Viewport::TransformScreenToViewportSpace(const Vec2& pnt)
  {
    return ScreenSpaceToViewportSpace(pnt, m_contentAreaLocation, m_wndContentAreaSize);
  }

  bool Viewport::IsOrthographic()
  {
    if (Camera* cam = GetCamera())
    {
      return cam->IsOrtographic();
    }

    return false;
  }

  void Viewport::AttachCamera(ULongID camId)
  {
    m_attachedCamera = camId;
  }

  float Viewport::GetBillboardScale()
  {
    Camera* cam = GetCamera();
    if (cam->IsOrtographic())
    {
      return cam->m_orthographicScale;
    }

    return m_wndContentAreaSize.y;
  }

} // namespace ToolKit
