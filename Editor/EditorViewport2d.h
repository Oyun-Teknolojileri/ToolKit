#pragma once

#include "AnchorMod.h"
#include "EditorViewport.h"
#include "ToolKit.h"
#include "UI.h"
#include "Viewport.h"

namespace ToolKit
{
  namespace Editor
  {
    class EditorViewport2d : public EditorViewport
    {
     public:
      explicit EditorViewport2d(XmlNode* node);
      explicit EditorViewport2d(const Vec2& size);
      EditorViewport2d(float width, float height);
      virtual ~EditorViewport2d();

      // Window Overrides.
      void Show() override;
      Type GetType() const override;
      void Update(float deltaTime) override;
      void OnResizeContentArea(float width, float height) override;
      void DispatchSignals() const override;

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
      AnchorMod* m_anchorMode = nullptr;
      Vec2 m_canvasSize       = Vec2(640.0f, 480.0f);
      Vec2 m_canvasPos;

     public:
      float m_zoomPercentage     = 100.0f;
      uint m_gridCellSizeByPixel = 10;
    };

  } // namespace Editor
} // namespace ToolKit
