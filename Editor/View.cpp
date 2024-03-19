/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "View.h"

#include "MaterialView.h"
#include "PropInspector.h"

#include <FileManager.h>
#include <Mesh.h>
#include <ToolKit.h>

namespace ToolKit
{
  namespace Editor
  {

    View::View(const StringView viewName) : m_viewName(viewName) {}

    View::~View() {}

    bool View::IsTextInputFinalized()
    {
      return (ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) || ImGui::IsKeyPressed(ImGuiKey_Enter) ||
              ImGui::IsKeyPressed(ImGuiKey_Tab));
    }

    void View::DropZone(uint fallbackIcon,
                        const String& file,
                        std::function<void(DirectoryEntry& entry)> dropAction,
                        const String& dropName,
                        bool isEditable)
    {
      DirectoryEntry dirEnt;

      bool fileExist                        = GetFileManager()->CheckFileFromResources(file);
      FolderWindowRawPtrArray folderWindows = g_app->GetAssetBrowsers();
      for (FolderWindow* folderWnd : folderWindows)
      {
        if (folderWnd->GetFileEntry(file, dirEnt))
        {
          fileExist = true;
        }
      }

      uint iconId                        = fallbackIcon;
      ImVec2 texCoords                   = ImVec2(1.0f, 1.0f);

      ThumbnailManager& thumbnailManager = g_app->m_thumbnailManager;

      if (dirEnt.m_ext.length())
      {
        if (thumbnailManager.TryGetThumbnail(iconId, dirEnt))
        {
          texCoords = ImVec2(1.0f, -1.0f);
        }
      }
      else if (fileExist)
      {
        DecomposePath(file, &dirEnt.m_rootPath, &dirEnt.m_fileName, &dirEnt.m_ext);

        thumbnailManager.TryGetThumbnail(iconId, dirEnt);
      }

      if (!dropName.empty())
      {
        ImGui::Text(dropName.c_str());
      }

      ImGui::ImageButton(reinterpret_cast<void*>((intptr_t) iconId),
                         ImVec2(48.0f, 48.0f),
                         ImVec2(0.0f, 0.0f),
                         texCoords);

      bool clicked  = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
      clicked      &= ImGui::IsItemHovered();

      if (isEditable && ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
        {
          const FileDragData& dragData = FolderView::GetFileDragData();
          DirectoryEntry& entry        = *dragData.Entries[0]; // get first entry
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
          auto textureRepFn = [&info, file](const TexturePtr& t) -> void
          {
            if (t)
            {
              String file, ext;
              DecomposePath(t->GetFile(), nullptr, &file, &ext);

              info += "Texture: " + file + ext + "\n";
              info += "Width: " + std::to_string(t->m_width) + "\n";
              info += "Height: " + std::to_string(t->m_height);
            }
          };

          if (man->m_baseType == Material::StaticClass())
          {
            MaterialPtr mr = man->Create<Material>(file);
            if (clicked)
            {
              PropInspector* propInspector = g_app->GetPropInspector();
              propInspector->GetMaterialView()->SetSelectedMaterial(mr);
              propInspector->SetActiveView(ViewType::Material);
            }

            info += "File: " + dirEnt.m_fileName + dirEnt.m_ext + "\n";
            textureRepFn(mr->m_diffuseTexture);
          }

          if (man->m_baseType == Texture::StaticClass())
          {
            TexturePtr t = man->Create<Texture>(file);
            textureRepFn(t);
          }

          if (man->m_baseType->IsSublcassOf(Mesh::StaticClass()))
          {
            MeshPtr mesh = man->Create<Mesh>(file);

            if (clicked)
            {
              g_app->GetPropInspector()->SetMeshView(mesh);
            }

            info += "File: " + dirEnt.m_fileName + dirEnt.m_ext + "\n";
            info += "Vertex Count: " + std::to_string(mesh->m_vertexCount) + "\n";
            info += "Index Count: " + std::to_string(mesh->m_indexCount) + "\n";
            if (mesh->m_faces.size())
            {
              info += "Face Count: " + std::to_string(mesh->m_faces.size()) + "\n";
            }
          }
        }
      }

      if (fileExist && info != "")
      {
        UI::HelpMarker(TKLoc + file, info.c_str(), 0.1f);
      }
    }

    void View::DropSubZone(const String& title,
                           uint fallbackIcon,
                           const String& file,
                           std::function<void(const DirectoryEntry& entry)> dropAction,
                           bool isEditable)
    {
      bool isOpen = ImGui::TreeNodeEx(title.c_str());
      if (isOpen)
      {
        DropZone(fallbackIcon, file, dropAction, "", isEditable);
        ImGui::TreePop();
      }
    }

  } // namespace Editor
} // namespace ToolKit