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

#include "MaterialView.h"

#include "App.h"
#include "DirectionComponent.h"
#include "EditorViewport.h"
#include "GradientSky.h"

namespace ToolKit
{
  namespace Editor
  {

    // MaterialView
    //////////////////////////////////////////////////////////////////////////

    MaterialView::MaterialView() : View("Material View")
    {
      m_viewID                    = 3;
      m_viewIcn                   = UI::m_materialIcon;
      m_viewport                  = new PreviewViewport(300u, 150u);

      // Initialize ground entity
      MaterialPtr groundMat       = GetMaterialManager()->GetCopyOfDefaultMaterial();
      groundMat->m_diffuseTexture = GetTextureManager()->Create<Texture>(TexturePath("checkerBoard.png", true));

      Cube* ground                = new Cube(Vec3(20.0f, 0.01f, 20.0f));
      ground->GetMeshComponent()->SetCastShadowVal(false);
      ground->GetMeshComponent()->GetMeshVal()->m_material = groundMat;

      ScenePtr scene                                       = m_viewport->GetScene();
      scene->AddEntity(ground);

      // Initialize preview entity (to show primitive meshes)
      Entity* previewEntity = new Entity;
      previewEntity->m_node->Translate(Vec3(0.0f, 1.5f, 0.0f));
      previewEntity->AddComponent(std::make_shared<MaterialComponent>());
      MeshComponentPtr meshComp = std::make_shared<MeshComponent>();
      previewEntity->AddComponent(meshComp);
      scene->AddEntity(previewEntity);

      // Merge ShaderBall scene into preview
      ScenePtr shaderBallScene = GetSceneManager()->Create<Scene>(ScenePath("ShaderBall.scene", true));
      scene->Merge(shaderBallScene);

      ResetCamera();
    }

    MaterialView::~MaterialView() { SafeDel(m_viewport); }

    void MaterialView::SetSelectedMaterial(MaterialPtr m_mat)
    {
      auto find = std::find(m_materials.cbegin(), m_materials.cend(), m_mat);
      if (find != m_materials.cend())
      {
        m_currentMaterialIndex = int(find - m_materials.cbegin());
      }
    }

    void MaterialView::SetMaterials(const MaterialPtrArray& mat) { m_materials = mat; }

    void MaterialView::ResetCamera() { m_viewport->ResetCamera(); }

    void MaterialView::UpdatePreviewScene()
    {
      const EntityRawPtrArray& entities = m_viewport->GetScene()->GetEntities();
      Entity* primNtt                   = nullptr;
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
          if (ntt->GetMaterialComponent() && m_materials.size() > 0)
          {
            m_currentMaterialIndex = glm::clamp(m_currentMaterialIndex, 0, (int) m_materials.size());

            ntt->GetMaterialComponent()->SetFirstMaterial(m_materials[m_currentMaterialIndex]);
          }
        }
      }

      if (primNtt)
      {
        primNtt->SetVisibleVal(primEntityVis);
      }
    }

    int DrawTypeToInt(DrawType drawType)
    {
      switch (drawType)
      {
      case DrawType::Triangle:
        return 0;
      case DrawType::Line:
        return 1;
      case DrawType::LineStrip:
        return 2;
      case DrawType::LineLoop:
        return 3;
      case DrawType::Point:
        return 4;
      default:
        return -1;
      }
    }

    DrawType IntToDrawType(int drawType)
    {
      switch (drawType)
      {
      case 0:
        return DrawType::Triangle;
      case 1:
        return DrawType::Line;
      case 2:
        return DrawType::LineStrip;
      case 3:
        return DrawType::LineLoop;
      case 4:
        return DrawType::Point;
      default:
        return DrawType::Triangle;
      }
    }

    void MaterialView::ShowMaterial(MaterialPtr mat)
    {
      if (!mat)
      {
        ImGui::Text("\nSelect a material");
        return;
      }

      String name, ext, path;
      DecomposePath(mat->GetFile(), &path, &name, &ext);
      UI::HeaderText(name.c_str());
      GetFileManager()->GetRelativeResourcesPath(path);
      UI::HelpMarker(TKLoc, path.c_str());

      // 0th slot was pbr and removed, this is why we are doing -1 adjustments.
      int matType     = glm::clamp((int) mat->m_materialType, 1, 2) - 1;
      int currentType = matType;
      if (ImGui::Combo("Material Type", &matType, "PBR\0Custom"))
      {
        if (matType != currentType)
        {
          mat->m_materialType = (MaterialType) (matType + 1);
          mat->SetDefaultMaterialTypeShaders();
          mat->m_dirty = true;
        }
      }
      ImGui::Separator();

      if (ImGui::CollapsingHeader("Material Preview", ImGuiTreeNodeFlags_DefaultOpen))
      {
        static const ImVec2 iconSize = ImVec2(16.0f, 16.0f);
        const ImVec2 spacing         = ImGui::GetStyle().ItemSpacing;
        UpdatePreviewScene();
        if (UI::ImageButtonDecorless(UI::m_cameraIcon->m_textureId, iconSize, false))
        {
          ResetCamera();
        }

        const ImVec2 viewportSize = ImVec2(ImGui::GetContentRegionAvail().x - iconSize.x - 9.0f * spacing.x, 130.0f);
        if (viewportSize.x > 1 && viewportSize.y > 1)
        {
          ImGui::SameLine();
          m_viewport->m_isTempView = m_isTempView;
          m_viewport->ResizeWindow((uint) viewportSize.x, (uint) viewportSize.y);
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
        ImGui::Spacing();
      }

      auto updateThumbFn = [this, mat]() -> void
      {
        DirectoryEntry dirEnt(mat->GetFile());
        g_app->m_thumbnailManager.UpdateThumbnail(dirEnt);
        mat->m_dirty = true;
      };

      if (mat->m_materialType == MaterialType::Custom)
      {
        if (ImGui::CollapsingHeader("Shaders"))
        {
          ImGui::BeginGroup();
          ImGui::LabelText("##vertShader", "Vertex Shader: ");
          DropZone(UI::m_codeIcon->m_textureId,
                   mat->m_vertexShader->GetFile(),
                   [this, mat, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
                   {
                     if (strcmp(dirEnt.m_ext.c_str(), ".shader") != 0)
                     {
                       g_app->m_statusMsg = "Failed. Shader expected.";
                       return;
                     }
                     mat->m_vertexShader = GetShaderManager()->Create<Shader>(dirEnt.GetFullPath());
                     mat->m_vertexShader->Init();
                     updateThumbFn();
                   });
          ImGui::EndGroup();

          ImGui::SameLine();

          ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 35.0f);

          ImGui::BeginGroup();
          ImGui::LabelText("##fragShader", "Fragment Shader: ");
          DropZone(UI::m_codeIcon->m_textureId,
                   mat->m_fragmentShader->GetFile(),
                   [this, mat, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
                   {
                     mat->m_fragmentShader = GetShaderManager()->Create<Shader>(dirEnt.GetFullPath());
                     mat->m_fragmentShader->Init();
                     updateThumbFn();
                   });
          ImGui::EndGroup();
        }
      }

      if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
      {
        uint textureIndx = 0;
        auto exposeTextureFn =
            [this, &textureIndx, &updateThumbFn](TexturePtr& texture, StringView sId, StringView label) -> void
        {
          ImGui::BeginGroup();

          ImGui::LabelText(sId.data(), label.data());
          String target = GetPathSeparatorAsStr();
          if (texture)
          {
            target = texture->GetFile();
          }

          DropZone(UI::m_imageIcon->m_textureId,
                   target,
                   [&texture, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
                   {
                     texture = GetTextureManager()->Create<Texture>(dirEnt.GetFullPath());
                     texture->Init();
                     updateThumbFn();
                   });

          if (texture)
          {
            ImGui::SameLine();
            ImGui::PushID(textureIndx++);
            if (UI::ImageButtonDecorless(UI::m_closeIcon->m_textureId, Vec2(16.0f, 16.0f), false))
            {
              texture = nullptr;
              updateThumbFn();
            }
            ImGui::PopID();
          }

          ImGui::EndGroup();
        };

        exposeTextureFn(mat->m_diffuseTexture, "##diffTexture", "Diffuse");
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 35.0f);
        exposeTextureFn(mat->m_emissiveTexture, "##emissiveTexture", "Emissive");
        exposeTextureFn(mat->m_normalMap, "##normalMap", "Normal Map");
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 35.0f);
        exposeTextureFn(mat->m_metallicRoughnessTexture, "##metallicRoughnessTexture", "Metallic Roughness");
      }

      RenderState* renderState = mat->GetRenderState();

      if (ImGui::CollapsingHeader("Render States", ImGuiTreeNodeFlags_DefaultOpen))
      {
        if (mat->m_diffuseTexture == nullptr)
        {
          if (ImGui::ColorEdit3("Diffuse Color", &mat->m_color.x))
          {
            updateThumbFn();
          }
          if (ImGui::DragFloat("Alpha", &mat->GetAlpha(), 1.0f / 256.0f, 0.0f, 1.0f))
          {
            bool isForward             = mat->GetAlpha() < 0.99f;
            renderState->blendFunction = isForward ? BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA : BlendFunction::NONE;

            updateThumbFn();
          }
        }

        if (mat->m_emissiveTexture == nullptr)
        {
          if (ImGui::ColorEdit3("Emissive Color Multiplier##1",
                                &mat->m_emissiveColor.x,
                                ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_Float))
          {
            updateThumbFn();
          }
          ImGui::SameLine();
          ImGui::Text("Emissive Color");
        }

        if (mat->IsPBR() && mat->m_metallicRoughnessTexture == nullptr)
        {
          if (ImGui::DragFloat("Metallic", &(mat->m_metallic), 0.001f, 0.0f, 1.0f))
          {
            updateThumbFn();
          }

          if (ImGui::DragFloat("Roughness", &(mat->m_roughness), 0.001f, 0.0f, 1.0f))
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
        if (ImGui::Combo("Blend mode", &blendMode, "None\0Alpha Blending\0Alpha Mask"))
        {
          renderState->blendFunction = (BlendFunction) blendMode;
          if (renderState->blendFunction == BlendFunction::NONE)
          {
            mat->SetAlpha(1.0f);
          }

          mat->m_dirty = true;

          updateThumbFn();
        }

        int drawType = DrawTypeToInt(mat->GetRenderState()->drawType);

        if (ImGui::Combo("Draw mode", &drawType, "Triangle\0Line\0Line Strip\0Line Loop\0Point"))
        {
          renderState->drawType = IntToDrawType(drawType);
          updateThumbFn();
        }

        if (renderState->blendFunction == BlendFunction::ALPHA_MASK)
        {
          float alphaMaskTreshold = renderState->alphaMaskTreshold;
          if (ImGui::DragFloat("Alpha Mask Threshold", &alphaMaskTreshold, 0.001f, 0.0f, 1.0f, "%.3f"))
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

    void MaterialView::Show()
    {
      ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);

      float treeHeight              = glm::min(20.0f + (m_materials.size() * 30.0f), 90.0f);
      int numMaterials              = (int) m_materials.size();

      const auto showMaterialNodeFn = [this](MaterialPtr mat, int i) -> void
      {
        String name;
        DecomposePath(m_materials[i]->GetFile(), nullptr, &name, nullptr);

        bool isSelected          = i == m_currentMaterialIndex;
        ImGuiTreeNodeFlags flags = isSelected * ImGuiTreeNodeFlags_Selected;

        ImGui::TreeNodeEx(name.c_str(), flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);

        if (ImGui::IsItemClicked())
        {
          m_currentMaterialIndex = i;
        }
      };

      if (m_materials.size() == 0)
      {
        ImGui::Text("There is no material selected.");
      }
      else if (numMaterials > 1)
      {
        ImGui::BeginChild("##MultiMaterials", ImVec2(0.0f, treeHeight), true);

        if (ImGui::TreeNode("Multi Materials"))
        {
          for (int i = 0; i < m_materials.size(); i++)
          {
            showMaterialNodeFn(m_materials[i], i);
          }
          ImGui::TreePop();
        }
      }

      if (m_materials.size() > 1)
      {
        ImGui::EndChild();
      }

      ImGui::Spacing();

      if (m_materials.size() > 0)
      {
        m_currentMaterialIndex = glm::clamp(m_currentMaterialIndex, 0, (int) (m_materials.size()) - 1);
        ShowMaterial(m_materials[m_currentMaterialIndex]);
      }
    }

    // ######   TempMaterialWindow   ######

    TempMaterialWindow::TempMaterialWindow()
    {
      m_view               = std::make_shared<MaterialView>();
      m_view->m_isTempView = true;
      UI::AddTempWindow(this);
    }

    TempMaterialWindow::~TempMaterialWindow()
    {
      UI::RemoveTempWindow(this);
      m_view = nullptr;
    }

    void TempMaterialWindow::SetMaterial(MaterialPtr mat) { m_view->SetMaterials({mat}); }

    void TempMaterialWindow::OpenWindow() { m_isOpen = true; }

    void TempMaterialWindow::Show()
    {
      if (!m_isOpen)
      {
        return;
      }

      ImGuiIO io       = ImGui::GetIO();

      ImGuiStyle style = ImGui::GetStyle();
      ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_Once);
      ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                              ImGuiCond_Once,
                              ImVec2(0.5f, 0.5f));

      ImGui::Begin("Material View", &m_isOpen);

      m_view->Show();

      ImGui::End();
    }
  } // namespace Editor
} // namespace ToolKit