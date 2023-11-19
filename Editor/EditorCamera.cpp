/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "EditorCamera.h"

#include "App.h"

#include <Material.h>
#include <Mesh.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    TKDefineClass(EditorCamera, Camera);

    EditorCamera::EditorCamera() {}

    EditorCamera::EditorCamera(const EditorCamera* cam) { cam->CopyTo(this); }

    EditorCamera::~EditorCamera() {}

    void EditorCamera::NativeConstruct()
    {
      Super::NativeConstruct();
      CreateGizmo();
    }

    ObjectPtr EditorCamera::Copy() const
    {
      EditorCameraPtr cpy = MakeNewPtr<EditorCamera>();
      Camera::CopyTo(cpy.get());
      cpy->CreateGizmo();
      cpy->ParameterConstructor();

      return cpy;
    }

    void EditorCamera::PostDeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
    {
      Super::PostDeSerializeImp(info, parent);
      CreateGizmo();
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

                                      eye,        corners[1], corners[2], corners[2], corners[2], corners[2],

                                      eye,        corners[2], corners[3], corners[3], corners[3], corners[3],

                                      eye,        corners[3], corners[0], corners[0], corners[0], corners[0],

                                      corners[0], corners[1], corners[1], corners[2], corners[3], corners[3],
                                      corners[2], corners[3], corners[3], corners[0]};

      MeshComponentPtr camMeshComp = GetComponent<MeshComponent>();
      LineBatchPtr frusta          = MakeNewPtr<LineBatch>();
      frusta->Generate(lines, g_cameraGizmoColor, DrawType::Line);
      camMeshComp->SetMeshVal(frusta->GetComponent<MeshComponent>()->GetMeshVal());

      // Triangle part.
      VertexArray vertices;
      vertices.resize(3);

      vertices[0].pos                                 = Vec3(-0.3f, 0.35f, -1.6f);
      vertices[1].pos                                 = Vec3(0.3f, 0.35f, -1.6f);
      vertices[2].pos                                 = Vec3(0.0f, 0.65f, -1.6f);

      MeshPtr subMesh                                 = MakeNewPtr<Mesh>();
      subMesh->m_vertexCount                          = (uint) vertices.size();
      subMesh->m_clientSideVertices                   = vertices;
      subMesh->m_material                             = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
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
      if (GetComponent<MeshComponent>() == nullptr)
      {
        MeshComponentPtr meshCom = AddComponent<MeshComponent>();
        meshCom->SetCastShadowVal(false);
      }

      GenerateFrustum();
    }

    void EditorCamera::ParameterConstructor()
    {
      Super::ParameterConstructor();

      Poses_Define(
          [this]() -> void
          {
            if (Viewport* av = g_app->GetActiveViewport())
            {
              if (m_posessed)
              {
                av->AttachCamera(NULL_HANDLE);
                ParamPoses().m_name = "Poses";
              }
              else
              {
                av->AttachCamera(GetIdVal());
                ParamPoses().m_name = "Free";
              }

              m_posessed = !m_posessed;
            }
          },
          CameraCategory.Name,
          CameraCategory.Priority,
          true,
          true);
    }

  } // namespace Editor
} // namespace ToolKit
