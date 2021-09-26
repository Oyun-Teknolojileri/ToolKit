#pragma once

#include "ToolKit.h"
#include "MathUtil.h"

namespace ToolKit
{
  class Camera;
  class RenderTarget;

  class Viewport
  {
  public:
    Viewport();
    Viewport(float width, float height);
    virtual ~Viewport();

    // Update internal states. Window provider should fill this in.
    virtual void Update(float deltaTime) = 0;

    // Utility Functions.
    Ray RayFromMousePosition();
    Ray RayFromScreenSpacePoint(const Vec2& pnt);
    Vec3 GetLastMousePosWorldSpace();
    virtual Vec2 GetLastMousePosViewportSpace();
    virtual Vec2 GetLastMousePosScreenSpace();
    virtual Vec3 TransformViewportToWorldSpace(const Vec2& pnt);
    virtual Vec2 TransformScreenToViewportSpace(const Vec2& pnt);

  protected:
    // Internal window handling.
    void OnResize(float width, float height);

  public:
    // ToolKit bindings.
    Camera* m_camera = nullptr;
    RenderTarget* m_viewportImage = nullptr;

    // Window properties.
    Vec2 m_wndPos;
    float m_width;
    float m_height;

  protected:
    // States.
    bool m_mouseOverContentArea = false;
    Vec2 m_wndContentAreaSize;
    IVec2 m_mousePosBegin;
    IVec2 m_lastMousePosRelContentArea;
  };

}
