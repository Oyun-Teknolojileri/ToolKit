/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Grid.h"

#include "Global.h"

#include <Material.h>
#include <MathUtil.h>
#include <Mesh.h>
#include <Primative.h>
#include <ToolKit.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    // GridFragmentShader
    //////////////////////////////////////////////////////////////////////////

    TKDefineClass(GridFragmentShader, Shader);

    GridFragmentShader::GridFragmentShader() { SetFile(ShaderPath("gridFragment.shader", true)); }

    GridFragmentShader::~GridFragmentShader() {}

    // Grid
    //////////////////////////////////////////////////////////////////////////

    TKDefineClass(Grid, Entity);

    Grid::Grid()
    {
      Vec3 m_horizontalAxisColor = g_gridAxisRed;
      Vec3 m_verticalAxisColor   = g_gridAxisBlue;

      m_gridCellSize             = 0.1f;
      m_maxLinePixelCount        = 2.0f;
      m_horizontalAxisColor      = X_AXIS;
      m_verticalAxisColor        = Z_AXIS;
      m_is2d                     = false;
    }

    void Grid::NativeConstruct()
    {
      Super::NativeConstruct();

      Init();
      UpdateShaderParams();
    }

    void Grid::Resize(UVec2 size, AxisLabel axis, float cellSize, float linePixelCount)
    {
      // Set cell size
      m_gridCellSize      = cellSize;
      m_maxLinePixelCount = linePixelCount;

      if (VecAllEqual<UVec2>(size, m_size))
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
        mainMesh = MakeNewPtr<Mesh>();
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
        gridMeshCount[dimIndx]  = m_size[dimIndx] / maxGridSize[dimIndx];
        bool isThereRemaining   = (m_size[dimIndx] % maxGridSize[dimIndx]);
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
            clientVertex.pos = clientVertex.pos - Vec3(Vec2(m_size / UVec2(2)) - Vec2(gridIndx * maxGridSize), 0.0f);
            clientVertex.tex = clientVertex.pos * m_gridCellSize;
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
      if (RayBoxIntersection(ray, GetBoundingBox(true), dist))
      {
        pos = PointOnRay(ray, dist);
        return true;
      }

      return false;
    }

    void Grid::Init()
    {
      if (m_initiated)
      {
        return;
      }

      AddComponent<MeshComponent>();
      AddComponent<MaterialComponent>();

      // Create grid material.
      if (!GetMaterialManager()->Exist(g_gridMaterialName))
      {
        MaterialPtr material = GetMaterialManager()->GetCopyOfUnlitMaterial();

        material->UnInit();
        material->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
        material->GetRenderState()->cullMode      = CullingType::TwoSided;
        material->m_vertexShader  = GetShaderManager()->Create<Shader>(ShaderPath("gridVertex.shader", true));

        // Custom creation & shader management.
        GridFragmentShaderPtr gfs = MakeNewPtr<GridFragmentShader>();
        gfs->Load();
        GetShaderManager()->Manage(gfs);

        material->m_fragmentShader                          = gfs;
        GetMaterialManager()->m_storage[g_gridMaterialName] = material;
      }

      m_material = GetMaterialManager()->Create<ShaderMaterial>(g_gridMaterialName);
      GetMaterialComponent()->SetFirstMaterial(m_material);

      m_initiated = true;
    }

    void Grid::UpdateShaderParams()
    {
      GridFragmentShader* gfs = static_cast<GridFragmentShader*>(m_material->m_fragmentShader.get());

      m_material->UpdateProgramUniform("GridData.cellSize", m_gridCellSize);
      m_material->UpdateProgramUniform("GridData.lineMaxPixelCount", m_maxLinePixelCount);
      m_material->UpdateProgramUniform("GridData.horizontalAxisColor", m_horizontalAxisColor);
      m_material->UpdateProgramUniform("GridData.verticalAxisColor", m_verticalAxisColor);
      m_material->UpdateProgramUniform("GridData.is2DViewport", m_is2d);
    }

  } //  namespace Editor
} //  namespace ToolKit
