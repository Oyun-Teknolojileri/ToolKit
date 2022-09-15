#include "Grid.h"

#include "GlobalDef.h"
#include "Primative.h"
#include "ToolKit.h"

#include <glm/detail/_swizzle.hpp>
#include <glm/detail/_swizzle_func.hpp>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    Grid::Grid(UVec2 size, AxisLabel axis, float cellSize, float linePixelCount)
    {
      Init();

      // Create grid mesh.
      Resize(size, axis, cellSize);

      m_initiated = true;
    }

    void Grid::Resize(UVec2 size,
                      AxisLabel axis,
                      float cellSize,
                      float linePixelCount)
    {
      if (VecAllEqual<UVec2>(size, m_size) && m_initiated)
      {
        return;
      }

      const UVec2 maxGridSize(100);
      for (int i = 0; i < 2; i++)
      {
        m_size[i] = size[i] % 2 == 0 ? size[i] : size[i] + 1;
      }

      // Set cell size
      m_gridCellSize      = cellSize;
      m_maxLinePixelCount = linePixelCount;

      // Rotate according to axis label and set origin axis line colors
      switch (axis)
      {
      case AxisLabel::XY:
        m_node->SetOrientation(glm::angleAxis(glm::radians(90.0f), Z_AXIS) *
                               RotationTo(X_AXIS, Y_AXIS));
        m_horizontalAxisColor = g_gridAxisRed;
        m_verticalAxisColor   = g_gridAxisGreen;
        break;
      case AxisLabel::ZX:
        m_node->SetOrientation(RotationTo(Z_AXIS, Y_AXIS));
        m_horizontalAxisColor = g_gridAxisRed;
        m_verticalAxisColor   = g_gridAxisBlue;
        break;
      case AxisLabel::YZ:
        m_node->SetOrientation(glm::angleAxis(glm::radians(90.0f), Y_AXIS) *
                               RotationTo(X_AXIS, Y_AXIS));
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
            if (gridIndx[dimIndx] == gridMeshCount[dimIndx] - 1 &&
                m_size[dimIndx] % maxGridSize[dimIndx])
            {
              scale[dimIndx] = float(m_size[dimIndx] % maxGridSize[dimIndx]);
            }
          }

          VertexArray currentQuad = quadVertexBuffer;
          for (int j = 0; j < 6; j++)
          {
            Vertex& clientVertex = currentQuad[j];
            clientVertex.pos     = (clientVertex.pos * Vec3(scale, 0.0f));
            clientVertex.pos.xy -=
                Vec2(m_size / UVec2(2)) - Vec2(gridIndx * maxGridSize);
            // clientVertex.pos.xy += Vec2(maxGridSize / UVec2(2));
            clientVertex.tex = clientVertex.pos.xy * m_gridCellSize;
          }

          mainMesh->m_clientSideVertices.insert(
              mainMesh->m_clientSideVertices.end(),
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
        material->GetRenderState()->blendFunction =
            BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;

        material->GetRenderState()->cullMode = CullingType::TwoSided;

        material->m_vertexShader = GetShaderManager()->Create<Shader>(
            ShaderPath("gridVertex.shader", true));

        material->m_fragmetShader = GetShaderManager()->Create<Shader>(
            ShaderPath("gridFragment.shader", true));

        material->GetRenderState()->priority = 100;
        material->Init();

        GetMaterialManager()->m_storage[g_gridMaterialName] = material;
      }

      GetMaterialComponent()->SetMaterialVal(
          GetMaterialManager()->Create<Material>(g_gridMaterialName));
    }

  } //  namespace Editor
} //  namespace ToolKit
