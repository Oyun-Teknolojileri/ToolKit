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
        m_material->m_diffuseTexture = GetTextureManager()->Create<Texture>(
          TexturePath("grid.png"));
        m_material->GetRenderState()->blendFunction =
          BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
        m_material->GetRenderState()->cullMode = CullingType::TwoSided;
        m_material->Init();

        GetMaterialManager()->m_storage[g_gridMaterialName] = m_material;
      }
      else
      {
        m_material = GetMaterialManager()->Create<Material>(g_gridMaterialName);
      }

      // Create grid mesh.
      m_size.x = size.x % 2 == 0 ? size.x : size.x + 1;
      m_size.y = size.y % 2 == 0 ? size.y : size.y + 1;
      Resize(size);
    }

    void Grid::Resize(UVec2 size, AxisLabel axis, float gridSpaceScale)
    {
      m_size.x = size.x % 2 == 0 ? size.x : size.x + 1;
      m_size.y = size.y % 2 == 0 ? size.y : size.y + 1;

      bool axises[3] = { false, false, false };
      if (axis != AxisLabel::XY &&
        axis != AxisLabel::YZ &&
        axis != AxisLabel::ZX)
      {
        GetLogger()->WriteConsole(LogType::Error,
          "Grid::Resize has invalid x,y,z info; please report this!"); return;
      }
      else
      {
        if (axis == AxisLabel::XY)
        {
          axises[0] = true; axises[1] = true;
        }
        if (axis == AxisLabel::YZ)
        {
          axises[1] = true; axises[2] = true;
        }
        if (axis == AxisLabel::ZX)
        {
          axises[0] = true; axises[2] = true;
        }
      }
      MeshPtr& parentMesh = GetMeshComponent()->Mesh();
      parentMesh->UnInit();
      glm::vec2 scale = glm::vec2(m_size) * glm::vec2(0.5f);

      Vec3Array offsets =
      {
        Vec3(-scale.x * 0.5f - 0.025f, 0.0f, scale.y * 0.5f + 0.025f),
        Vec3(scale.x * 0.5f + 0.025f, 0.0f, scale.y * 0.5f + 0.025f),
        Vec3(scale.x * 0.5f + 0.025f, 0.0f, -scale.y * 0.5f - 0.025f),
        Vec3(-scale.x * 0.5f - 0.025f, 0.0f, -scale.y * 0.5f - 0.025f)
      };

      for (int i = 0; i < 4; i++)
      {
        Quad quad;
        MeshPtr& mesh = quad.GetMeshComponent()->Mesh();
        for (int j = 0; j < 4; j++)
        {
          ToolKit::Vertex& clientVertex = mesh->m_clientSideVertices[j];
          clientVertex.pos = (clientVertex.pos
            * glm::vec3(scale.x, scale.y, 0.0f)).xzy + offsets[i];
          clientVertex.tex = clientVertex.pos.xz * gridSpaceScale;


          // Convert according to new axises
          Vec3 prevVertex = clientVertex.pos;
          bool isFirstOnePicked = false;
          for (uint8_t axisIndex = 0; axisIndex < 3; axisIndex++)
          {
            if (axises[axisIndex] == false)
            {
              clientVertex.pos[axisIndex] = prevVertex.y;
            }
            else if (!isFirstOnePicked)
            {
              clientVertex.pos[axisIndex] = prevVertex.x;
              isFirstOnePicked = true;
            }
            else
            {
              clientVertex.pos[axisIndex] = prevVertex.z;
            }
          }
        }

        mesh->m_material = m_material;
        if (i == 0)
        {
          GetMeshComponent()->Mesh() = mesh;
        }
        else
        {
          parentMesh->m_subMeshes.push_back(mesh);
        }
      }

      VertexArray vertices;
      vertices.resize(2);

      // x - z lines.
      Vec3 ls_es[2] = {
        Vec3(0.05f, scale.x * 2.0f, 1.0f),
        Vec3(scale.y * 2.0f, 0.05f, 1.0f)
      };
      for (int i = 0; i < 2; i++)
      {
        MaterialPtr solidMat = GetMaterialManager()->
          GetCopyOfUnlitColorMaterial();
        solidMat->GetRenderState()->cullMode = CullingType::TwoSided;
        solidMat->m_color = g_gridAxisBlue;
        Vec3 ls = ls_es[i];

        // Choose axis color
        if (i == 0)
        {
          // Set axis colors
          switch (axis)
          {
            case AxisLabel::XY:
            solidMat->m_color = g_gridAxisGreen;
            break;
            case AxisLabel::YZ:
            solidMat->m_color = g_gridAxisBlue;
            break;
            case AxisLabel::ZX:
            solidMat->m_color = g_gridAxisBlue;
            break;
          }
        }
        else
        {
          // Set axis colors
          switch (axis)
          {
            case AxisLabel::XY:
            solidMat->m_color = g_gridAxisRed;
            break;
            case AxisLabel::YZ:
            solidMat->m_color = g_gridAxisGreen;
            break;
            case AxisLabel::ZX:
            solidMat->m_color = g_gridAxisRed;
            break;
          }
        }

        Quad quad;
        MeshPtr& mesh = quad.GetMeshComponent()->Mesh();
        for (int j = 0; j < 4; j++)
        {
          Vertex& clientVertex = mesh->m_clientSideVertices[j];
          clientVertex.pos = (clientVertex.pos * ls).xzy;

          // Convert according to new axises
          Vec3 prevVertex = clientVertex.pos;
          bool isFirstOnePicked = false;
          for (uint8_t axisIndex = 0; axisIndex < 3; axisIndex++)
          {
            if (axises[axisIndex] == false)
            {
              clientVertex.pos[axisIndex] = prevVertex.y;
            }
            else if (!isFirstOnePicked)
            {
              clientVertex.pos[axisIndex] = prevVertex.x;
              isFirstOnePicked = true;
            }
            else
            {
              clientVertex.pos[axisIndex] = prevVertex.z;
            }
          }
        }
        mesh->m_material = solidMat;
        parentMesh->m_subMeshes.push_back(mesh);
      }

      // Set axis colors
      switch (axis)
      {
        case AxisLabel::XY:
        parentMesh->m_subMeshes[0]->m_material->m_color = g_gridAxisRed;
        parentMesh->m_subMeshes[1]->m_material->m_color = g_gridAxisGreen;
        break;
        case AxisLabel::YZ:
        parentMesh->m_subMeshes[0]->m_material->m_color = g_gridAxisGreen;
        parentMesh->m_subMeshes[1]->m_material->m_color = g_gridAxisBlue;
        break;
        case AxisLabel::ZX:
        parentMesh->m_subMeshes[0]->m_material->m_color = g_gridAxisBlue;
        parentMesh->m_subMeshes[1]->m_material->m_color = g_gridAxisRed;
        break;
        default:
        break;
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
