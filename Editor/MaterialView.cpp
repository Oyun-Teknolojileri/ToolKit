#include "MaterialView.h"

#include "App.h"

namespace ToolKit
{
  namespace Editor
  {

    // MaterialView
    //////////////////////////////////////////////////////////////////////////

    MaterialView::MaterialView() : View("Material View")
    {
      m_viewID  = 3;
      m_viewIcn = UI::m_materialIcon;

      // Initialize scene
      m_previewScene = std::make_shared<Scene>();
      Entity* previewEntity = new Entity;
      previewEntity->AddComponent(std::make_shared<MaterialComponent>());
      m_previewScene->AddEntity(previewEntity);
    }
    void MaterialView::SetMaterial(MaterialPtr mat)
    {
      Entity* previewEntity = m_previewScene->GetEntities()[0];
      previewEntity->GetMaterialComponent()->SetMaterialVal(mat);
    }

    void MaterialView::Show()
    {
      Entity* previewEntity = m_previewScene->GetEntities()[0];
      MaterialPtr mat = previewEntity->GetMaterialComponent()->GetMaterialVal();
      if (!mat)
      {
        ImGui::Text("Select a material");
        return;
      }

      String name, ext;
      DecomposePath(mat->GetFile(), nullptr, &name, &ext);

      ImGui::Text("Material: %s%s", name.c_str(), ext.c_str());
      ImGui::Separator();

      if (ImGui::CollapsingHeader("Material Preview",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {

        // Draw Preview Scene
      }

      auto updateThumbFn = [&mat]() -> void {
        DirectoryEntry dirEnt(mat->GetFile());
        g_app->m_thumbnailCache.erase(mat->GetFile());

        dirEnt.GenerateThumbnail();
        mat->m_dirty = true;
      };

      if (ImGui::CollapsingHeader("Shaders"))
      {
        ImGui::LabelText("##vertShader", "Vertex Shader: ");
        DropZone(UI::m_codeIcon->m_textureId,
                 mat->m_vertexShader->GetFile(),
                 [&mat, &updateThumbFn](const DirectoryEntry& dirEnt) -> void {
                   if (strcmp(dirEnt.m_ext.c_str(), ".shader") != 0)
                   {
                     g_app->m_statusMsg = "An imported shader file expected!";
                     return;
                   }
                   mat->m_vertexShader =
                       GetShaderManager()->Create<Shader>(dirEnt.GetFullPath());
                   mat->m_vertexShader->Init();
                   updateThumbFn();
                 });

        ImGui::LabelText("##fragShader", "Fragment Shader: ");
        DropZone(UI::m_codeIcon->m_textureId,
                 mat->m_fragmentShader->GetFile(),
                 [&mat, &updateThumbFn](const DirectoryEntry& dirEnt) -> void {
                   mat->m_fragmentShader =
                       GetShaderManager()->Create<Shader>(dirEnt.GetFullPath());
                   mat->m_fragmentShader->Init();
                   updateThumbFn();
                 });
      }

      if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
      {
        ImGui::LabelText("##diffTexture", "Diffuse Texture: ");
        String target = GetPathSeparatorAsStr();
        if (mat->m_diffuseTexture)
        {
          target = mat->m_diffuseTexture->GetFile();
        }

        DropZone(UI::m_imageIcon->m_textureId,
                 target,
                 [&mat, &updateThumbFn](const DirectoryEntry& dirEnt) -> void {
                   // Switch from solid color material to default for texturing.
                   if (mat->m_diffuseTexture == nullptr)
                   {
                     mat->m_fragmentShader = GetShaderManager()->Create<Shader>(
                         ShaderPath("defaultFragment.shader", true));
                     mat->m_fragmentShader->Init();
                   }
                   mat->m_diffuseTexture = GetTextureManager()->Create<Texture>(
                       dirEnt.GetFullPath());
                   mat->m_diffuseTexture->Init();
                   updateThumbFn();
                 });
      }

      if (ImGui::CollapsingHeader("Render States",
                                  ImGuiTreeNodeFlags_DefaultOpen))
      {
        Vec4 col = Vec4(mat->m_color, mat->m_alpha);
        if (ImGui::ColorEdit4(
                "Material Color##1", &col.x, ImGuiColorEditFlags_NoLabel))
        {
          mat->m_color = col.xyz;
          mat->m_alpha = col.a;
          updateThumbFn();
        }

        int cullMode = static_cast<int>(mat->GetRenderState()->cullMode);
        if (ImGui::Combo("Cull mode", &cullMode, "Two Sided\0Front\0Back"))
        {
          mat->GetRenderState()->cullMode = (CullingType) cullMode;
          mat->m_dirty                    = true;
        }

        int blendMode = static_cast<int>(mat->GetRenderState()->blendFunction);
        if (ImGui::Combo("Blend mode", &blendMode, "None\0Alpha Blending"))
        {
          mat->GetRenderState()->blendFunction = (BlendFunction) blendMode;
          mat->m_dirty                         = true;
        }

        int drawType = -1;
        switch (mat->GetRenderState()->drawType)
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
            mat->GetRenderState()->drawType = DrawType::Triangle;
            break;
          case 1:
            mat->GetRenderState()->drawType = DrawType::Line;
            break;
          case 2:
            mat->GetRenderState()->drawType = DrawType::LineStrip;
            break;
          case 3:
            mat->GetRenderState()->drawType = DrawType::LineLoop;
            break;
          case 4:
            mat->GetRenderState()->drawType = DrawType::Point;
            break;
          }

          mat->m_dirty = true;
        }

        bool depthTest = mat->GetRenderState()->depthTestEnabled;
        if (ImGui::Checkbox("Enable depth test", &depthTest))
        {
          mat->GetRenderState()->depthTestEnabled = depthTest;
          mat->m_dirty                            = true;
        }

        bool AOInUse = mat->GetRenderState()->AOInUse;
        if (ImGui::Checkbox("Ambient Occlusion", &AOInUse))
        {
          mat->GetRenderState()->AOInUse = AOInUse;
          mat->m_dirty                   = true;
        }
      }

      /*
      bool entityMod = true;
      if ((mat = m_material))
      {
        entityMod = false;
      }
      else
      {
        if (drawable == nullptr)
        {
          return;
        }
        mat = drawable->GetMesh()->m_material;
      }



      if (entityMod)
      {
        DropSubZone(
            "Material##" + std::to_string(mat->m_id),
            UI::m_materialIcon->m_textureId,
            mat->GetFile(),
            [&drawable](const DirectoryEntry& dirEnt) -> void {
              MeshPtr mesh = drawable->GetMesh();
              if (strcmp(dirEnt.m_ext.c_str(), ".material") != 0)
              {
                g_app->m_statusMsg = "An imported material file expected!";
                return;
              }
              mesh->m_material =
                  GetMaterialManager()->Create<Material>(dirEnt.GetFullPath());
              mesh->m_material->Init();
              mesh->m_dirty = true;
            },
            true);
      }*/
    }
  } // namespace Editor
} // namespace ToolKit
