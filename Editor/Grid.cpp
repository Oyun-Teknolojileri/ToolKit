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

#include "Grid.h"

#include "Global.h"
#include "Primative.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    Grid::Grid(UVec2 size, AxisLabel axis, float cellSize, float linePixelCount, bool is2d)
    {
      m_is2d                = is2d;
      m_horizontalAxisColor = g_gridAxisRed;
      m_verticalAxisColor   = g_gridAxisBlue;

      Init();

      // Create grid mesh.
      Resize(size, axis, cellSize);

      UpdateShaderParams();

      m_initiated = true;
    }

    void Grid::Resize(UVec2 size, AxisLabel axis, float cellSize, float linePixelCount)
    {
      // Set cell size
      m_gridCellSize      = cellSize;
      m_maxLinePixelCount = linePixelCount;

      if (VecAllEqual<UVec2>(size, m_size) && m_initiated)
      {
        return;
      }

      const UVec2 maxGridSize(50u);

      for (int i = 0; i < 2; i++)
      {
        m_size[i] = size[i] % 2 == 0 ? size[i] : size[i] + 1;
      }

      // Rotate according to axis label and set origin axis line colors
      switch (axis)
      {
      case AxisLabel::XY:
        m_node->SetOrientation(glm::angleAxis(glm::radians(90.0f), Z_AXIS) * RotationTo(X_AXIS, Y_AXIS));
        m_horizontalAxisColor = g_gridAxisRed;
        m_verticalAxisColor   = g_gridAxisGreen;
        break;
      case AxisLabel::ZX:
        m_node->SetOrientation(RotationTo(Z_AXIS, Y_AXIS));
        m_horizontalAxisColor = g_gridAxisRed;
        m_verticalAxisColor   = g_gridAxisBlue;
        break;
      case AxisLabel::YZ:
        m_node->SetOrientation(glm::angleAxis(glm::radians(90.0f), Y_AXIS) * RotationTo(X_AXIS, Y_AXIS));
        m_horizontalAxisColor = g_gridAxisGreen;
        m_verticalAxisColor   = g_gridAxisBlue;
        break;
      }

      // Get main mesh
      MeshPtr mainMesh = GetMeshComponent()->GetMeshVal();
      if (mainMesh == nullptr)
      {
        mainMesh = std::make_shared<Mesh>();
      }
      mainMesh->UnInit();

      // 40 -> 4 grid grid mesh (32,32 & 8,32 & 32,8 & 8,8)
      // 32 -> 1 grid mesh
      // 64 -> 4 grid mesh
      // 40, 32 -> 2 grid mesh (32,32 & 8, 32)
      // Create new submeshes
      UVec2 gridMeshCount(0);
      for (uint dimIndx = 0; dimIndx < 2; dimIndx++)
      {
        gridMeshCount[dimIndx] = m_size[dimIndx] / maxGridSize[dimIndx];
        bool isThereRemaining  = (m_size[dimIndx] % maxGridSize[dimIndx]);
        gridMeshCount[dimIndx] += isThereRemaining ? 1 : 0;
      }

      // [0,1] Quad, not [-0.5, 0.5] Quad
      VertexArray quadVertexBuffer;
      {
        quadVertexBuffer.resize(6);

        quadVertexBuffer[0].pos  = Vec3(0.0f, 1.0f, 0.0f);
        quadVertexBuffer[0].tex  = Vec2(0.0f, 0.0f);
        quadVertexBuffer[0].norm = Vec3(0.0f, 0.0f, 1.0f);
        quadVertexBuffer[0].btan = Vec3(0.0f, 1.0f, 0.0f);

        quadVertexBuffer[1].pos  = Vec3(0.0f, 0.0f, 0.0f);
        quadVertexBuffer[1].tex  = Vec2(0.0f, 1.0f);
        quadVertexBuffer[1].norm = Vec3(0.0f, 0.0f, 1.0f);
        quadVertexBuffer[1].btan = Vec3(0.0f, 1.0f, 0.0f);

        quadVertexBuffer[2].pos  = Vec3(1.0f, 0.0f, 0.0f);
        quadVertexBuffer[2].tex  = Vec2(1.0f, 1.0f);
        quadVertexBuffer[2].norm = Vec3(0.0f, 0.0f, 1.0f);
        quadVertexBuffer[2].btan = Vec3(0.0f, 1.0f, 0.0f);

        quadVertexBuffer[3]      = quadVertexBuffer[0];
        quadVertexBuffer[4]      = quadVertexBuffer[2];
        quadVertexBuffer[5].pos  = Vec3(1.0f, 1.0f, 0.0f);
        quadVertexBuffer[5].tex  = Vec2(1.0f, 0.0f);
        quadVertexBuffer[5].norm = Vec3(0.0f, 0.0f, 1.0f);
        quadVertexBuffer[5].btan = Vec3(0.0f, 1.0f, 0.0f);
      }

      for (UVec2 gridIndx(0); gridIndx.x < gridMeshCount.x; gridIndx.x++)
      {
        for (gridIndx.y = 0; gridIndx.y < gridMeshCount.y; gridIndx.y++)
        {
          Vec2 scale = min(m_size, maxGridSize);

          for (uint dimIndx = 0; dimIndx < 2; dimIndx++)
          {
            if (gridIndx[dimIndx] == gridMeshCount[dimIndx] - 1 && m_size[dimIndx] % maxGridSize[dimIndx])
            {
              scale[dimIndx] = float(m_size[dimIndx] % maxGridSize[dimIndx]);
            }
          }

          VertexArray currentQuad = quadVertexBuffer;
          for (int j = 0; j < 6; j++)
          {
            Vertex& clientVertex = currentQuad[j];
            clientVertex.pos     = (clientVertex.pos * Vec3(scale, 0.0f));
            clientVertex.pos.xy  -= Vec2(m_size / UVec2(2)) - Vec2(gridIndx * maxGridSize);
            // clientVertex.pos.xy += Vec2(maxGridSize / UVec2(2));
            clientVertex.tex     = clientVertex.pos.xy * m_gridCellSize;
          }

          mainMesh->m_clientSideVertices.insert(mainMesh->m_clientSideVertices.end(),
                                                currentQuad.begin(),
                                                currentQuad.end());
        }
      }

      mainMesh->CalculateAABB();
      mainMesh->Init();

      GetMeshComponent()->SetMeshVal(mainMesh);
    }

    bool Grid::HitTest(const Ray& ray, Vec3& pos)
    {
      float dist = 0.0f;
      if (RayBoxIntersection(ray, GetAABB(true), dist))
      {
        pos = PointOnRay(ray, dist);
        return true;
      }

      return false;
    }

    void Grid::Init()
    {
      AddComponent(new MeshComponent());
      AddComponent(new MaterialComponent());

      // Create grid material.
      if (!GetMaterialManager()->Exist(g_gridMaterialName))
      {
        MaterialPtr material = GetMaterialManager()->GetCopyOfUnlitMaterial();
        material->UnInit();
        material->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;

        material->GetRenderState()->cullMode      = CullingType::TwoSided;

        material->m_vertexShader  = GetShaderManager()->Create<Shader>(ShaderPath("gridVertex.shader", true));

        // Custom creationg & shader management.
        GridFragmentShaderPtr gfs = std::make_shared<GridFragmentShader>();
        gfs->Load();
        GetShaderManager()->Manage(gfs);

        material->m_fragmentShader                          = gfs;
        GetMaterialManager()->m_storage[g_gridMaterialName] = material;
      }

      m_material = GetMaterialManager()->Create<Material>(g_gridMaterialName);
      GetMaterialComponent()->SetFirstMaterial(m_material);
    }

    void Grid::UpdateShaderParams()
    {
      GridFragmentShader* gfs    = static_cast<GridFragmentShader*>(m_material->m_fragmentShader.get());

      gfs->m_sizeEachCell        = m_gridCellSize;
      gfs->m_axisColorHorizontal = m_horizontalAxisColor;
      gfs->m_axisColorVertical   = m_verticalAxisColor;
      gfs->m_maxLinePixelCount   = m_maxLinePixelCount;
      gfs->m_is2DViewport        = m_is2d;
    }

    GridFragmentShader::GridFragmentShader()
    {
      SetFile(ShaderPath("gridFragment.shader", true));

      // Set defaults.
      m_sizeEachCell        = 0.1f;
      m_maxLinePixelCount   = 2.0f;
      m_axisColorHorizontal = X_AXIS;
      m_axisColorVertical   = Z_AXIS;
      m_is2DViewport        = false;
    }

    GridFragmentShader::~GridFragmentShader() {}

    void GridFragmentShader::UpdateShaderParameters()
    {
      SetShaderParameter("GridData.cellSize", m_sizeEachCell);
      SetShaderParameter("GridData.lineMaxPixelCount", m_maxLinePixelCount);
      SetShaderParameter("GridData.horizontalAxisColor", m_axisColorHorizontal);
      SetShaderParameter("GridData.verticalAxisColor", m_axisColorVertical);
      SetShaderParameter("GridData.is2DViewport", m_is2DViewport);
    }

  } //  namespace Editor
} //  namespace ToolKit
