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

      MeshComponentPtr camMeshComp = GetComponent<MeshComponent>();
      LineBatch frusta(lines, Vec3(), DrawType::Line);
      camMeshComp->m_mesh = frusta.GetComponent<MeshComponent>()->m_mesh;

      // Triangle part.
      VertexArray vertices;
      vertices.resize(3);

      vertices[0].pos = Vec3(-0.3f, 0.35f, 1.6f);
      vertices[1].pos = Vec3(0.3f, 0.35f, 1.6f);
      vertices[2].pos = Vec3(0.0f, 0.65f, 1.6f);

      MeshPtr subMesh = std::make_shared<Mesh>();
      subMesh->m_vertexCount = (uint)vertices.size();
      subMesh->m_clientSideVertices = vertices;
      subMesh->m_material = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      subMesh->m_material->m_color = Vec3();
      subMesh->m_material->m_color = Vec3();
      subMesh->m_material->GetRenderState()->cullMode = CullingType::TwoSided;

      subMesh->CalculateAABB();
      subMesh->ConstructFaces();

      camMeshComp->m_mesh->m_subMeshes.push_back(subMesh);
      camMeshComp->Init(false);
    }

  }

}
