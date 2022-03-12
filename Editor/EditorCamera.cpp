#include "stdafx.h"
#include "EditorCamera.h"
#include "DebugNew.h"

namespace ToolKit
{

  namespace Editor
  {

    EditorCamera::EditorCamera()
    {
      MeshComponent* meshCom = new MeshComponent();
      AddComponent(meshCom);
      GenerateFrustum();
    }

    EditorCamera::~EditorCamera()
    {

    }

    bool EditorCamera::IsDrawable() const
    {
      return false;
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

      MeshComponent* mc = static_cast<MeshComponent*> (GetFirstComponent(ComponentType::Component_Mesh));
      
      LineBatch fs(lines, Vec3(), DrawType::Line);
      mc->m_mesh = fs.m_mesh;
      fs.m_mesh = nullptr;

      // Triangle part.
      VertexArray vertices;
      vertices.resize(3);

      vertices[0].pos = Vec3(-0.3f, 0.35f, 1.6f);
      vertices[1].pos = Vec3(0.3f, 0.35f, 1.6f);
      vertices[2].pos = Vec3(0.0f, 0.65f, 1.6f);

      MeshPtr mesh = std::make_shared<Mesh>();
      mesh->m_vertexCount = (uint)vertices.size();
      mesh->m_clientSideVertices = vertices;
      mesh->m_material = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      mesh->m_material->m_color = Vec3();
      mesh->m_material->m_color = Vec3();
      mesh->m_material->GetRenderState()->cullMode = CullingType::TwoSided;

      mesh->CalculateAABB();
      mesh->ConstructFaces();

      mc->m_mesh->m_subMeshes.push_back(mesh);
      mc->m_mesh->Init();
    }

  }

}
