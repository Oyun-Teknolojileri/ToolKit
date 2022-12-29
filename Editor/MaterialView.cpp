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
        g_app->m_thumbnailCache.erase(m_mat->GetFile());

        dirEnt.GenerateThumbnail();
        m_mat->m_dirty = true;
      };

      int matType     = (int) m_mat->m_materialType;
      int currentType = matType;
      if (ImGui::Combo("Material Type", &matType, "Phong\0PBR\0Custom"))
      {
        if (matType != currentType)
        {
          m_mat->m_materialType = (MaterialType) matType;
          m_mat->SetDefaultMaterialTypeShaders();
          m_mat->m_dirty        = true;
        }
      }

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
        ImGui::LabelText("##diffTexture", "Diffuse Texture: ");
        String target = GetPathSeparatorAsStr();
        if (m_mat->m_diffuseTexture)
        {
          target = m_mat->m_diffuseTexture->GetFile();
        }

        DropZone(UI::m_imageIcon->m_textureId,
                 target,
                 [this, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
                 {
                   m_mat->m_diffuseTexture =
                       GetTextureManager()->Create<Texture>(
                           dirEnt.GetFullPath());
                   m_mat->m_diffuseTexture->Init();
                   updateThumbFn();
                 });

        // Display emissive color multiplier if fragment is emissive
        for (Uniform u : m_mat->m_fragmentShader->m_uniforms)
        {
          if (u == Uniform::EMISSIVE_COLOR)
          {
            ImGui::LabelText("##emissiveTexture", "Emissive Texture: ");
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
              if (UI::ImageButtonDecorless(UI::m_closeIcon->m_textureId,
                                           Vec2(16.0f, 16.0f),
                                           false))
              {
                m_mat->m_emissiveTexture = nullptr;
                m_mat->m_dirty           = true;
              }
            }
          }
        }
      }

      if (ImGui::CollapsingHeader("Render States",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {
        Vec4 col = Vec4(m_mat->m_color, m_mat->m_alpha);
        if (ImGui::ColorEdit4("Material Color##1",
                              &col.x,
                              ImGuiColorEditFlags_NoLabel))
        {
          m_mat->m_color = col.xyz;
          m_mat->m_alpha = col.a;
          updateThumbFn();
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

        if (m_mat->m_materialType == MaterialType::PBR)
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

          if (ImGui::DragFloat("Roughess",
                               &(m_mat->m_roughness),
                               0.001f,
                               0.0f,
                               1.0f,
                               "%.3f"))
          {
            updateThumbFn();
          }
        }

        int cullMode = (int) m_mat->GetRenderState()->cullMode;
        if (ImGui::Combo("Cull mode", &cullMode, "Two Sided\0Front\0Back"))
        {
          m_mat->GetRenderState()->cullMode = (CullingType) cullMode;
          m_mat->m_dirty                    = true;
        }

        int blendMode = (int) m_mat->GetRenderState()->blendFunction;
        if (ImGui::Combo("Blend mode",
                         &blendMode,
                         "None\0Alpha Blending\0Alpha Mask"))
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

        bool useForwardPath = m_mat->GetRenderState()->useForwardPath;
        if (ImGui::Checkbox("Use Forward Path", &useForwardPath))
        {
          m_mat->GetRenderState()->useForwardPath = useForwardPath;
          m_mat->m_dirty                          = true;
        }

        bool isColorMaterial = m_mat->GetRenderState()->isColorMaterial;
        if (ImGui::Checkbox("Color Material", &isColorMaterial))
        {
          m_mat->GetRenderState()->isColorMaterial = isColorMaterial;
          m_mat->m_dirty                           = true;
        }

        if (m_mat->GetRenderState()->blendFunction == BlendFunction::ALPHA_MASK)
        {
          float alphaMaskTreshold = m_mat->GetRenderState()->alphaMaskTreshold;
          if (ImGui::DragFloat("Alpha Mask Threshold",
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
