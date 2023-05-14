#pragma once

#include "Entity.h"
#include "Shader.h"

namespace ToolKit
{
  namespace Editor
  {

    class GridFragmentShader : public Shader
    {
     public:
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
      explicit Grid(UVec2 size,
                    AxisLabel axis,
                    float cellSize,
                    float linePixelCount,
                    bool is2d);

      void Resize(UVec2 size,
                  AxisLabel axis       = AxisLabel::ZX,
                  float cellSize       = 1.0f,
                  float linePixelCount = 2.0f);

      bool HitTest(const Ray& ray, Vec3& pos);
      void UpdateShaderParams();

     private:
      void Init();

     private:
      Vec3 m_horizontalAxisColor;
      Vec3 m_verticalAxisColor;

      UVec2 m_size;                     // m^2 size of the grid.
      float m_gridCellSize      = 1.0f; // m^2 size of each cell
      float m_maxLinePixelCount = 2.0f;
      bool m_is2d               = false;
      bool m_initiated          = false;
      MaterialPtr m_material    = nullptr;
    };

  } //  namespace Editor
} //  namespace ToolKit
