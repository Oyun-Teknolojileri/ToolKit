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
      m_viewID              = 3;
      m_viewIcn             = UI::m_materialIcon;
      m_viewport            = new PreviewViewport(300u, 150u);

      // Initialize ground entity
      MaterialPtr groundMat = GetMaterialManager()->GetCopyOfDefaultMaterial();
      groundMat->m_diffuseTexture = GetTextureManager()->Create<Texture>(
          TexturePath("checkerBoard.png", true));

      Cube* ground = new Cube(Vec3(20.0f, 0.01f, 20.0f));
      ground->GetMeshComponent()->SetCastShadowVal(false);
      ground->GetMeshComponent()->GetMeshVal()->m_material = groundMat;

      ScenePtr scene = m_viewport->GetScene();
      scene->AddEntity(ground);

      // Initialize preview entity (to show primitive meshes)
      Entity* previewEntity = new Entity;
      previewEntity->m_node->Translate(Vec3(0.0f, 1.5f, 0.0f));
      previewEntity->AddComponent(std::make_shared<MaterialComponent>());
      MeshComponentPtr meshComp = std::make_shared<MeshComponent>();
      previewEntity->AddComponent(meshComp);
      scene->AddEntity(previewEntity);

      // Merge ShaderBall scene into preview
      ScenePtr shaderBallScene =
          GetSceneManager()->Create<Scene>(ScenePath("ShaderBall.scene", true));
      scene->Merge(shaderBallScene);

      GradientSky* sky = new GradientSky();
      sky->SetIlluminateVal(true);
      sky->ReInit();
      scene->AddEntity(sky);

      ResetCamera();
    }

    MaterialView::~MaterialView() { SafeDel(m_viewport); }

    void MaterialView::SetMaterial(MaterialPtr mat) { m_mat = mat; }

    void MaterialView::ResetCamera() { m_viewport->ResetCamera(); }

    void MaterialView::UpdatePreviewScene()
    {
      EntityRawPtrArray& entities = m_viewport->GetScene()->AccessEntityArray();
      Entity* primNtt             = nullptr;
      if (entities.size() > 1u)
      {
        primNtt = entities[1];
      }

      if (m_isMeshChanged)
      {
        MeshComponentPtr newMeshComp = std::make_shared<MeshComponent>();
        switch (m_activeObjectIndx)
        {
        case 0:
          Sphere::Generate(newMeshComp, 1.35f);
          primNtt->m_node->SetTranslation(Vec3(0.0f, 1.35f, 0.0f));
          break;
        case 1:
          Cube::Generate(newMeshComp, Vec3(2.3f));
          primNtt->m_node->SetTranslation(Vec3(0.0f, 2.3f * 0.5f, 0.0f));
          break;
        default:
          primNtt->m_node->SetTranslation(Vec3(0.0f));
        }

        if (primNtt)
        {
          primNtt->RemoveComponent(primNtt->GetMeshComponent()->m_id);
          primNtt->AddComponent(newMeshComp);
        }

        m_isMeshChanged = false;
      }

      bool primEntityVis = m_activeObjectIndx == 2 ? false : true;
      for (uint i = 1; i < entities.size(); i++)
      {
        Entity* ntt = entities[i];
        if (!ntt->IsSkyInstance())
        {
          ntt->SetVisibleVal(!primEntityVis);
          if (ntt->GetMaterialComponent())
          {
            ntt->GetMaterialComponent()->SetMaterialVal(m_mat);
          }
        }
      }

      if (primNtt)
      {
        primNtt->SetVisibleVal(primEntityVis);
      }
    }

    void MaterialView::Show()
    {
      if (!m_mat)
      {
        ImGui::Text("\nSelect a material");
        return;
      }

      String name, ext;
      DecomposePath(m_mat->GetFile(), nullptr, &name, &ext);

      ImGui::Text("\nMaterial: %s%s", name.c_str(), ext.c_str());
      // 0th slot was pbr and removed, this is why we are doing -1 adjustments.
      int matType     = glm::clamp((int) m_mat->m_materialType, 1, 2) - 1;
      int currentType = matType;
      if (ImGui::Combo("Material Type", &matType, "PBR\0Custom"))
      {
        if (matType != currentType)
        {
          m_mat->m_materialType = (MaterialType) (matType + 1);
          m_mat->SetDefaultMaterialTypeShaders();
          m_mat->m_dirty = true;
        }
      }
      ImGui::Separator();

      if (ImGui::CollapsingHeader("Material Preview",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {
        static const ImVec2 iconSize = ImVec2(16.0f, 16.0f);
        const ImVec2 spacing         = ImGui::GetStyle().ItemSpacing;
        UpdatePreviewScene();
        if (UI::ImageButtonDecorless(UI::m_cameraIcon->m_textureId,
                                     iconSize,
                                     false))
        {
          ResetCamera();
        }

        const ImVec2 viewportSize = ImVec2(ImGui::GetContentRegionAvail().x -
                                               iconSize.x - 9.0f * spacing.x,
                                           150.0f);
        if (viewportSize.x > 1 && viewportSize.y > 1)
        {
          ImGui::SameLine();
          m_viewport->ResizeWindow((uint) viewportSize.x,
                                   (uint) viewportSize.y);
          m_viewport->Update(g_app->GetDeltaTime());
          m_viewport->Show();
          ImGui::SameLine();
          ImGui::BeginGroup();

          auto setIconFn = [this](TexturePtr icon, uint id) -> void
          {
            if (ImGui::ImageButton(Convert2ImGuiTexture(icon), iconSize))
            {
              m_activeObjectIndx = id;
              m_isMeshChanged    = true;
            }
          };

          setIconFn(UI::m_sphereIcon, 0u);
          setIconFn(UI::m_cubeIcon, 1u);
          setIconFn(UI::m_shaderBallIcon, 2u);

          ImGui::EndGroup();
        }
      }

      auto updateThumbFn = [this]() -> void
      {
        DirectoryEntry dirEnt(m_mat->GetFile());
        g_app->m_thumbnailManager.UpdateThumbnail(dirEnt);
        m_mat->m_dirty = true;
      };

      if (m_mat->m_materialType == MaterialType::Custom)
      {
        if (ImGui::CollapsingHeader("Shaders"))
        {
          ImGui::LabelText("##vertShader", "Vertex Shader: ");
          DropZone(UI::m_codeIcon->m_textureId,
                   m_mat->m_vertexShader->GetFile(),
                   [this, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
                   {
                     if (strcmp(dirEnt.m_ext.c_str(), ".shader") != 0)
                     {
                       g_app->m_statusMsg = "Failed. Shader expected.";
                       return;
                     }
                     m_mat->m_vertexShader = GetShaderManager()->Create<Shader>(
                         dirEnt.GetFullPath());
                     m_mat->m_vertexShader->Init();
                     updateThumbFn();
                   });

          ImGui::LabelText("##fragShader", "Fragment Shader: ");
          DropZone(UI::m_codeIcon->m_textureId,
                   m_mat->m_fragmentShader->GetFile(),
                   [this, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
                   {
                     m_mat->m_fragmentShader =
                         GetShaderManager()->Create<Shader>(
                             dirEnt.GetFullPath());
                     m_mat->m_fragmentShader->Init();
                     updateThumbFn();
                   });
        }
      }

      if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
      {
        uint textureIndx = 0;
        for (Uniform u : m_mat->m_fragmentShader->m_uniforms)
        {
          switch (u)
          {
          case Uniform::DIFFUSE_TEXTURE_IN_USE:
          {
            ImGui::LabelText("##diffTexture", "Diffuse");
            String target = GetPathSeparatorAsStr();
            if (m_mat->m_diffuseTexture)
            {
              target = m_mat->m_diffuseTexture->GetFile();
            }

            DropZone(
                UI::m_imageIcon->m_textureId,
                target,
                [this, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
                {
                  m_mat->m_diffuseTexture =
                      GetTextureManager()->Create<Texture>(
                          dirEnt.GetFullPath());
                  m_mat->m_diffuseTexture->Init();
                  updateThumbFn();
                });

            if (m_mat->m_diffuseTexture)
            {
              ImGui::SameLine();
              ImGui::PushID(textureIndx++);
              if (UI::ImageButtonDecorless(UI::m_closeIcon->m_textureId,
                                           Vec2(16.0f, 16.0f),
                                           false))
              {
                m_mat->m_diffuseTexture = nullptr;
                updateThumbFn();
              }
              ImGui::PopID();
            }
          }
          break;
          case Uniform::EMISSIVE_TEXTURE_IN_USE:
          {
            ImGui::LabelText("##emissiveTexture", "Emissive");
            String target = GetPathSeparatorAsStr();
            if (m_mat->m_emissiveTexture)
            {
              target = m_mat->m_emissiveTexture->GetFile();
            }

            DropZone(
                UI::m_imageIcon->m_textureId,
                target,
                [this, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
                {
                  m_mat->m_emissiveTexture =
                      GetTextureManager()->Create<Texture>(
                          dirEnt.GetFullPath());
                  m_mat->m_emissiveTexture->Init();
                  updateThumbFn();
                });

            if (m_mat->m_emissiveTexture)
            {
              ImGui::SameLine();
              ImGui::PushID(textureIndx++);
              if (UI::ImageButtonDecorless(UI::m_closeIcon->m_textureId,
                                           Vec2(16.0f, 16.0f),
                                           false))
              {
                m_mat->m_emissiveTexture = nullptr;
                updateThumbFn();
              }
              ImGui::PopID();
            }
          }
          break;
          case Uniform::NORMAL_MAP_IN_USE:
          {
            ImGui::LabelText("##normalMap", "Normal Map");
            String target = GetPathSeparatorAsStr();
            if (m_mat->m_normalMap)
            {
              target = m_mat->m_normalMap->GetFile();
            }

            DropZone(
                UI::m_imageIcon->m_textureId,
                target,
                [this, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
                {
                  m_mat->m_normalMap = GetTextureManager()->Create<Texture>(
                      dirEnt.GetFullPath());
                  m_mat->m_normalMap->Init();
                  updateThumbFn();
                });

            if (m_mat->m_normalMap)
            {
              ImGui::SameLine();
              ImGui::PushID(textureIndx++);
              if (UI::ImageButtonDecorless(UI::m_closeIcon->m_textureId,
                                           Vec2(16.0f, 16.0f),
                                           false))
              {
                m_mat->m_normalMap = nullptr;
                updateThumbFn();
              }
              ImGui::PopID();
            }
          }
          break;
          case Uniform::METALLIC_ROUGHNESS_TEXTURE_IN_USE:
          {
            ImGui::LabelText("##metallicRoughnessTexture",
                             "Metallic Roughness");
            String target = GetPathSeparatorAsStr();
            if (m_mat->m_metallicRoughnessTexture)
            {
              target = m_mat->m_metallicRoughnessTexture->GetFile();
            }

            DropZone(
                UI::m_imageIcon->m_textureId,
                target,
                [this, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
                {
                  m_mat->m_metallicRoughnessTexture =
                      GetTextureManager()->Create<Texture>(
                          dirEnt.GetFullPath());
                  m_mat->m_metallicRoughnessTexture->Init();
                  updateThumbFn();
                });

            if (m_mat->m_metallicRoughnessTexture)
            {
              ImGui::SameLine();
              ImGui::PushID(textureIndx++);
              if (UI::ImageButtonDecorless(UI::m_closeIcon->m_textureId,
                                           Vec2(16.0f, 16.0f),
                                           false))
              {
                m_mat->m_metallicRoughnessTexture = nullptr;
                updateThumbFn();
              }
              ImGui::PopID();
            }
          }
          break;
          }
        }
      }

      RenderState* renderState = m_mat->GetRenderState();

      if (ImGui::CollapsingHeader("Render States",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {
        if (m_mat->m_diffuseTexture == nullptr)
        {
          if (ImGui::ColorEdit3("Diffuse Color", &m_mat->m_color.x))
          {
            updateThumbFn();
          }
          if (ImGui::DragFloat("Alpha",
                               &m_mat->m_alpha,
                               1.0f / 256.0f,
                               0.0f,
                               1.0f))
          {
            bool isForward = m_mat->m_alpha < 0.99f;
            renderState->blendFunction =
                isForward ? BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA
                          : BlendFunction::NONE;

            renderState->useForwardPath = isForward;
            updateThumbFn();
          }
        }
        // Display emissive color multiplier if fragment is emissive
        for (Uniform u : m_mat->m_fragmentShader->m_uniforms)
        {
          if (u == Uniform::EMISSIVE_COLOR &&
              m_mat->m_emissiveTexture == nullptr)
          {
            if (ImGui::ColorEdit3("Emissive Color Multiplier##1",
                                  &m_mat->m_emissiveColor.x,
                                  ImGuiColorEditFlags_HDR |
                                      ImGuiColorEditFlags_NoLabel |
                                      ImGuiColorEditFlags_Float))
            {
              updateThumbFn();
            }
            ImGui::SameLine();
            ImGui::Text("Emissive Color");
          }
        }

        if (m_mat->m_materialType == MaterialType::PBR &&
            m_mat->m_metallicRoughnessTexture == nullptr)
        {
          if (ImGui::DragFloat("Metallic",
                               &(m_mat->m_metallic),
                               0.001f,
                               0.0f,
                               1.0f,
                               "%.3f"))
          {
            updateThumbFn();
          }

          if (ImGui::DragFloat("Roughness",
                               &(m_mat->m_roughness),
                               0.001f,
                               0.0f,
                               1.0f,
                               "%.3f"))
          {
            updateThumbFn();
          }
        }

        int cullMode = (int) renderState->cullMode;
        if (ImGui::Combo("Cull mode", &cullMode, "Two Sided\0Front\0Back"))
        {
          renderState->cullMode = (CullingType) cullMode;
          updateThumbFn();
        }

        int blendMode = (int) renderState->blendFunction;
        if (ImGui::Combo("Blend mode",
                         &blendMode,
                         "None\0Alpha Blending\0Alpha Mask"))
        {
          renderState->blendFunction = (BlendFunction) blendMode;
          if (renderState->blendFunction == BlendFunction::NONE)
          {
            m_mat->m_alpha = 1.0f;
          }

          renderState->useForwardPath =
              renderState->blendFunction ==
              BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;

          m_mat->m_dirty = true;

          updateThumbFn();
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
            renderState->drawType = DrawType::Triangle;
            break;
          case 1:
            renderState->drawType = DrawType::Line;
            break;
          case 2:
            renderState->drawType = DrawType::LineStrip;
            break;
          case 3:
            renderState->drawType = DrawType::LineLoop;
            break;
          case 4:
            renderState->drawType = DrawType::Point;
            break;
          }

          updateThumbFn();
        }

        if (renderState->blendFunction == BlendFunction::ALPHA_MASK)
        {
          float alphaMaskTreshold = renderState->alphaMaskTreshold;
          if (ImGui::DragFloat("Alpha Mask Threshold",
                               &alphaMaskTreshold,
                               0.001f,
                               0.0f,
                               1.0f,
                               "%.3f"))
          {
            renderState->alphaMaskTreshold = alphaMaskTreshold;
            updateThumbFn();
          }
        }

        for (int i = 0; i < 3; i++)
        {
          ImGui::Spacing();
        }
      }
    }

  } // namespace Editor
} // namespace ToolKit