#include "MaterialView.h"

#include "App.h"
#include "EditorViewport.h"

#include <DirectionComponent.h>

namespace ToolKit
{
  namespace Editor
  {

    // MaterialView
    //////////////////////////////////////////////////////////////////////////

    MaterialView::MaterialView() : View("Material View")
    {
      m_viewID   = 3;
      m_viewIcn  = UI::m_materialIcon;
      m_viewport = new PreviewViewport(300, 150);

      // Initialize ground entity
      Cube* ground = new Cube(Vec3(50, 0.01, 50));
      ground->GetMeshComponent()->GetMeshVal()->m_material =
          GetMaterialManager()->GetCopyOfSolidMaterial();
      m_viewport->GetScene()->AddEntity(ground);

      // Initialize preview entity (to show primitive meshes)
      Entity* previewEntity = new Entity;
      previewEntity->m_node->Translate(Vec3(0.0f, 1.5f, 0.0f));
      previewEntity->AddComponent(std::make_shared<MaterialComponent>());
      MeshComponentPtr meshComp = std::make_shared<MeshComponent>();
      previewEntity->AddComponent(meshComp);
      m_viewport->GetScene()->AddEntity(previewEntity);

      // Merge ShaderBall scene into preview
      ScenePtr shaderBallScene = GetSceneManager()->Create<Scene>(
          ResourcePath(true) + "/Scenes/ShaderBall.scene");
      m_viewport->GetScene()->Merge(shaderBallScene);

      ResetCamera();
    }
    MaterialView::~MaterialView()
    {
      SafeDel(m_viewport);
    }
    void MaterialView::SetMaterial(MaterialPtr mat)
    {
      m_mat = mat;
    }
    void MaterialView::ResetCamera()
    {
      m_viewport->GetCamera()->m_node->SetTranslation(Vec3(0, 2.0, 5));
      m_viewport->GetCamera()->GetComponent<DirectionComponent>()->LookAt(
          Vec3(0));
    }

    void MaterialView::updatePreviewScene()
    {
      if (isMeshChanged)
      {
        MeshComponentPtr newMeshComp = std::make_shared<MeshComponent>();
        switch (m_activeObjectIndx)
        {
        case 0:
          Sphere::Generate(newMeshComp, 1.5f);
          break;
        case 1:
          Cube::Generate(newMeshComp, Vec3(3.0));
          break;
        }
        Entity* primNtt = m_viewport->GetScene()->GetEntities()[1];
        primNtt->RemoveComponent(primNtt->GetMeshComponent()->m_id);
        primNtt->AddComponent(newMeshComp);
        isMeshChanged = false;
      }

      bool primEntityVis = m_activeObjectIndx == 2 ? false : true;
      for (uint32_t i = 1; i < m_viewport->GetScene()->GetEntities().size();
           i++)
      {
        Entity* ntt = m_viewport->GetScene()->GetEntities()[i];
        ntt->SetVisibleVal(!primEntityVis);
        if (ntt->GetMaterialComponent())
        {
          ntt->GetMaterialComponent()->SetMaterialVal(m_mat);
        }
      }
      m_viewport->GetScene()->GetEntities()[1]->SetVisibleVal(primEntityVis);
    }

    void MaterialView::Show()
    {
      if (!m_mat)
      {
        ImGui::Text("Select a material");
        return;
      }

      String name, ext;
      DecomposePath(m_mat->GetFile(), nullptr, &name, &ext);

      ImGui::Text("Material: %s%s", name.c_str(), ext.c_str());
      ImGui::Separator();

      if (ImGui::CollapsingHeader("Material Preview",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 300.0f) /
                             2.0f);
        updatePreviewScene();
        if (UI::ImageButtonDecorless(
                UI::m_cameraIcon->m_textureId, Vec2(16.0f), false))
        {
          ResetCamera();
        }
        ImGui::SameLine();
        m_viewport->Update(g_app->GetDeltaTime());
        m_viewport->Show();
        ImGui::SameLine();
        ImGui::BeginGroup();
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_sphereIcon),
                               ImVec2(16, 16)))
        {
          m_activeObjectIndx = 0;
          isMeshChanged      = true;
        }
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_cubeIcon),
                               ImVec2(16, 16)))
        {
          m_activeObjectIndx = 1;
          isMeshChanged      = true;
        }
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_shaderBallIcon),
                               ImVec2(16, 16)))
        {
          m_activeObjectIndx = 2;
          isMeshChanged      = true;
        }
        ImGui::EndGroup();
      }

      auto updateThumbFn = [this]() -> void {
        DirectoryEntry dirEnt(m_mat->GetFile());
        g_app->m_thumbnailCache.erase(m_mat->GetFile());

        dirEnt.GenerateThumbnail();
        m_mat->m_dirty = true;
      };

      if (ImGui::CollapsingHeader("Shaders"))
      {
        ImGui::LabelText("##vertShader", "Vertex Shader: ");
        DropZone(UI::m_codeIcon->m_textureId,
                 m_mat->m_vertexShader->GetFile(),
                 [this, &updateThumbFn](const DirectoryEntry& dirEnt) -> void {
                   if (strcmp(dirEnt.m_ext.c_str(), ".shader") != 0)
                   {
                     g_app->m_statusMsg = "An imported shader file expected!";
                     return;
                   }
                   m_mat->m_vertexShader =
                       GetShaderManager()->Create<Shader>(dirEnt.GetFullPath());
                   m_mat->m_vertexShader->Init();
                   updateThumbFn();
                 });

        ImGui::LabelText("##fragShader", "Fragment Shader: ");
        DropZone(UI::m_codeIcon->m_textureId,
                 m_mat->m_fragmentShader->GetFile(),
                 [this, &updateThumbFn](const DirectoryEntry& dirEnt) -> void {
                   m_mat->m_fragmentShader =
                       GetShaderManager()->Create<Shader>(dirEnt.GetFullPath());
                   m_mat->m_fragmentShader->Init();
                   updateThumbFn();
                 });
      }

      if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
      {
        ImGui::LabelText("##diffTexture", "Diffuse Texture: ");
        String target = GetPathSeparatorAsStr();
        if (m_mat->m_diffuseTexture)
        {
          target = m_mat->m_diffuseTexture->GetFile();
        }

        DropZone(
            UI::m_imageIcon->m_textureId,
            target,
            [this, &updateThumbFn](const DirectoryEntry& dirEnt) -> void {
              // Switch from solid color Material to default for texturing.
              if (m_mat->m_diffuseTexture == nullptr)
              {
                m_mat->m_fragmentShader = GetShaderManager()->Create<Shader>(
                    ShaderPath("defaultFragment.shader", true));
                m_mat->m_fragmentShader->Init();
              }
              m_mat->m_diffuseTexture =
                  GetTextureManager()->Create<Texture>(dirEnt.GetFullPath());
              m_mat->m_diffuseTexture->Init();
              updateThumbFn();
            });
      }

      if (ImGui::CollapsingHeader("Render States",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {
        Vec4 col = Vec4(m_mat->m_color, m_mat->m_alpha);
        if (ImGui::ColorEdit4(
                "Material Color##1", &col.x, ImGuiColorEditFlags_NoLabel))
        {
          m_mat->m_color = col.xyz;
          m_mat->m_alpha = col.a;
          updateThumbFn();
        }

        int cullMode = static_cast<int>(m_mat->GetRenderState()->cullMode);
        if (ImGui::Combo("Cull mode", &cullMode, "Two Sided\0Front\0Back"))
        {
          m_mat->GetRenderState()->cullMode = (CullingType) cullMode;
          m_mat->m_dirty                    = true;
        }

        int blendMode =
            static_cast<int>(m_mat->GetRenderState()->blendFunction);
        if (ImGui::Combo(
                "Blend mode", &blendMode, "None\0Alpha Blending\0Alpha Mask"))
        {
          m_mat->GetRenderState()->blendFunction = (BlendFunction) blendMode;
          m_mat->m_dirty                         = true;
        }

        int drawType = -1;
        switch (m_mat->GetRenderState()->drawType)
        {
        case DrawType::Triangle:
          drawType = 0;
          break;
        case DrawType::Line:
          drawType = 1;
          break;
        case DrawType::LineStrip:
          drawType = 2;
          break;
        case DrawType::LineLoop:
          drawType = 3;
          break;
        case DrawType::Point:
          drawType = 4;
          break;
        }

        if (ImGui::Combo("Draw mode",
                         &drawType,
                         "Triangle\0Line\0Line Strip\0Line Loop\0Point"))
        {
          switch (drawType)
          {
          case 0:
            m_mat->GetRenderState()->drawType = DrawType::Triangle;
            break;
          case 1:
            m_mat->GetRenderState()->drawType = DrawType::Line;
            break;
          case 2:
            m_mat->GetRenderState()->drawType = DrawType::LineStrip;
            break;
          case 3:
            m_mat->GetRenderState()->drawType = DrawType::LineLoop;
            break;
          case 4:
            m_mat->GetRenderState()->drawType = DrawType::Point;
            break;
          }

          m_mat->m_dirty = true;
        }

        bool depthTest = m_mat->GetRenderState()->depthTestEnabled;
        if (ImGui::Checkbox("Enable depth test", &depthTest))
        {
          m_mat->GetRenderState()->depthTestEnabled = depthTest;
          m_mat->m_dirty                            = true;
        }

        bool AOInUse = m_mat->GetRenderState()->AOInUse;
        if (ImGui::Checkbox("Ambient Occlusion", &AOInUse))
        {
          m_mat->GetRenderState()->AOInUse = AOInUse;
          m_mat->m_dirty                   = true;
        }

        bool isUnlit = m_mat->GetRenderState()->isUnlit;
        if (ImGui::Checkbox("Unlit", &isUnlit))
        {
          m_mat->GetRenderState()->isUnlit = isUnlit;
          m_mat->m_dirty                   = true;
        }

        if (m_mat->GetRenderState()->blendFunction == BlendFunction::ALPHA_MASK)
        {
          float alphaMaskTreshold = m_mat->GetRenderState()->alphaMaskTreshold;
          if (ImGui::DragFloat("Alpha Mask Treshold",
                               &alphaMaskTreshold,
                               0.001f,
                               0.0f,
                               1.0f,
                               "%.3f"))
          {
            m_mat->GetRenderState()->alphaMaskTreshold = alphaMaskTreshold;
            m_mat->m_dirty                             = true;
          }
        }
      }
    }
  } // namespace Editor
} // namespace ToolKit
