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
      void OnResize(float width, float height) override;

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
      void DrawCanvasToolBar();
      void AdjustZoom(float delta) override;

     private:
      void Init2dCam();
      void UpdateCanvasSize();
      void PanZoom(float deltaTime);

     private:
      Vec2 m_canvasSize;
      Vec2 m_canvasPos;

      Vec2 m_contentAreaMin;
      Vec2 m_contentAreaMax;
      Vec2 m_scroll;

      Vec2 m_layoutSize;

      DirectionalLight m_forwardLight;

     public:
      float m_zoomPercentage = 100;
      uint16_t m_gridCellSizeByPixel = 10;
      IVec2 m_gridWholeSize = IVec2(640, 480);
    };

  }  // namespace Editor

}  // namespace ToolKit
