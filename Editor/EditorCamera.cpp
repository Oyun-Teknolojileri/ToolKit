#include "EditorCamera.h"

#include "Global.h"
#include "Mesh.h"
#include "Primative.h"
#include "ResourceComponent.h"
#include "ToolKit.h"

#include <memory>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    EditorCamera::EditorCamera() { CreateGizmo(); }

    EditorCamera::EditorCamera(const EditorCamera* cam)
    {
      cam->CopyTo(this);

      CreateGizmo();
    }

    EditorCamera::~EditorCamera() {}

    Entity* EditorCamera::Copy() const
    {
      EditorCamera* cpy = new EditorCamera();
      Camera::CopyTo(cpy);
      cpy->CreateGizmo();
      return cpy;
    }

    void EditorCamera::GenerateFrustum()
    {
      // Line frustum.
      Vec3Array corners = {
          Vec3(-0.5f, 0.3f, -1.6f),
          Vec3(0.5f, 0.3f, -1.6f),
          Vec3(0.5f, -0.3f, -1.6f),
          Vec3(-0.5f, -0.3f, -1.6f),
      };

      // Below is a tricky frustum construction.
      // I am forcing to create triangles to use in ray/triangle intersection
      // in picking.
      // At the same time, not causing additional lines for frustom drawing.

      Vec3 eye;
      Vec3Array lines              = {eye,        corners[0], corners[1],
                                      corners[1], // Triangle widhout line.
                                      corners[1], corners[1],

                                      eye,        corners[1], corners[2], corners[2],
                                      corners[2], corners[2],

                                      eye,        corners[2], corners[3], corners[3],
                                      corners[3], corners[3],

                                      eye,        corners[3], corners[0], corners[0],
                                      corners[0], corners[0],

                                      corners[0], corners[1], corners[1], corners[2],
                                      corners[3], corners[3], corners[2], corners[3],
                                      corners[3], corners[0]};

      MeshComponentPtr camMeshComp = GetComponent<MeshComponent>();
      LineBatch frusta(lines, g_cameraGizmoColor, DrawType::Line);
      camMeshComp->SetMeshVal(
          frusta.GetComponent<MeshComponent>()->GetMeshVal());

      // Triangle part.
      VertexArray vertices;
      vertices.resize(3);

      vertices[0].pos               = Vec3(-0.3f, 0.35f, -1.6f);
      vertices[1].pos               = Vec3(0.3f, 0.35f, -1.6f);
      vertices[2].pos               = Vec3(0.0f, 0.65f, -1.6f);

      MeshPtr subMesh               = std::make_shared<Mesh>();
      subMesh->m_vertexCount        = (uint) vertices.size();
      subMesh->m_clientSideVertices = vertices;
      subMesh->m_material = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      subMesh->m_material->m_color                    = ZERO;
      subMesh->m_material->m_color                    = ZERO;
      subMesh->m_material->GetRenderState()->cullMode = CullingType::TwoSided;
      subMesh->ConstructFaces();

      camMeshComp->GetMeshVal()->m_subMeshes.push_back(subMesh);
      camMeshComp->GetMeshVal()->CalculateAABB();
      camMeshComp->Init(false);

      // Do not expose camera mesh component
      camMeshComp->ParamMesh().m_exposed = false;
    }

    void EditorCamera::CreateGizmo()
    {
      // Recreate frustum.
      AddComponent(new MeshComponent());
      GetMeshComponent()->SetCastShadowVal(false);
      GenerateFrustum();
    }

  } // namespace Editor
} // namespace ToolKit
