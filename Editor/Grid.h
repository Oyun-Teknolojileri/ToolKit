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

#include <Entity.h>
#include <Shader.h>

namespace ToolKit
{
  namespace Editor
  {

    class GridFragmentShader : public Shader
    {
     public:
      TKDeclareClass(GridFragmentShader, Shader);

      GridFragmentShader();
      virtual ~GridFragmentShader();
      void UpdateShaderParameters() override;

     public:
      ParameterVariant m_sizeEachCell;
      ParameterVariant m_maxLinePixelCount;
      ParameterVariant m_axisColorHorizontal;
      ParameterVariant m_axisColorVertical;
      ParameterVariant m_is2DViewport;
    };

    typedef std::shared_ptr<GridFragmentShader> GridFragmentShaderPtr;

    class Grid : public Entity
    {
     public:
      TKDeclareClass(Grid, Entity);

      Grid();
      void NativeConstruct() override;
      void Resize(UVec2 size, AxisLabel axis = AxisLabel::ZX, float cellSize = 1.0f, float linePixelCount = 2.0f);
      bool HitTest(const Ray& ray, Vec3& pos);
      void UpdateShaderParams();

     private:
      void Init();

     public:
      bool m_is2d = false;

     private:
      Vec3 m_horizontalAxisColor;
      Vec3 m_verticalAxisColor;

      UVec2 m_size;                     // m^2 size of the grid.
      float m_gridCellSize      = 1.0f; // m^2 size of each cell
      float m_maxLinePixelCount = 2.0f;
      bool m_initiated          = false;
      MaterialPtr m_material    = nullptr;
    };

    typedef std::shared_ptr<Grid> GridPtr;

  } //  namespace Editor
} //  namespace ToolKit
