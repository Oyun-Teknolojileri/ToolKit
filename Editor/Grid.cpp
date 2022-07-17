#include "Grid.h"
#include "GlobalDef.h"
#include "ToolKit.h"
#include "Primative.h"
#include "DebugNew.h"
#include <glm/detail/_swizzle.hpp>
#include <glm/detail/_swizzle_func.hpp>

namespace ToolKit
{
  namespace Editor
  {

    Grid::Grid(UVec2 size)
    {
      AddComponent(new MeshComponent());

      // Create grid material.
      if (!GetMaterialManager()->Exist(g_gridMaterialName))
      {
        m_material = GetMaterialManager()->GetCopyOfUnlitMaterial();
        m_material->UnInit();
        m_material->GetRenderState()->blendFunction =
          BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
        m_material->GetRenderState()->cullMode = CullingType::TwoSided;

        m_material->m_vertexShader =
          GetShaderManager()->Create<Shader>
          (
            ShaderPath("gridVertex.shader", true)
          );
        m_material->m_fragmetShader =
          GetShaderManager()->Create<Shader>
          (
            ShaderPath("gridFragment.shader", true)
          );

        m_material->Init();
        GetMaterialManager()->m_storage[g_gridMaterialName] = m_material;
      }
      else
      {
        m_material = GetMaterialManager()->Create<Material>(g_gridMaterialName);
      }

      // Create grid mesh.
      Resize(size);
    }

    void Grid::Resize(UVec2 size, AxisLabel axis, float gridSpaceScale)
    {
      for (int i = 0; i < 2; i++)
      {
        m_size[i] = size[i] % 2 == 0 ? size[i] : size[i] + 1;
      }

      // Set cell size
      m_gridCellSize = gridSpaceScale;

      // Rotate according to axis label and set origin axis line colors
      switch (axis)
      {
        case AxisLabel::XY:
        m_node->SetOrientation
        (
          glm::angleAxis(glm::radians(90.0f), Z_AXIS) *
          RotationTo(X_AXIS, Y_AXIS)
        );
        m_horizontalAxisColor = g_gridAxisRed;
        m_verticalAxisColor = g_gridAxisGreen;
        break;
        case AxisLabel::ZX:
        m_node->SetOrientation(RotationTo(Z_AXIS, Y_AXIS));
        m_horizontalAxisColor = g_gridAxisRed;
        m_verticalAxisColor = g_gridAxisBlue;
        break;
        case AxisLabel::YZ:
        m_node->SetOrientation(
          glm::angleAxis(glm::radians(90.0f), Y_AXIS) *
          RotationTo(X_AXIS, Y_AXIS)
        );
        m_horizontalAxisColor = g_gridAxisGreen;
        m_verticalAxisColor = g_gridAxisBlue;
        break;
      }

      MeshPtr parentMesh = GetMeshComponent()->GetMeshVal();
      parentMesh->UnInit();
      glm::vec2 scale = glm::vec2(m_size) * glm::vec2(0.5f);

      Vec3Array offsets =
      {
        Vec3(-scale.x * 0.5f, scale.y * 0.5f, 0.0f),
        Vec3(scale.x * 0.5f, scale.y * 0.5f, 0.0f),
        Vec3(scale.x * 0.5f, -scale.y * 0.5f, 0.0f),
        Vec3(-scale.x * 0.5f, -scale.y * 0.5f, 0.0f)
      };

      for (int i = 0; i < 4; i++)
      {
        Quad quad;
        MeshPtr mesh = quad.GetMeshComponent()->GetMeshVal();
        for (int j = 0; j < 4; j++)
        {
          Vertex& clientVertex = mesh->m_clientSideVertices[j];
          clientVertex.pos = (clientVertex.pos * glm::vec3(scale, 0.0f)) +
            offsets[i];
          clientVertex.tex = clientVertex.pos.xy * m_gridCellSize;
        }

        mesh->m_material = m_material;
        if (i == 0)
        {
          GetMeshComponent()->SetMeshVal(mesh);
          parentMesh = mesh;
        }
        else
        {
          parentMesh->m_subMeshes.push_back(mesh);
        }
      }

      parentMesh->CalculateAABB();
    }

    bool Grid::HitTest(const Ray& ray, Vec3& pos)
    {
      Mat4 ts = m_node->GetTransform(TransformationSpace::TS_WORLD);
      Mat4 its = glm::inverse(ts);
      Ray rayInObjectSpace =
      {
        its * Vec4(ray.position, 1.0f),
        its * Vec4(ray.direction, 0.0f)
      };

      float dist = 0.0f;
      if (RayBoxIntersection(rayInObjectSpace, GetAABB(), dist))
      {
        pos = PointOnRay(rayInObjectSpace, dist);
        return true;
      }

      return false;
    }

  }  //  namespace Editor
}  //  namespace ToolKit
