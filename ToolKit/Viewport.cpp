#include "Viewport.h"

#include <algorithm>

#include "Camera.h"
#include "DirectionComponent.h"
#include "Renderer.h"
#include "Node.h"
#include "Primative.h"
#include "Util.h"
#include "ToolKit.h"
#include "DebugNew.h"

namespace ToolKit
{

  Camera* ViewportBase::GetCamera() const
  {
    if (m_attachedCamera != NULL_HANDLE)
    {
      if (ScenePtr currScene = GetSceneManager()->GetCurrentScene())
      {
        if
        (
          Camera* cam = static_cast<Camera*>
          (
            currScene->GetEntity(m_attachedCamera)
          )
        )
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
    m_camera = cam;
    m_attachedCamera = cam->GetIdVal();
  }

  ViewportBase::ViewportBase()
  {
    m_camera = new Camera();
  }

  ViewportBase::~ViewportBase()
  {
    SafeDel(m_camera);
  }

  Viewport::Viewport()
  {
  }

  Viewport::Viewport(float width, float height)
    : m_width(width), m_height(height)
  {
    GetCamera()->SetLens(glm::quarter_pi<float>(), width, height);
    ResetViewportImage(GetRenderTargetSettings());
  }

  Viewport::~Viewport()
  {
    SafeDel(m_viewportImage);
  }

  void Viewport::OnResize(float width, float height)
  {
    m_width = width;
    m_height = height;

    UpdateCameraLens(m_width, m_height);
    ResetViewportImage(GetRenderTargetSettings());
  }

  void Viewport::AdjustZoom(float delta)
  {
    m_zoom -= delta;
    m_zoom = glm::max(m_zoom, 0.01f);

    Camera* cam = GetCamera();
    cam->m_node->Translate
    (
      Vec3(0.0f, 0.0f, -delta),
      TransformationSpace::TS_LOCAL
    );
    if (cam->IsOrtographic())
    {
      cam->SetLens
      (
        -m_zoom * m_width * 0.5f,
        m_zoom * m_width * 0.5f,
        -m_zoom * m_height * 0.5f,
        m_zoom * m_height * 0.5f,
        0.5f,
        1000.0f
      );
    }
  }

  void Viewport::UpdateCameraLens(float width, float height)
  {
    Camera* cam = GetCamera();
    if (cam->IsOrtographic())
    {
      cam->SetLens(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 1000.0f);
    }
    else
    {
      cam->SetLens(cam->GetData().fov, width, height);
    }
  }

  RenderTargetSettigs Viewport::GetRenderTargetSettings()
  {
    return RenderTargetSettigs();
  }

  void Viewport::ResetViewportImage(const RenderTargetSettigs& settings)
  {
    if (m_viewportImage)
    {
      m_viewportImage->Reconstrcut((uint)m_width, (uint)m_height, settings);
    }
    else
    {
      m_viewportImage = new RenderTarget
      (
        (uint)m_width,
        (uint)m_height,
        settings
      );
      m_viewportImage->Init();
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
      ray.direction = glm::normalize
      (
        ray.position - cam->m_node->GetTranslation
        (
          TransformationSpace::TS_WORLD
        )
      );
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
    screenPoint.y = m_wndContentAreaSize.y - screenPoint.y;

    return screenPoint;
  }

  Vec2 Viewport::GetLastMousePosScreenSpace()
  {
    Vec2 screenPoint = GetLastMousePosViewportSpace();
    screenPoint.y = m_wndContentAreaSize.y - screenPoint.y;

    return m_wndPos + screenPoint;
  }

  Vec3 Viewport::TransformViewportToWorldSpace(const Vec2& pnt)
  {
    Vec3 screenPoint = Vec3(pnt, 0.0f);

    Camera* cam = GetCamera();
    Mat4 view = cam->GetViewMatrix();
    Mat4 project = cam->GetProjectionMatrix();

    return glm::unProject
    (
      screenPoint,
      view,
      project,
      Vec4(0.0f, 0.0f, m_width, m_height)
    );
  }

  Vec2 Viewport::TransformWorldSpaceToScreenSpace(const Vec3& pnt)
  {
    Camera* cam = GetCamera();
    glm::mat4 view = cam->GetViewMatrix();
    glm::mat4 project = cam->GetData().projection;
    Vec3 screenPos = glm::project(
      pnt,
      view,
      project,
      glm::vec4(0.0f, 0.0f, m_width, m_height)
    );
    screenPos.x += m_wndPos.x;
    screenPos.y = m_wndContentAreaSize.y + m_wndPos.y - screenPos.y;
    return screenPos.xy;
  }

  Vec2 Viewport::TransformScreenToViewportSpace(const Vec2& pnt)
  {
    Vec2 vp = pnt - m_wndPos;  // In window space.
    vp.y = m_wndContentAreaSize.y - vp.y;  // In viewport space.
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

  void Viewport::AttachCamera(ULongID camId)
  {
    m_attachedCamera = camId;
    UpdateCameraLens(m_width, m_height);
  }

}  // namespace ToolKit
