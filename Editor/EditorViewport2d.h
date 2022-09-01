#pragma once

#include "ToolKit.h"
#include "Viewport.h"
#include "UI.h"
#include "EditorViewport.h"

namespace ToolKit
{
  class Camera;
  class RenderTarget;

  namespace Editor
  {
    class EditorViewport2d : public EditorViewport
    {
     public:
      explicit EditorViewport2d(XmlNode* node);
      EditorViewport2d(float width, float height);
      virtual ~EditorViewport2d();

      // Window Overrides.
      void Show() override;
      Type GetType() const override;
      void Update(float deltaTime) override;
      void OnResizeContentArea(float width, float height) override;

      // Viewport Overrides.
      Vec2 GetLastMousePosViewportSpace() override;
      Vec2 GetLastMousePosScreenSpace() override;
      Vec3 TransformViewportToWorldSpace(const Vec2& pnt) override;
      Vec2 TransformScreenToViewportSpace(const Vec2& pnt) override;

      // Editor overrides.
      // Consider Canvas as the content area.
      virtual void GetContentAreaScreenCoordinates(Vec2* min, Vec2* max) const;

     protected:
      void UpdateContentArea();
      void UpdateWindow();
      void DrawCommands();
      void HandleDrop();
      void DrawOverlays();
      void AdjustZoom(float delta) override;

     private:
      void PanZoom(float deltaTime);
      void InitViewport();

     private:
      Vec2 m_canvasSize = Vec2(640.0f, 480.0f);
      Vec2 m_canvasPos;

     public:
      float m_zoomPercentage = 100.0f;
      uint m_gridCellSizeByPixel = 10;
    };

  }  // namespace Editor
}  // namespace ToolKit
