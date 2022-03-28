#include "stdafx.h"

#include "Viewport.h"
#include "Directional.h"
#include "Renderer.h"
#include "Node.h"
#include "Primative.h"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{

  Camera* ViewportBase::GetCamera() const
  {
    return m_camera;
  }

  void ViewportBase::SetCamera(Camera* cam)
  {
    m_camera = cam;
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
    m_viewportImage = new RenderTarget((uint)width, (uint)height);
    m_viewportImage->Init();
  }

  Viewport::~Viewport()
  {
    SafeDel(m_viewportImage);
  }

  void Viewport::OnResize(float width, float height)
  {
    m_width = width;
    m_height = height;

    Camera* cam = GetCamera();
    if (cam->IsOrtographic())
    {
      cam->SetLens(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 1000.0f);
    }
    else
    {
      cam->SetLens(cam->GetData().fov, width, height);
    }

    m_viewportImage->UnInit();
    m_viewportImage->m_width = (uint)width;
    m_viewportImage->m_height = (uint)height;
    m_viewportImage->Init();
  }

  void Viewport::AdjustZoom(float delta)
  {
    m_zoom -= delta;
    m_zoom = glm::max(m_zoom, 0.01f);

    Camera* cam = GetCamera();
    cam->Translate(Vec3(0.0f, 0.0f, -delta));
    if (cam->IsOrtographic())
    {
      cam->SetLens
      (
        -m_zoom * m_width * 0.5f,
        m_zoom * m_width * 0.5f,
        -m_zoom * m_height * 0.5f,
        m_zoom * m_height * 0.5f,
        0.01f,
        1000.0f
      );
    }
  }

  Ray Viewport::RayFromMousePosition()
  {
    return RayFromScreenSpacePoint(GetLastMousePosScreenSpace());
  }

  Ray Viewport::RayFromScreenSpacePoint(const Vec2& pnt)
  {
    Vec2 mcInVs = TransformScreenToViewportSpace(pnt);

    Ray ray;
    ray.position = TransformViewportToWorldSpace(mcInVs);

    Camera* cam = GetCamera();
    if (cam->IsOrtographic())
    {
      ray.direction = cam->GetDir();
    }
    else
    {
      ray.direction = glm::normalize(ray.position - cam->m_node->GetTranslation(TransformationSpace::TS_WORLD));
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

    return glm::unProject(screenPoint, view, project, Vec4(0.0f, 0.0f, m_width, m_height));
  }

  Vec2 Viewport::TransformScreenToViewportSpace(const Vec2& pnt)
  {
    Vec2 vp = pnt - m_wndPos; // In window space.
    vp.y = m_wndContentAreaSize.y - vp.y; // In viewport space.
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

}
