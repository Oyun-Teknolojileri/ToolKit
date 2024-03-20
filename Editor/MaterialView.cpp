/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "MaterialView.h"

#include "App.h"
#include "EditorViewport.h"
#include "PreviewViewport.h"

#include <DirectionComponent.h>
#include <FileManager.h>
#include <GradientSky.h>
#include <Material.h>

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

      m_viewport = MakeNewPtr<PreviewViewport>();
      m_viewport->Init({300.0f, 150.0f});

      SceneManager* scnMan = GetSceneManager();
      m_scenes[0]          = scnMan->Create<Scene>(ScenePath("ms-sphere.scene", true));
      m_scenes[1]          = scnMan->Create<Scene>(ScenePath("ms-box.scene", true));
      m_scenes[2]          = scnMan->Create<Scene>(ScenePath("ms-ball.scene", true));

      m_viewport->SetScene(m_scenes[0]);

      ResetCamera();
    }

    MaterialView::~MaterialView() { m_viewport = nullptr; }

    void MaterialView::SetSelectedMaterial(MaterialPtr mat)
    {
      auto find = std::find(m_materials.cbegin(), m_materials.cend(), mat);
      if (find != m_materials.cend())
      {
        m_currentMaterialIndex = int(find - m_materials.cbegin());
      }
    }

    void MaterialView::SetMaterials(const MaterialPtrArray& mat) { m_materials = mat; }

    void MaterialView::ResetCamera() { m_viewport->ResetCamera(); }

    void MaterialView::UpdatePreviewScene()
    {
      m_viewport->SetScene(m_scenes[m_activeObjectIndx]);

      EntityPtrArray materialNtties = m_viewport->GetScene()->GetByTag("target");
      for (EntityPtr ntt : materialNtties)
      {
        ntt->GetMaterialComponent()->SetFirstMaterial(m_materials[m_currentMaterialIndex]);
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

      if (ImGui::CollapsingHeader("Material Preview", ImGuiTreeNodeFlags_DefaultOpen))
      {
        const ImVec2 iconSize = ImVec2(16.0f, 16.0f);
        const ImVec2 spacing  = ImGui::GetStyle().ItemSpacing;
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
          m_viewport->SetViewportSize((uint) viewportSize.x, (uint) viewportSize.y);
          m_viewport->Update(g_app->GetDeltaTime());
          m_viewport->Show();
          ImGui::SameLine();
          ImGui::BeginGroup();

          auto setIconFn = [&](TexturePtr icon, uint id) -> void
          {
            if (ImGui::ImageButton(Convert2ImGuiTexture(icon), iconSize))
            {
              m_activeObjectIndx = id;
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
        mat->UpdateRuntimeVersion();
      };

      if (ImGui::CollapsingHeader("Shaders"))
      {
        ImGui::BeginGroup();
        String vertName;
        DecomposePath(mat->m_vertexShader->GetFile(), nullptr, &vertName, nullptr);

        ImGui::LabelText("##vertex shader: %s", vertName.c_str());
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

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 20.0f);

        ImGui::BeginGroup();
        String fragName = mat->m_fragmentShader->GetFile();
        DecomposePath(mat->m_fragmentShader->GetFile(), nullptr, &fragName, nullptr);

        ImGui::LabelText("##fragShader fragment shader: %s", fragName.c_str());
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
            mat->SetAlpha(mat->GetAlpha());
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
      m_view               = MakeNewPtr<MaterialView>();
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