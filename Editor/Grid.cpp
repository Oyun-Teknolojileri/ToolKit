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
        m_material->m_diffuseTexture = GetTextureManager()->Create<Texture>
          (
            TexturePath("grid.png")
          );
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
      Resize(size);
    }

    void Grid::Resize(UVec2 size, AxisLabel axis, float gridSpaceScale)
    {
      for (int i = 0; i < 2; i++)
      {
        m_size[i] = size[i] % 2 == 0 ? size[i] : size[i] + 1;
      }

      bool axises[3] = { false, false, false };

      assert
      (
        (
          axis == AxisLabel::XY ||
          axis == AxisLabel::YZ ||
          axis == AxisLabel::ZX
        ) && "Only works for given values."
      );

      if (axis == AxisLabel::XY)
      {
        axises[0] = true;
        axises[1] = true;
      }

      if (axis == AxisLabel::YZ)
      {
        axises[1] = true;
        axises[2] = true;
      }

      if (axis == AxisLabel::ZX)
      {
        axises[0] = true;
        axises[2] = true;
      }

      MeshPtr parentMesh = GetMeshComponent()->GetMeshVal();
      parentMesh->UnInit();
      glm::vec2 scale = glm::vec2(m_size) * glm::vec2(0.5f);

      Vec3Array offsets =
      {
        Vec3(-scale.x * 0.5f - 0.025f, 0.0f, scale.y * 0.5f + 0.025f),
        Vec3(scale.x * 0.5f + 0.025f, 0.0f, scale.y * 0.5f + 0.025f),
        Vec3(scale.x * 0.5f + 0.025f, 0.0f, -scale.y * 0.5f - 0.025f),
        Vec3(-scale.x * 0.5f - 0.025f, 0.0f, -scale.y * 0.5f - 0.025f)
      };

      // Used to manupulate axes.
      auto axisConvertFn = [&axises](Vertex& v) -> void
      {
        Vec3 prevVertex = v.pos;
        bool isFirstOnePicked = false;
        for (ubyte axisIndex = 0; axisIndex < 3; axisIndex++)
        {
          if (axises[axisIndex] == false)
          {
            v.pos[axisIndex] = prevVertex.y;
          }
          else if (!isFirstOnePicked)
          {
            v.pos[axisIndex] = prevVertex.x;
            isFirstOnePicked = true;
          }
          else
          {
            v.pos[axisIndex] = prevVertex.z;
          }
        }
      };

      for (int i = 0; i < 4; i++)
      {
        Quad quad;
        MeshPtr mesh = quad.GetMeshComponent()->GetMeshVal();
        for (int j = 0; j < 4; j++)
        {
          Vertex& clientVertex = mesh->m_clientSideVertices[j];
          clientVertex.pos =
            (
              clientVertex.pos *
              glm::vec3
              (
                scale.x,
                scale.y,
                0.0f
              )
            ).xzy + offsets[i];
          clientVertex.tex = clientVertex.pos.xz * gridSpaceScale;
          axisConvertFn(clientVertex);
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

      VertexArray vertices;
      vertices.resize(2);

      // x - z lines.
      Vec3 lines[2] =
      {
        Vec3(0.05f, scale.x * 2.0f, 1.0f),
        Vec3(scale.y * 2.0f, 0.05f, 1.0f)
      };

      for (int i = 0; i < 2; i++)
      {
        MaterialPtr solidMat = GetMaterialManager()->
          GetCopyOfUnlitColorMaterial();
        solidMat->GetRenderState()->cullMode = CullingType::TwoSided;
        solidMat->m_color = g_gridAxisBlue;
        Vec3 ls = lines[i];

        // Set axis colors
        bool iZero = i == 0;
        switch (axis)
        {
          case AxisLabel::XY:
            solidMat->m_color = iZero ? g_gridAxisGreen : g_gridAxisRed;
          break;
          case AxisLabel::YZ:
            solidMat->m_color = iZero ? g_gridAxisBlue : g_gridAxisGreen;
          break;
          case AxisLabel::ZX:
            solidMat->m_color = iZero ? g_gridAxisBlue : g_gridAxisRed;
          break;
        }

        Quad quad;
        MeshPtr mesh = quad.GetMeshComponent()->GetMeshVal();
        for (int j = 0; j < 4; j++)
        {
          Vertex& clientVertex = mesh->m_clientSideVertices[j];
          clientVertex.pos = (clientVertex.pos * ls).xzy;
          axisConvertFn(clientVertex);
        }
        mesh->m_material = solidMat;
        parentMesh->m_subMeshes.push_back(mesh);
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
