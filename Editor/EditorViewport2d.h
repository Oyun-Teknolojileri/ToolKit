#pragma once

#include "AnchorMod.h"
#include "EditorViewport.h"

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
      Type GetType() const override;
      void Update(float deltaTime) override;
      void OnResizeContentArea(float width, float height) override;
      void DispatchSignals() const override;

     protected:
      void HandleDrop() override;
      void DrawOverlays() override;
      void AdjustZoom(float delta) override;

     private:
      void PanZoom(float deltaTime);
      void InitViewport();

     private:
      AnchorMod* m_anchorMode = nullptr;

     public:
      float m_zoomPercentage     = 100.0f;
      uint m_gridCellSizeByPixel = 10;
    };

  } // namespace Editor
} // namespace ToolKit
