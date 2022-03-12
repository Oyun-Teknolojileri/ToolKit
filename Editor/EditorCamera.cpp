#include "stdafx.h"
#include "EditorCamera.h"

namespace ToolKit
{

  namespace Editor
  {

    EditorCamera::EditorCamera()
    {

    }

    EditorCamera::~EditorCamera()
    {

    }

    bool EditorCamera::IsDrawable() const
    {
      return true;
    }

    void EditorCamera::GenerateFrustum()
    {
      // Line frustum.
      Vec3Array corners =
      {
        Vec3(-0.5f, 0.3f, 1.6f),
        Vec3(0.5f, 0.3f, 1.6f),
        Vec3(0.5f, -0.3f, 1.6f),
        Vec3(-0.5f, -0.3f, 1.6f),
      };

      Vec3 eye;
      Vec3Array lines =
      {
        eye, corners[0],
        eye, corners[1],
        eye, corners[2],
        eye, corners[3],
        corners[0], corners[1],
        corners[1], corners[2],
        corners[2], corners[3],
        corners[3], corners[0]
      };

      LineBatch fs(lines, Vec3(), DrawType::Line);
      m_mesh = fs.m_mesh;
      fs.m_mesh = nullptr;

      // Triangle part.
      VertexArray vertices;
      vertices.resize(3);

      vertices[0].pos = Vec3(-0.5f, 0.5f, 0.0f);
      vertices[1].pos = Vec3(-0.5f, -0.5f, 0.0f);
      vertices[2].pos = Vec3(0.5f, -0.5f, 0.0f);

      MeshPtr mesh = std::make_shared<Mesh>();
      mesh->m_vertexCount = (uint)vertices.size();
      mesh->m_clientSideVertices = vertices;
      mesh->m_material = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      mesh->m_material->m_color = Vec3();

      mesh->CalculateAABB();
      mesh->ConstructFaces();

      m_mesh->m_subMeshes.push_back(mesh);
      m_mesh->Init();
    }

  }

}
