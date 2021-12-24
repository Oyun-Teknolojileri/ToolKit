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
      EditorViewport2d(XmlNode* node);
      EditorViewport2d(float width, float height);
      virtual ~EditorViewport2d();

      // Window Overrides.
      virtual void Show() override;
      virtual Type GetType() const override;
      virtual void Update(float deltaTime) override;
      virtual void OnResize(float width, float height) override;

      // Viewport Overrides.
      virtual Vec2 GetLastMousePosViewportSpace() override;
      virtual Vec2 GetLastMousePosScreenSpace() override;
      virtual Vec3 TransformViewportToWorldSpace(const Vec2& pnt) override;
      virtual Vec2 TransformScreenToViewportSpace(const Vec2& pnt) override;

      // Editor overrides.
      virtual void Render(App* app) override;
      virtual void GetContentAreaScreenCoordinates(Vec2& min, Vec2& max) const override; // Consider Canvas as the content area.

    protected:
      void UpdateContentArea();
      void UpdateWindow();
      void DrawCommands();
      void HandleDrop();
      void DrawOverlays();
      void DrawCanvasToolBar();
      virtual void AdjustZoom(float delta) override;

    private:
      void Init2dCam();
      void UpdateCanvasSize();
      void PanZoom(float deltaTime);
      void GenerateGrid();

    public:
      Vec2 m_canvasSize;

    private:
      Vec2 m_canvasPos;
      Vec2 m_contentAreaMin;
      Vec2 m_contentAreaMax;
      Vec2 m_scroll;
      Light m_forwardLight;
      LineBatch m_grid;
    };

  }

}
