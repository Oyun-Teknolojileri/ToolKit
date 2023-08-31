/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "EditorCamera.h"

#include "App.h"

#include <Material.h>

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

    TKObjectPtr EditorCamera::Copy() const
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

      ParameterConstructor();
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
            if (Viewport* av = g_app->GetViewport(g_3dViewport))
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
