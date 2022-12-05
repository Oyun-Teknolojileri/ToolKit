#include "PropInspector.h"

#include "AnchorMod.h"
#include "App.h"
#include "ConsoleWindow.h"
#include "ImGui/imgui_stdlib.h"
#include "MaterialInspector.h"
#include "Prefab.h"
#include "TransformMod.h"
#include "Util.h"

#include <memory>
#include <utility>

#include "DebugNew.h"
#include "EntityView.h"
#include "PrefabView.h"
#include "CustomDataView.h"
#include "ComponentView.h"
#include "MaterialView.h"

namespace ToolKit
{
  namespace Editor
  {

    // View
    //////////////////////////////////////////////////////////////////////////

    bool View::IsTextInputFinalized()
    {
      return (ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) ||
              ImGui::IsKeyPressed(ImGuiKey_Enter) ||
              ImGui::IsKeyPressed(ImGuiKey_Tab));
    }

    void View::DropZone(
        uint fallbackIcon,
                  const String& file,
                  std::function<void(const DirectoryEntry& entry)> dropAction,
                  const String& dropName)
    {
      DirectoryEntry dirEnt;

      bool fileExist                        = false;
      FolderWindowRawPtrArray folderWindows = g_app->GetAssetBrowsers();
      for (FolderWindow* folderWnd : folderWindows)
      {
        if (folderWnd->GetFileEntry(file, dirEnt))
        {
          fileExist = true;
        }
      }
      uint iconId = fallbackIcon;

      ImVec2 texCoords = ImVec2(1.0f, 1.0f);
      if (RenderTargetPtr thumb = dirEnt.GetThumbnail())
      {
        texCoords = ImVec2(1.0f, -1.0f);
        iconId    = thumb->m_textureId;
      }
      else if (fileExist)
      {
        dirEnt.GenerateThumbnail();

        if (RenderTargetPtr thumb = dirEnt.GetThumbnail())
        {
          iconId = thumb->m_textureId;
        }
      }

      if (!dropName.empty())
      {
        ImGui::Text(dropName.c_str());
      }

      bool clicked =
          ImGui::ImageButton(reinterpret_cast<void*>((intptr_t) iconId),
                             ImVec2(48.0f, 48.0f),
                             ImVec2(0.0f, 0.0f),
                             texCoords);

      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("BrowserDragZone"))
        {
          IM_ASSERT(payload->DataSize == sizeof(DirectoryEntry));
          DirectoryEntry entry = *(const DirectoryEntry*) payload->Data;
          dropAction(entry);
        }

        ImGui::EndDragDropTarget();
      }

      // Show file info.
      String info = "Drop zone";
      if (!file.empty() && !dirEnt.m_fileName.empty())
      {
        info = "";
        if (ResourceManager* man = dirEnt.GetManager())
        {
          auto textureRepFn = [&info, file](const TexturePtr& t) -> void {
            if (t)
            {
              String file, ext;
              DecomposePath(t->GetFile(), nullptr, &file, &ext);

              info += "Texture: " + file + ext + "\n";
              info += "Width: " + std::to_string(t->m_width) + "\n";
              info += "Height: " + std::to_string(t->m_height);
            }
          };

          if (man->m_type == ResourceType::Material)
          {
            MaterialPtr mr = man->Create<Material>(file);
            if (clicked)
            {
              g_app->GetMaterialInspector()->m_material = mr;
            }

            info += "File: " + dirEnt.m_fileName + dirEnt.m_ext + "\n";
            textureRepFn(mr->m_diffuseTexture);
          }

          if (man->m_type == ResourceType::Texture)
          {
            TexturePtr t = man->Create<Texture>(file);
            textureRepFn(t);
          }

          if (man->m_type == ResourceType::Mesh ||
              man->m_type == ResourceType::SkinMesh)
          {
            MeshPtr mesh = man->Create<Mesh>(file);
            info += "File: " + dirEnt.m_fileName + dirEnt.m_ext + "\n";
            info +=
                "Vertex Count: " + std::to_string(mesh->m_vertexCount) + "\n";
            info += "Index Count: " + std::to_string(mesh->m_indexCount) + "\n";
            if (mesh->m_faces.size())
            {
              info +=
                  "Face Count: " + std::to_string(mesh->m_faces.size()) + "\n";
            }
          }
        }
      }

      UI::HelpMarker(TKLoc + file, info.c_str(), 0.1f);
    }

    void View::DropSubZone(
        const String& title,
        uint fallbackIcon,
        const String& file,
        std::function<void(const DirectoryEntry& entry)> dropAction,
        bool isEditable)
    {
      ImGui::EndDisabled();
      bool isOpen = ImGui::TreeNodeEx(title.c_str());
      ImGui::BeginDisabled(!isEditable);
      if (isOpen)
      {
        DropZone(fallbackIcon, file, dropAction);
        ImGui::TreePop();
      }
    }


    View::View(const StringView viewName) : m_viewName(viewName)
    {
    }

    // PropInspector
    //////////////////////////////////////////////////////////////////////////

    PropInspector::PropInspector(XmlNode* node) : PropInspector()
    {
      DeSerialize(nullptr, node);
    }

    PropInspector::PropInspector()
    {
      m_views.push_back(new EntityView());
      m_views.push_back(new PrefabView());
      m_views.push_back(new CustomDataView());
      m_views.push_back(new ComponentView());
    }

    PropInspector::~PropInspector()
    {
      for (ViewRawPtr& view : m_views)
      {
        SafeDel(view);
      }
    }

    void PropInspector::Show()
    {
      ImVec4 windowBg  = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
      ImVec4 childBg   = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
      ImGuiStyle style = ImGui::GetStyle();
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                          ImVec2(2, style.ItemSpacing.y));
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        const ImVec2 windowSize      = ImGui::GetWindowSize();
        const ImVec2 sidebarIconSize = ImVec2(18, 18);

        // Show ViewType sidebar
        ImGui::GetStyle()    = style;
        const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
        const ImVec2 sidebarSize =
            ImVec2(spacing.x + sidebarIconSize.x + spacing.x, windowSize.y);
        if (ImGui::BeginChildFrame(
                ImGui::GetID("ViewTypeSidebar"), sidebarSize, 0))
        {
          for (uint viewIndx = 0; viewIndx < m_views.size(); viewIndx++)
          {
            ViewRawPtr view = m_views[viewIndx];

            if (m_activeViewIndx == viewIndx)
            {
              ImGui::PushStyleColor(ImGuiCol_Button, windowBg);
            }
            else
            {
              ImGui::PushStyleColor(ImGuiCol_Button, childBg);
            }
            if (ImGui::ImageButton(reinterpret_cast<void*>(
                                       (intptr_t) view->m_viewIcn->m_textureId),
                                   sidebarIconSize))
            {
              m_activeViewIndx = viewIndx;
            }
            UI::HelpMarker("View#" + std::to_string(viewIndx),
                           view->m_viewName.data());
            ImGui::PopStyleColor(1);
          }
          ImGui::EndChildFrame();
        }

        ImGui::SameLine();

        if (ImGui::BeginChild(
                ImGui::GetID("PropInspectorActiveView"),
                Vec2(windowSize.x - sidebarSize.x - spacing.x, windowSize.y)))
        {
          m_views[m_activeViewIndx]->Show();
          ImGui::EndChild();
        }
      }
      ImGui::End();
      ImGui::PopStyleVar(2);
    }

    Window::Type PropInspector::GetType() const
    {
      return Window::Type::Inspector;
    }

    void PropInspector::DispatchSignals() const
    {
      ModShortCutSignals();
    }

  } // namespace Editor
} // namespace ToolKit
