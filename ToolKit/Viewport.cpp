#include "Viewport.h"

#include "Camera.h"
#include "DirectionComponent.h"
#include "Node.h"
#include "Primative.h"
#include "Renderer.h"
#include "ToolKit.h"
#include "Util.h"
#include "Viewport.h"

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
    if (m_camera != nullptr)
    {
      // delete the camera and remove it from the scene
      GetSceneManager()->GetCurrentScene()->RemoveEntity(m_camera->GetIdVal());
      SafeDel(m_camera);
    }
    m_camera         = cam;
    m_attachedCamera = NULL_HANDLE;
  }

  ViewportBase::ViewportBase()
  {
    m_camera         = new Camera();
    m_viewportId     = GetHandleManager()->GetNextHandle();
    m_attachedCamera = NULL_HANDLE;
  }

  ViewportBase::~ViewportBase() { SafeDel(m_camera); }

  Viewport::Viewport() {}

  Viewport::Viewport(float width, float height)
      : m_wndContentAreaSize(width, height)
  {
    GetCamera()->SetLens(glm::quarter_pi<float>(), width / height);
    ResetViewportImage(GetRenderTargetSettings());
  }

  Viewport::~Viewport() {}

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
                         false,
                         true});

    m_renderTarget =
        std::make_shared<RenderTarget>((uint) m_wndContentAreaSize.x,
                                       (uint) m_wndContentAreaSize.y,
                                       settings);
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

    Camera* cam  = GetCamera();
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
    Vec3 screenPoint = Vec3(pnt, 0.0f);

    Camera* cam      = GetCamera();
    Mat4 view        = cam->GetViewMatrix();
    Mat4 project     = cam->GetProjectionMatrix();

    return glm::unProject(
        screenPoint,
        view,
        project,
        Vec4(0.0f, 0.0f, m_wndContentAreaSize.x, m_wndContentAreaSize.y));
  }

  Vec2 Viewport::TransformWorldSpaceToScreenSpace(const Vec3& pnt)
  {
    Camera* cam       = GetCamera();
    glm::mat4 view    = cam->GetViewMatrix();
    glm::mat4 project = cam->GetProjectionMatrix();

    Vec3 screenPos    = glm::project(
        pnt,
        view,
        project,
        Vec4(0.0f, 0.0f, m_wndContentAreaSize.x, m_wndContentAreaSize.y));

    screenPos.x += m_contentAreaLocation.x;
    screenPos.y =
        m_wndContentAreaSize.y + m_contentAreaLocation.y - screenPos.y;

    return screenPos.xy;
  }

  Vec2 Viewport::TransformScreenToViewportSpace(const Vec2& pnt)
  {
    Vec2 vp = pnt - m_contentAreaLocation;   // In window space.
    vp.y    = m_wndContentAreaSize.y - vp.y; // In viewport space.
    return vp;
  }

  bool Viewport::IsOrthographic()
  {
    if (Camera* cam = GetCamera())
    {
      return cam->IsOrtographic();
    }

    return false;
  }

  void Viewport::AttachCamera(ULongID camId) { m_attachedCamera = camId; }

  void Viewport::DetachCamera() { m_attachedCamera = NULL_HANDLE; }

  ULongID Viewport::GetAttachedCamera() { return m_attachedCamera; }

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
