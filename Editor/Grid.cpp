#include "stdafx.h"

#include "ImGui/imgui.h"

#include "Grid.h"
#include "GlobalDef.h"
#include "ToolKit.h"
#include "Primative.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    Grid::Grid(uint size)
    {
      // Create grid material.
      if (!GetMaterialManager()->Exist(g_gridMaterialName))
      {
        m_material = GetMaterialManager()->GetCopyOfUnlitMaterial();
        m_material->UnInit();
        m_material->m_diffuseTexture = GetTextureManager()->Create<Texture>(TexturePath("grid.png"));
        m_material->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
        m_material->GetRenderState()->cullMode = CullingType::TwoSided;
        m_material->Init();

        GetMaterialManager()->m_storage[g_gridMaterialName] = m_material;
      }
      else
      {
        m_material = GetMaterialManager()->Create<Material>(g_gridMaterialName);
      }

      // Create grid mesh.
      m_size = size % 2 == 0 ? size : size + 1;
      Resize(size);
    }

    void Grid::Resize(uint size, float gridSpaceScale)
    {
      MeshPtr& parentMesh = GetMesh();
      parentMesh->UnInit();
      float scale = (float)m_size * 0.5f;

      Vec3Array offsets =
      {
        Vec3(-scale * 0.5f - 0.025f, 0.0f, scale * 0.5f + 0.025f),
        Vec3(scale * 0.5f + 0.025f, 0.0f, scale * 0.5f + 0.025f),
        Vec3(scale * 0.5f + 0.025f, 0.0f, -scale * 0.5f - 0.025f),
        Vec3(-scale * 0.5f - 0.025f, 0.0f, -scale * 0.5f - 0.025f)
      };

      for (int i = 0; i < 4; i++)
      {
        Quad quad;
        MeshPtr& mesh = quad.GetMesh();
        for (int j = 0; j < 4; j++)
        {
          mesh->m_clientSideVertices[j].pos = (mesh->m_clientSideVertices[j].pos * scale).xzy + offsets[i];
          mesh->m_clientSideVertices[j].tex = mesh->m_clientSideVertices[j].pos.xz * gridSpaceScale;
        }

        mesh->m_material = m_material;
        if (i == 0)
        {
          SetMesh(mesh);
        }
        else
        {
          parentMesh->m_subMeshes.push_back(mesh);
        }
      }

      VertexArray vertices;
      vertices.resize(2);

      // x - z lines.
      Vec3 ls = Vec3(0.05f, scale * 2.0f, 1.0f);
      for (int i = 0; i < 2; i++)
      {
        MaterialPtr solidMat = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
        solidMat->GetRenderState()->cullMode = CullingType::TwoSided;
        solidMat->m_color = g_gridAxisBlue;

        if (i == 1)
        {
          ls = ls.yxz;
          solidMat->m_color = g_gridAxisRed;
        }

        Quad quad;
        MeshPtr& mesh = quad.GetMesh();
        for (int j = 0; j < 4; j++)
        {
          mesh->m_clientSideVertices[j].pos = (mesh->m_clientSideVertices[j].pos * ls).xzy;
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
  }
}
