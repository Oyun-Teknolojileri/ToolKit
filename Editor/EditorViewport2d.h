/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
      EditorViewport2d();
      virtual ~EditorViewport2d();

      void Init(Vec2 size) override;

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

     private:
      AnchorMod* m_anchorMode = nullptr;

     public:
      float m_zoomPercentage     = 100.0f;
      uint m_gridCellSizeByPixel = 10;
    };

  } // namespace Editor
} // namespace ToolKit
