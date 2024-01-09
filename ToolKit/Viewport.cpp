/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Viewport.h"

#include "Camera.h"
#include "DirectionComponent.h"
#include "Node.h"
#include "Primative.h"
#include "Renderer.h"
#include "Scene.h"
#include "ToolKit.h"
#include "Util.h"
#include "Viewport.h"

#include "DebugNew.h"

namespace ToolKit
{

  CameraPtr ViewportBase::GetCamera() const
  {
    if (m_attachedCamera != NULL_HANDLE)
    {
      if (ScenePtr currScene = GetSceneManager()->GetCurrentScene())
      {
        if (EntityPtr camNtt = currScene->GetEntity(m_attachedCamera))
        {
          if (CameraPtr cam = std::static_pointer_cast<Camera>(camNtt))
          {
            assert(cam->IsA<Camera>());
            return cam;
          }
        }
      }
    }

    return m_camera;
  }

  void ViewportBase::SetCamera(CameraPtr cam)
  {
    m_camera         = cam;
    m_attachedCamera = NULL_HANDLE;
  }

  void ViewportBase::SwapCamera(CameraPtr& cam, ULongID& attachment)
  {
    cam.swap(m_camera);

    ULongID tmpHdnl  = m_attachedCamera;
    m_attachedCamera = attachment;
    attachment       = tmpHdnl;
  }

  void ViewportBase::AttachCamera(ULongID camId)
  {
    assert(camId == NULL_HANDLE || GetSceneManager()->GetCurrentScene()->GetEntity(camId) != nullptr &&
                                       "Given camera must be in the current scene.");

    m_attachedCamera = camId;
  }

  ViewportBase::ViewportBase()
  {
    m_camera         = MakeNewPtr<Camera>();
    m_viewportId     = GetHandleManager()->GenerateHandle();
    m_attachedCamera = NULL_HANDLE;
  }

  ViewportBase::~ViewportBase() {}

  Viewport::Viewport() {}

  Viewport::Viewport(float width, float height) : m_wndContentAreaSize(width, height)
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
    CameraPtr cam = GetCamera();
    cam->m_node->Translate(Vec3(0.0f, 0.0f, -delta), TransformationSpace::TS_LOCAL);

    if (cam->IsOrtographic())
    {
      float zoom               = cam->m_orthographicScale - delta;
      cam->m_orthographicScale = glm::max(zoom, 0.01f);
    }
  }

  TextureSettings Viewport::GetRenderTargetSettings() { return TextureSettings(); }

  void Viewport::ResetViewportImage(const TextureSettings& settings)
  {
    if (m_framebuffer == nullptr)
    {
      m_framebuffer = MakeNewPtr<Framebuffer>();
    }

    m_framebuffer->UnInit();
    m_framebuffer->Init({(uint) m_wndContentAreaSize.x, (uint) m_wndContentAreaSize.y, false, true});

    m_renderTarget = MakeNewPtr<RenderTarget>((uint) m_wndContentAreaSize.x, (uint) m_wndContentAreaSize.y, settings);
    m_renderTarget->Init();

    if (m_renderTarget->m_initiated)
    {
      m_framebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_renderTarget);
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
    ray.position  = TransformViewportToWorldSpace(mcInVs);

    CameraPtr cam = GetCamera();
    if (cam->IsOrtographic())
    {
      ray.direction = cam->GetComponent<DirectionComponent>()->GetDirection();
    }
    else
    {
      ray.direction = glm::normalize(ray.position - cam->m_node->GetTranslation(TransformationSpace::TS_WORLD));
    }

    return ray;
  }

  Vec3 Viewport::GetLastMousePosWorldSpace() { return TransformViewportToWorldSpace(GetLastMousePosViewportSpace()); }

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
    Vec3 pnt3d    = Vec3(pnt, 0.0f);

    CameraPtr cam = GetCamera();
    Mat4 view     = cam->GetViewMatrix();
    Mat4 project  = cam->GetProjectionMatrix();

    return glm::unProject(pnt3d, view, project, Vec4(0.0f, 0.0f, m_wndContentAreaSize.x, m_wndContentAreaSize.y));
  }

  Vec2 Viewport::TransformWorldSpaceToScreenSpace(const Vec3& pnt)
  {
    CameraPtr cam  = GetCamera();
    Mat4 view      = cam->GetViewMatrix();
    Mat4 project   = cam->GetProjectionMatrix();

    Vec3 screenPos = glm::project(pnt, view, project, Vec4(0.0f, 0.0f, m_wndContentAreaSize.x, m_wndContentAreaSize.y));

    screenPos.x += m_contentAreaLocation.x;
    screenPos.y  = m_wndContentAreaSize.y + m_contentAreaLocation.y - screenPos.y;

    return screenPos;
  }

  Vec2 Viewport::TransformScreenToViewportSpace(const Vec2& pnt)
  {
    Vec2 vp = pnt - m_contentAreaLocation;   // In window space.
    vp.y    = m_wndContentAreaSize.y - vp.y; // In viewport space.
    return vp;
  }

  bool Viewport::IsOrthographic()
  {
    if (CameraPtr cam = GetCamera())
    {
      return cam->IsOrtographic();
    }

    return false;
  }

  float Viewport::GetBillboardScale()
  {
    CameraPtr cam = GetCamera();
    if (cam->IsOrtographic())
    {
      return cam->m_orthographicScale;
    }

    return m_wndContentAreaSize.y;
  }

} // namespace ToolKit
