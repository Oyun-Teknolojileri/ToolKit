#include "MaterialInspector.h"
#include "App.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    // MaterialView
    //////////////////////////////////////////////////////////////////////////

    void MaterialView::Show()
    {
      Drawable* drawable = static_cast<Drawable*> (m_entity);
      MaterialPtr entry;

      bool entityMod = true;
      if ((entry = m_material))
      {
        entityMod = false;
      }
      else
      {
        if (drawable == nullptr)
        {
          return;
        }
        entry = drawable->GetMesh()->m_material;
      }

      auto updateThumbFn = [&entry]() -> void
      {
        DirectoryEntry dirEnt(entry->GetFile());
        dirEnt.GenerateThumbnail();
        entry->m_dirty = true;
      };

      String name, ext;
      DecomposePath(entry->GetFile(), nullptr, &name, &ext);

      ImGui::Text("Material: %s%s", name.c_str(), ext.c_str());
      ImGui::Separator();

      Vec4 col = Vec4(entry->m_color, entry->m_alpha);
      if
      (
        ImGui::ColorEdit4
        (
          "Material Color##1",
          &col.x,
          ImGuiColorEditFlags_NoLabel
        )
      )
      {
        entry->m_color = col.xyz;
        entry->m_alpha = col.a;
        updateThumbFn();
      }

      if (ImGui::TreeNode("Textures"))
      {
        ImGui::LabelText("##diffTexture", "Diffuse Texture: ");
        String target = GetPathSeparatorAsStr();
        if (entry->m_diffuseTexture)
        {
          target = entry->m_diffuseTexture->GetFile();
        }

        DropZone
        (
          UI::m_imageIcon->m_textureId,
          target,
          [&entry, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
          {
            // Switch from solid color material to default for texturing.
            if (entry->m_diffuseTexture == nullptr)
            {
              entry->m_fragmetShader =
              GetShaderManager()->Create<Shader>
              (
                ShaderPath("defaultFragment.shader", true)
              );
              entry->m_fragmetShader->Init();
            }
            entry->m_diffuseTexture =
            GetTextureManager()->Create<Texture>(dirEnt.GetFullPath());
            entry->m_diffuseTexture->Init();
            updateThumbFn();
          }
        );

        ImGui::TreePop();
      }

      if (ImGui::TreeNode("Shaders"))
      {
        ImGui::LabelText("##vertShader", "Vertex Shader: ");
        DropZone
        (
          UI::m_codeIcon->m_textureId,
          entry->m_vertexShader->GetFile(),
          [&entry, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
          {
            if (strcmp(dirEnt.m_ext.c_str(), ".shader") != 0)
            {
              g_app->m_statusMsg = "An imported shader file expected!";
              return;
            }
            entry->m_vertexShader =
            GetShaderManager()->Create<Shader>(dirEnt.GetFullPath());
            entry->m_vertexShader->Init();
            updateThumbFn();
          }
        );

        ImGui::LabelText("##fragShader", "Fragment Shader: ");
        DropZone
        (
          UI::m_codeIcon->m_textureId,
          entry->m_fragmetShader->GetFile(),
          [&entry, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
          {
            entry->m_fragmetShader =
            GetShaderManager()->Create<Shader>(dirEnt.GetFullPath());
            entry->m_fragmetShader->Init();
            updateThumbFn();
          }
        );
        ImGui::TreePop();
      }

      if (ImGui::TreeNode("Render State"))
      {
        int cullMode = static_cast<int>(entry->GetRenderState()->cullMode);
        if (ImGui::Combo("Cull mode", &cullMode, "Two Sided\0Front\0Back"))
        {
          entry->GetRenderState()->cullMode = (CullingType)cullMode;
          entry->m_dirty = true;
        }

        int blendMode =
        static_cast<int>(entry->GetRenderState()->blendFunction);
        if (ImGui::Combo("Blend mode", &blendMode, "None\0Alpha Blending"))
        {
          entry->GetRenderState()->blendFunction = (BlendFunction)blendMode;
          entry->m_dirty = true;
        }

        int drawType = -1;
        switch (entry->GetRenderState()->drawType)
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

        if
        (
          ImGui::Combo
          (
            "Draw mode",
            &drawType,
            "Triangle\0Line\0Line Strip\0Line Loop\0Point"
          )
        )
        {
          switch (drawType)
          {
          case 0:
            entry->GetRenderState()->drawType = DrawType::Triangle;
            break;
          case 1:
            entry->GetRenderState()->drawType = DrawType::Line;
            break;
          case 2:
            entry->GetRenderState()->drawType = DrawType::LineStrip;
            break;
          case 3:
            entry->GetRenderState()->drawType = DrawType::LineLoop;
            break;
          case 4:
            entry->GetRenderState()->drawType = DrawType::Point;
            break;
          }

          entry->m_dirty = true;
        }

        bool depthTest = entry->GetRenderState()->depthTestEnabled;
        if (ImGui::Checkbox("Enable depth test", &depthTest))
        {
          entry->GetRenderState()->depthTestEnabled = depthTest;
          entry->m_dirty = true;
        }

        ImGui::TreePop();
      }

      if (entityMod)
      {
        DropSubZone
        (
          "Material##" + std::to_string(entry->m_id),
          UI::m_materialIcon->m_textureId,
          entry->GetFile(),
          [&drawable](const DirectoryEntry& dirEnt) -> void
          {
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
          }
        );
      }
    }

    MaterialInspector::MaterialInspector(XmlNode* node)
      : MaterialInspector()
    {
      DeSerialize(nullptr, node);
    }

    MaterialInspector::MaterialInspector()
    {
      m_view = new MaterialView();
    }

    MaterialInspector::~MaterialInspector()
    {
      SafeDel(m_view);
    }

    void MaterialInspector::Show()
    {
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        if (m_material == nullptr)
        {
          ImGui::Text("Select a material");
        }
        else
        {
          m_view->m_material = m_material;
          m_view->Show();
        }
      }
      ImGui::End();
    }

    Window::Type MaterialInspector::GetType() const
    {
      return Type::MaterialInspector;
    }

    void MaterialInspector::DispatchSignals() const
    {
      ModShortCutSignals({ SDL_SCANCODE_DELETE });
    }

  }
}
