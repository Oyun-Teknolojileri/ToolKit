/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
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
