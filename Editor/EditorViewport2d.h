/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

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
      TKDeclareClass(EditorViewport2d, EditorViewport);

      EditorViewport2d();
      virtual ~EditorViewport2d();

      void Init(Vec2 size) override;

      // Window Overrides.
      void Update(float deltaTime) override;
      void OnResizeContentArea(float width, float height) override;
      void DispatchSignals() const override;

     protected:
      void HandleDrop() override;
      void DrawOverlays() override;
      void AdjustZoom(float delta) override;

     private:
      void PanZoom(float deltaTime);

     private:
      AnchorMod* m_anchorMode = nullptr;

     public:
      float m_zoomPercentage     = 100.0f;
      uint m_gridCellSizeByPixel = 10;
    };

  } // namespace Editor
} // namespace ToolKit
