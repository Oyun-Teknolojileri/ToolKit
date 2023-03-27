#include "FolderWindow.h"

#include "App.h"
#include "ConsoleWindow.h"
#include "DirectionComponent.h"
#include "Framebuffer.h"
#include "Gizmo.h"
#include "Global.h"
#include "Light.h"
#
#include "PopupWindows.h"
#include "PropInspector.h"
#include "Util.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {
    DirectoryEntry* FolderView::m_currentEntry = nullptr;

    DirectoryEntry::DirectoryEntry() {}

    DirectoryEntry::DirectoryEntry(const String& fullPath)
    {
      DecomposePath(fullPath, &m_rootPath, &m_fileName, &m_ext);
    }

    String DirectoryEntry::GetFullPath() const
    {
      return ConcatPaths({m_rootPath, m_fileName + m_ext});
    }

    ResourceManager* DirectoryEntry::GetManager() const
    {
      if (m_ext == ANIM)
      {
        return GetAnimationManager();
      }
      else if (m_ext == AUDIO)
      {
        return GetAudioManager();
      }
      else if (m_ext == MATERIAL)
      {
        return GetMaterialManager();
      }
      else if (m_ext == MESH || m_ext == SKINMESH)
      {
        return GetMeshManager();
      }
      else if (m_ext == SHADER)
      {
        return GetShaderManager();
      }
      else if (SupportedImageFormat(m_ext))
      {
        return GetTextureManager();
      }
      else if (m_ext == HDR)
      {
        return GetTextureManager();
      }
      else if (m_ext == SCENE)
      {
        return GetSceneManager();
      }

      return nullptr;
    }

    RenderTargetPtr DirectoryEntry::GetThumbnail() const
    {
      return g_app->m_thumbnailManager.GetThumbnail(*this);
    }

    FolderView::FolderView() { CreateItemActions(); }

    FolderView::FolderView(class FolderWindow* parent) : FolderView()
    {
      m_parent = parent;
    }

    void FolderView::Show()
    {
      bool* visCheck = nullptr;
      if (!m_currRoot)
      {
        visCheck = &m_visible;
      }

      ImGuiTabItemFlags flags =
          m_activateNext ? ImGuiTabItemFlags_SetSelected : 0;
      m_activateNext = false;
      if (ImGui::BeginTabItem(m_folder.c_str(), visCheck, flags))
      {
        m_parent->SetActiveView(this);

        if (m_dirty)
        {
          Iterate();
          m_dirty = false;
        }

        // Handle Item Icon size.
        ImGuiIO io                 = ImGui::GetIO();
        float delta                = io.MouseWheel;

        // Initial zoom value
        static float thumbnailZoom = m_thumbnailMaxZoom / 6.f;

        // Zoom in and out
        if (io.KeyCtrl &&
            ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
        {
          thumbnailZoom += delta * 10.0f;
        }

        // Clamp icon size
        if (thumbnailZoom < m_thumbnailMaxZoom / 6.f)
        {
          thumbnailZoom = m_thumbnailMaxZoom / 6.f;
        }
        if (thumbnailZoom > m_thumbnailMaxZoom)
        {
          thumbnailZoom = m_thumbnailMaxZoom;
        }

        m_iconSize.xy = Vec2(thumbnailZoom);

        // Item dropped to tab.
        MoveTo(m_path);

        // Start drawing folder items.
        const float footerHeightReserve = ImGui::GetStyle().ItemSpacing.y +
                                          ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("##Content", ImVec2(0, -footerHeightReserve), true);

        if (m_entries.empty())
        {
          // Handle context menu based on path / content type of the folder.
          ShowContextMenu();
        }
        else
        {
          // Draw folder items.
          for (int i = 0; i < static_cast<int>(m_entries.size()); i++)
          {
            // Prepare Item Icon.
            ImGuiStyle& style = ImGui::GetStyle();
            float visX2 =
                ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

            DirectoryEntry& dirEnt = m_entries[i];

            if (!Utf8CaseInsensitiveSearch(dirEnt.m_fileName, m_filter))
            {
              continue;
            }

            bool flipRenderTarget = false;
            uint iconId           = UI::m_fileIcon->m_textureId;

            auto genThumbFn = [&flipRenderTarget, &iconId, &dirEnt]() -> void
            {
              iconId           = dirEnt.GetThumbnail()->m_textureId;
              flipRenderTarget = true;
            };

            if (dirEnt.m_isDirectory)
            {
              iconId = UI::m_folderIcon->m_textureId;
            }
            else if (dirEnt.m_ext == SCENE || dirEnt.m_ext == LAYER)
            {
              iconId = UI::m_worldIcon->m_textureId;
            }
            else if (dirEnt.m_ext == MESH || dirEnt.m_ext == SKINMESH)
            {
              genThumbFn();
            }
            else if (dirEnt.m_ext == ANIM)
            {
              iconId = UI::m_clipIcon->m_textureId;
            }
            else if (dirEnt.m_ext == AUDIO)
            {
              iconId = UI::m_audioIcon->m_textureId;
            }
            else if (dirEnt.m_ext == SHADER)
            {
              iconId = UI::m_codeIcon->m_textureId;
            }
            else if (dirEnt.m_ext == SKELETON)
            {
              iconId = UI::m_boneIcon->m_textureId;
            }
            else if (dirEnt.m_ext == MATERIAL)
            {
              genThumbFn();
            }
            else if (SupportedImageFormat(dirEnt.m_ext))
            {
              genThumbFn();
            }
            else if (dirEnt.m_ext == HDR)
            {
              genThumbFn();
            }
            else
            {
              if (m_onlyNativeTypes)
              {
                continue;
              }
            }

            ImGui::PushID(i);
            ImGui::BeginGroup();
            ImVec2 texCoords =
                flipRenderTarget ? ImVec2(1.0f, -1.0f) : ImVec2(1.0f, 1.0f);

            // Draw Item Icon.
            if (ImGui::ImageButton(ConvertUIntImGuiTexture(iconId),
                                   m_iconSize,
                                   ImVec2(0.0f, 0.0f),
                                   texCoords))
            {
              if (ResourceManager* rm = dirEnt.GetManager())
              {
                switch (rm->m_type)
                {
                case ResourceType::Material:
                  g_app->GetPropInspector()->SetMaterialView(
                      rm->Create<Material>(dirEnt.GetFullPath()));
                  break;
                case ResourceType::Mesh:
                  g_app->GetPropInspector()->SetMeshView(
                      rm->Create<Mesh>(dirEnt.GetFullPath()));
                  break;
                case ResourceType::SkinMesh:
                  g_app->GetPropInspector()->SetMeshView(
                      rm->Create<SkinMesh>(dirEnt.GetFullPath()));
                  break;
                }
              }
            }

            // Handle context menu.
            ShowContextMenu(&dirEnt);

            // Handle if item is directory.
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
              if (ImGui::IsItemHovered())
              {
                if (dirEnt.m_isDirectory)
                {
                  if (m_parent != nullptr)
                  {
                    String path =
                        ConcatPaths({dirEnt.m_rootPath, dirEnt.m_fileName});
                    int indx = m_parent->Exist(path);
                    if (indx == -1)
                    {
                      FolderView view(m_parent);
                      view.SetPath(path);
                      view.Iterate();
                      view.Refresh();
                      m_parent->AddEntry(view);
                    }
                    else
                    {
                      FolderView& view    = m_parent->GetView(indx);
                      view.m_visible      = true;
                      view.m_activateNext = true;
                    }
                  }
                }
              }
            }

            // Handle mouse hover tips.
            String fullName = dirEnt.m_fileName + dirEnt.m_ext;
            UI::HelpMarker(TKLoc + fullName, fullName.c_str());

            // Handle drag - drop to scene / inspector.
            if (!dirEnt.m_isDirectory)
            {
              ImGui::PushStyleColor(ImGuiCol_PopupBg,
                                    ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
              if (ImGui::BeginDragDropSource(
                      ImGuiDragDropFlags_SourceAllowNullID))
              {
                ImGui::SetDragDropPayload("BrowserDragZone",
                                          &dirEnt,
                                          sizeof(DirectoryEntry));
                ImGui::ImageButton(ConvertUIntImGuiTexture(iconId),
                                   m_iconSize,
                                   ImVec2(0.0f, 0.0f),
                                   texCoords);
                ImGui::EndDragDropSource();
              }
              ImGui::PopStyleColor();
            }

            // Make directories drop target for resources.
            if (dirEnt.m_isDirectory)
            {
              MoveTo(ConcatPaths({dirEnt.m_rootPath, dirEnt.m_fileName}));
            }

            // Handle Item sub text.
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + m_iconSize.x);
            size_t charLim = size_t(m_iconSize.x * 0.1f);
            if (dirEnt.m_fileName.size() > charLim)
            {
              String shorten = dirEnt.m_fileName.substr(0, charLim) + "...";
              ImGui::TextWrapped("%s", shorten.c_str());
            }
            else
            {
              ImGui::TextWrapped("%s", dirEnt.m_fileName.c_str());
            }
            ImGui::PopTextWrapPos();
            ImGui::EndGroup();
            ImGui::PopID();

            // Handle next column / row.
            float lastBtnX2 = ImGui::GetItemRectMax().x;
            float nextBtnX2 = lastBtnX2 + style.ItemSpacing.x + m_iconSize.x;
            if (nextBtnX2 < visX2)
            {
              ImGui::SameLine();
            }
          }
        } // Tab item handling ends.
        ImGui::EndChild();

        ImGui::BeginTable("##FilterZoom", 5, ImGuiTableFlags_SizingFixedFit);

        ImGui::TableSetupColumn("##flt", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##zoom");
        ImGui::TableSetupColumn("##tglzoom");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // Handle searchbar
        ImGui::PushItemWidth(-1);
        ImGui::InputTextWithHint(" Search", "Search", &m_filter);
        ImGui::PopItemWidth();

        // Zoom.
        ImGui::TableNextColumn();
        ImGui::Text("%.0f%%", GetThumbnailZoomPercent(thumbnailZoom));

        // Zoom toggle button
        ImGui::TableNextColumn();
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_viewZoomIcon),
                               ImVec2(20.0f, 20.0f)))
        {
          // Toggle zoom
          if (thumbnailZoom == m_thumbnailMaxZoom)
          {
            // Small
            thumbnailZoom = m_thumbnailMaxZoom / 6.f;
          }
          // (7/12 ~ 0.5833)
          else if (thumbnailZoom >= m_thumbnailMaxZoom * 0.5833f)
          {
            // Big
            thumbnailZoom = m_thumbnailMaxZoom;
          }
          else if (thumbnailZoom >= m_thumbnailMaxZoom / 6.f)
          {
            // Medium
            thumbnailZoom = m_thumbnailMaxZoom * 0.5833f; // (7/12 ~ 0.5833)
          }
        }
        UI::HelpMarker(TKLoc, "Ctrl + mouse scroll to adjust thumbnail size.");

        ImGui::TableNextColumn();
        if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_diskDriveIcon),
                               ImVec2(20.0f, 20.0f)))
        {
          g_app->SaveAllResources();
        }
        UI::HelpMarker(TKLoc, "Saves all resources.");

        ImGui::EndTable();

        ImGui::EndTabItem();
      }
    }

    void FolderView::SetPath(const String& path)
    {
      m_path = path;
      StringArray splits;
      Split(path, GetPathSeparatorAsStr(), splits);
      m_folder = splits.back();
    }

    const String& FolderView::GetPath() const { return m_path; }

    void FolderView::Iterate()
    {
      // Temporary vectors that holds DirectoryEntry's
      std::vector<DirectoryEntry> m_temp_dirs;
      std::vector<DirectoryEntry> m_temp_files;

      m_entries.clear();
      for (const std::filesystem::directory_entry& e :
           std::filesystem::directory_iterator(m_path))
      {
        DirectoryEntry de;
        de.m_isDirectory = e.is_directory();
        de.m_rootPath    = e.path().parent_path().u8string();
        de.m_fileName    = e.path().stem().u8string();
        de.m_ext         = e.path().filename().extension().u8string();

        // Do not show hidden files
        if (de.m_fileName.size() > 1 && de.m_fileName[0] == '.')
        {
          continue;
        }
        if (de.m_isDirectory)
        {
          m_temp_dirs.push_back(de);
        }
        else
        {
          m_temp_files.push_back(de);
        }
      }

      // Folder first, files next
      for (int i = 0; i < (int) m_temp_dirs.size(); i++)
      {
        m_entries.push_back(m_temp_dirs[i]);
      }

      for (int i = 0; i < (int) m_temp_files.size(); i++)
      {
        m_entries.push_back(m_temp_files[i]);
      }
    }

    int FolderView::Exist(const String& file, const String& ext)
    {
      for (int i = 0; i < (int) m_entries.size(); i++)
      {
        if (m_entries[i].m_fileName == file && m_entries[i].m_ext == ext)
        {
          return i;
        }
      }

      return -1;
    }

    void FolderView::ShowContextMenu(DirectoryEntry* entry)
    {
      StringArray commands;
      String path = m_path + GetPathSeparatorAsStr();
      if (path.find(MaterialPath("")) != String::npos)
      {
        commands.push_back("Material/Create");
      }
      else if (path.find(ScenePath("")) != String::npos)
      {
        commands.push_back("Scene/Create");
      }
      else if (path.find(PrefabPath("")) != String::npos)
      {
        commands.push_back("Scene/Create");
      }
      else if (path.find(LayerPath("")) != String::npos)
      {
        commands.push_back("Layer/Create");
      }

      if (ImGui::BeginPopupContextItem())
      {
        m_itemActions["FileSystem/Cut"](entry, this);
        m_itemActions["FileSystem/Copy"](entry, this);
        m_itemActions["FileSystem/Duplicate"](entry, this);
        m_itemActions["FileSystem/Delete"](entry, this);
        m_itemActions["FileSystem/Rename"](entry, this);

        ImGui::EndPopup();
      }

      if (ImGui::BeginPopupContextWindow(nullptr,
                                         ImGuiPopupFlags_MouseButtonRight |
                                             ImGuiPopupFlags_NoOpenOverItems))
      {
        for (const String& cmd : commands)
        {
          m_itemActions[cmd](entry, this);
        }

        m_itemActions["FileSystem/Paste"](nullptr, this);
        m_itemActions["FileSystem/MakeDir"](nullptr, this);
        m_itemActions["Refresh"](nullptr, this);
        m_itemActions["FileSystem/CopyPath"](nullptr, this);

        ImGui::EndPopup();
      }
    }

    void FolderView::Refresh() { m_dirty = true; }

    float FolderView::GetThumbnailZoomPercent(float thumbnailZoom)
    {
      float percent = (thumbnailZoom * 0.36f) - 8.f;
      return percent;
    }

    typedef std::vector<FolderView*> FolderViewRawPtrArray;

    void FolderView::CreateItemActions()
    {
      auto getSameViewsFn = [](FolderView* thisView) -> FolderViewRawPtrArray
      {
        // Always fetch the active view for self.
        FolderViewRawPtrArray list = {};
        for (FolderWindow* folder : g_app->GetAssetBrowsers())
        {
          if (folder->GetActiveView(true)->GetPath() == thisView->GetPath())
          {
            list.push_back(folder->GetActiveView(true));
          }
        }

        return list;
      };

      auto deleteDirFn = [this, getSameViewsFn](const String& path,
                                                FolderView* thisView) -> void
      {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
        if (ec)
        {
          g_app->m_statusMsg = ec.message();
        }

        for (FolderView* view : getSameViewsFn(thisView))
        {
          view->m_dirty = true;
        }
      };

      // Copy file path.
      m_itemActions["FileSystem/CopyPath"] =
          [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::MenuItem("CopyPath"))
        {
          int copied = SDL_SetClipboardText(views[0]->m_path.c_str());
          if (copied < 0)
          {
            // Error
            g_app->GetConsole()->AddLog(
                "Could not copy the folder path to clipboard",
                LogType::Error);
          }
          ImGui::CloseCurrentPopup();
        }
      };

      // Refresh.
      m_itemActions["Refresh"] = [getSameViewsFn](DirectoryEntry* entry,
                                                  FolderView* thisView) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::MenuItem("Refresh"))
        {
          for (FolderView* view : views)
          {
            view->m_dirty = true;
          }
          ImGui::CloseCurrentPopup();
        }
      };

      // FileSystem/MakeDir.
      m_itemActions["FileSystem/MakeDir"] =
          [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::MenuItem("MakeDir"))
        {
          StringInputWindow* inputWnd =
              new StringInputWindow("New Directory##NwDirName", true);
          inputWnd->m_inputLabel = "Name";
          inputWnd->m_hint       = "Directory name...";
          inputWnd
              ->m_illegalChars = {'/', ':', '*', '?', '"', '<', '>', '|', '\\'};

          inputWnd->m_taskFn   = [views, inputWnd](const String& val)
          {
            String file = ConcatPaths({views[0]->m_path, val});
            std::filesystem::create_directories(file);
            for (FolderView* view : views)
            {
              view->m_dirty = true;
            }
          };
          ImGui::CloseCurrentPopup();
        }
      };

      // FileSystem/Rename.
      m_itemActions["FileSystem/Rename"] =
          [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::MenuItem("Rename"))
        {
          if (ResourceManager* rm = entry->GetManager())
          {
            String oldName;
            String oldFile = entry->GetFullPath();
            DecomposePath(oldFile, nullptr, &oldName, nullptr);

            StringInputWindow* inputWnd =
                new StringInputWindow("New Name##NwName", true);
            inputWnd->m_inputVal   = oldName;
            inputWnd->m_inputLabel = "New Name";
            inputWnd->m_hint       = "New name...";

            inputWnd->m_taskFn     = [views, oldFile](const String& val)
            {
              String path, ext;
              DecomposePath(oldFile, &path, nullptr, &ext);

              String file = ConcatPaths({path, val + ext});
              if (CheckFile(file))
              {
                g_app->GetConsole()->AddLog(
                    "Can't rename. A file with the same name exist",
                    LogType::Error);
              }
              else
              {
                std::filesystem::rename(oldFile, file);
                for (FolderView* view : views)
                {
                  view->m_dirty = true;
                }
              }
            };
          }

          ImGui::CloseCurrentPopup();
        }
      };

      // FileSystem/Delete.
      m_itemActions["FileSystem/Delete"] =
          [getSameViewsFn, deleteDirFn](DirectoryEntry* entry,
                                        FolderView* thisView) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::MenuItem("Delete"))
        {
          if (entry->m_isDirectory)
          {
            deleteDirFn(entry->GetFullPath(), thisView);
          }
          else
          {
            if (ResourceManager* rm = entry->GetManager())
            {
              rm->Remove(entry->GetFullPath());
            }

            std::filesystem::remove(entry->GetFullPath());
            for (FolderView* view : views)
            {
              view->m_dirty = true;
            }
          }

          ImGui::CloseCurrentPopup();
        }
      };

      // FileSystem/Copy.
      m_itemActions["FileSystem/Duplicate"] =
          [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::MenuItem("Duplicate"))
        {
          String fullPath = entry->GetFullPath();
          String cpyPath  = CreateCopyFileFullPath(fullPath);
          std::filesystem::copy(fullPath, cpyPath);

          for (FolderView* view : views)
          {
            view->m_dirty = true;
          }
          ImGui::CloseCurrentPopup();
        }
      };

      m_itemActions["FileSystem/Cut"] = [](DirectoryEntry* entry,
                                           FolderView* thisView) -> void
      {
        if (ImGui::MenuItem("Cut"))
        {
          m_currentEntry   = entry;
          entry->m_cutting = true;
          ImGui::CloseCurrentPopup();
        }
      };

      m_itemActions["FileSystem/Copy"] = [](DirectoryEntry* entry,
                                            FolderView* thisView) -> void
      {
        if (ImGui::MenuItem("Copy"))
        {
          m_currentEntry = entry;
          m_currentEntry->m_cutting = false;
          ImGui::CloseCurrentPopup();
        }
      };

      m_itemActions["FileSystem/Paste"] =
          [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
      {
        if (ImGui::MenuItem("Paste"))
        {
          if (m_currentEntry != nullptr)
          {
            String src = m_currentEntry->GetFullPath();
            String dst = ConcatPaths(
                {thisView->m_path,
                 m_currentEntry->m_fileName + m_currentEntry->m_ext});

            if (m_currentEntry->m_cutting)
            {
              // move file to its new position
              if (std::rename(src.c_str(), dst.c_str()))
              {
                GetLogger()->WriteConsole(LogType::Error,
                                          "file cut paste failed!");
              }
              m_currentEntry->m_cutting = false;
            }
            else
            {
              std::filesystem::copy(src, dst);
            }

            m_currentEntry = nullptr;
            // refresh all folder views
            for (FolderWindow* window : g_app->GetAssetBrowsers())
            {
              window->SetViewsDirty();
            }
          }
          ImGui::CloseCurrentPopup();
        }
      };

      auto sceneCreateFn = [getSameViewsFn](DirectoryEntry* entry,
                                            FolderView* thisView,
                                            const String& extention) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::MenuItem("Create"))
        {
          StringInputWindow* inputWnd =
              new StringInputWindow("Scene Name##ScnMat", true);
          inputWnd->m_inputVal   = "New Scene";
          inputWnd->m_inputLabel = "Name";
          inputWnd->m_hint       = "New scene name...";
          inputWnd->m_taskFn     = [views, extention](const String& val)
          {
            String file = ConcatPaths({views[0]->m_path, val + extention});
            if (CheckFile(file))
            {
              g_app->GetConsole()->AddLog(
                  "Can't create. A scene with the same name exist",
                  LogType::Error);
            }
            else
            {
              ScenePtr scene = std::make_shared<Scene>(file);
              scene->Save(false);
              for (FolderView* view : views)
              {
                view->m_dirty = true;
              }
            }
          };
          ImGui::CloseCurrentPopup();
        }
      };

      // Scene/Create.
      m_itemActions["Scene/Create"] =
          [sceneCreateFn](DirectoryEntry* entry, FolderView* thisView) -> void
      { sceneCreateFn(entry, thisView, SCENE); };

      // Layer/Create.
      m_itemActions["Layer/Create"] =
          [sceneCreateFn](DirectoryEntry* entry, FolderView* thisView) -> void
      { sceneCreateFn(entry, thisView, LAYER); };

      // Material/Create.
      m_itemActions["Material/Create"] =
          [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::MenuItem("Create"))
        {
          StringInputWindow* inputWnd =
              new StringInputWindow("Material Name##NwMat", true);
          inputWnd->m_inputVal   = "New Material";
          inputWnd->m_inputLabel = "Name";
          inputWnd->m_hint       = "New material name";
          inputWnd->m_taskFn     = [views](const String& val)
          {
            String file = ConcatPaths({views[0]->m_path, val + MATERIAL});
            if (CheckFile(file))
            {
              g_app->GetConsole()->AddLog(
                  "Can't create. A material with the same name exist",
                  LogType::Error);
            }
            else
            {
              MaterialManager* man = GetMaterialManager();
              MaterialPtr mat      = man->GetCopyOfDefaultMaterial();
              mat->m_name          = val;
              mat->SetFile(file);
              for (FolderView* view : views)
              {
                view->m_dirty = true;
              }
              mat->Save(true);
              man->Manage(mat);
            }
          };
          ImGui::CloseCurrentPopup();
        }
      };
    }

    void FolderView::MoveTo(const String& dst)
    {
      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("BrowserDragZone"))
        {
          IM_ASSERT(payload->DataSize == sizeof(DirectoryEntry));
          DirectoryEntry entry = *(const DirectoryEntry*) payload->Data;
          String newPath = ConcatPaths({dst, entry.m_fileName + entry.m_ext});

          std::error_code ec;
          std::filesystem::rename(entry.GetFullPath(), newPath, ec);
          if (ec)
          {
            g_app->m_statusMsg = ec.message();
          }
          else
          {
            // Update src & dst views.
            String src = entry.m_rootPath;
            if (src == m_path)
            {
              // Item moved across tabs.
              src = dst;
            }

            int indx = m_parent->Exist(src);
            if (indx != -1)
            {
              m_parent->GetView(indx).m_dirty = true;
            }
            m_dirty = true;
          }
        }
        ImGui::EndDragDropTarget();
      }
    }

    FolderWindow::FolderWindow(XmlNode* node)
    {
      DeSerialize(nullptr, node);
      Iterate(ResourcePath(), true);
    }

    FolderWindow::FolderWindow(bool addEngine)
    {
      Iterate(ResourcePath(), true, addEngine);
    }

    FolderWindow::~FolderWindow() {}

    void FolderWindow::Show()
    {
      ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        auto IsRootFn = [](const String& path)
        {
          size_t lastSep = path.find_last_of(GetPathSeparator());
          if (lastSep != String::npos)
          {
            String root = path.substr(0, lastSep);
            String end  = path.substr(lastSep, path.size());
            static String test =
                String(1, GetPathSeparator()) + String("Engine");
            return root == ResourcePath() || !end.compare(test);
          }

          return false;
        };

        if (!g_app->m_workspace.GetActiveWorkspace().empty() &&
            g_app->m_workspace.GetActiveProject().name.empty())
        {
          ImGui::Text("Load a project.");
          ImGui::End();
          return;
        }

        if (m_showStructure)
        {
          // Show Resource folder structure.
          ImGui::PushID("##FolderStructure");
          ImGui::BeginGroup();

          ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign,
                              ImVec2(0.0f, 0.5f));
          ImGui::TextUnformatted("Resources");

          ImGui::SameLine();
          if (ImGui::Button("<<"))
          {
            m_showStructure = !m_showStructure;
          }

          ImGui::BeginChild("##Folders", ImVec2(130, 0), true);
          for (int i = 0; i < static_cast<int>(m_entries.size()); i++)
          {
            if (!IsRootFn(m_entries[i].GetPath()))
            {
              continue;
            }

            bool currSel = false;
            if (m_activeFolder == i)
            {
              currSel = true;
            }

            currSel = UI::ToggleButton(m_entries[i].m_folder,
                                       ImVec2(100, 25),
                                       currSel);

            // Selection switch.
            if (currSel)
            {
              if (i != m_activeFolder)
              {
                for (FolderView& view : m_entries)
                {
                  view.m_visible = false;
                }
              }

              m_activeFolder = i;
            }
          }
          ImGui::EndChild();

          ImGui::PopStyleVar();
          ImGui::EndGroup();
          ImGui::PopID();
        }
        else
        {
          if (ImGui::Button(">>"))
          {
            m_showStructure = !m_showStructure;
          }
        }

        ImGui::SameLine();

        ImGui::PushID("##FolderContent");
        ImGui::BeginGroup();
        if (ImGui::BeginTabBar("Folders",
                               ImGuiTabBarFlags_NoTooltip |
                                   ImGuiTabBarFlags_AutoSelectNewTabs))
        {
          String currRootPath;
          auto IsDescendentFn = [&currRootPath](String candidate) -> bool
          {
            return !currRootPath.empty() &&
                   candidate.find(currRootPath) != std::string::npos;
          };

          for (int i = 0; i < static_cast<int>(m_entries.size()); i++)
          {
            FolderView& view = m_entries[i];
            String candidate = view.GetPath();
            if ((view.m_currRoot = (i == m_activeFolder)))
            {
              currRootPath = candidate;
            }

            // Show only current root folder or descendents.
            if (view.m_currRoot || IsDescendentFn(candidate))
            {
              view.Show();
            }
          }
          ImGui::EndTabBar();
        }

        ImGui::EndGroup();
        ImGui::PopID();
      }

      ImGui::End();
    }

    Window::Type FolderWindow::GetType() const { return Window::Type::Browser; }

    void FolderWindow::Iterate(const String& path, bool clear, bool addEngine)
    {
      if (clear)
      {
        m_entries.clear();
      }

      String resourceRoot = ResourcePath();
      char pathSep        = GetPathSeparator();
      int baseCount       = CountChar(resourceRoot, pathSep);

      for (const std::filesystem::directory_entry& entry :
           std::filesystem::directory_iterator(path))
      {
        if (entry.is_directory())
        {
          FolderView view(this);
          String path = entry.path().u8string();
          view.m_root = CountChar(path, pathSep) == baseCount + 1;

          view.SetPath(path);
          if (!view.m_folder.compare("Engine"))
          {
            continue;
          }

          if (m_viewSettings.find(path) != m_viewSettings.end())
          {
            ViewSettings vs     = m_viewSettings[path];
            view.m_iconSize     = vs.size;
            view.m_visible      = vs.visible;
            view.m_activateNext = vs.active;
          }

          view.Iterate();
          m_entries.push_back(view);
          Iterate(view.GetPath(), false, false);
        }
      }

      if (addEngine)
      {
        // Engine folder
        FolderView view(this);
        view.SetPath(DefaultAbsolutePath());

        if (m_viewSettings.find(path) != m_viewSettings.end())
        {
          ViewSettings vs     = m_viewSettings[path];
          view.m_iconSize     = vs.size;
          view.m_visible      = vs.visible;
          view.m_activateNext = vs.active;
        }

        view.Iterate();
        m_entries.push_back(view);
        Iterate(view.GetPath(), false, false);
      }
    }

    void FolderWindow::UpdateContent()
    {
      for (FolderView& view : m_entries)
      {
        view.Iterate();
      }
    }

    void FolderWindow::AddEntry(const FolderView& view)
    {
      if (Exist(view.GetPath()) == -1)
      {
        m_entries.push_back(view);
      }
    }

    void FolderWindow::SetViewsDirty()
    {
      for (FolderView& view : m_entries)
      {
        view.SetDirty();
      }
    }

    FolderView& FolderWindow::GetView(int indx) { return m_entries[indx]; }

    FolderView* FolderWindow::GetActiveView(bool deep)
    {
      if (m_activeFolder == -1)
      {
        return nullptr;
      }

      if (deep)
      {
        FolderView& rootView = GetView(m_activeFolder);
        for (FolderView& view : m_entries)
        {
          if (view.m_active && view.m_visible)
          {
            return &view;
          }
        }
      }

      return &GetView(m_activeFolder);
    }

    void FolderWindow::SetActiveView(FolderView* view)
    {
      int viewIndx = Exist(view->GetPath());
      if (viewIndx != -1)
      {
        for (FolderView& view : m_entries)
        {
          view.m_active = false;
        }
        m_entries[viewIndx].m_active = true;
      }
    }

    int FolderWindow::Exist(const String& folder)
    {
      for (size_t i = 0; i < m_entries.size(); i++)
      {
        if (m_entries[i].GetPath() == folder)
        {
          return static_cast<int>(i);
        }
      }

      return -1;
    }

    bool FolderWindow::GetFileEntry(const String& fullPath,
                                    DirectoryEntry& entry)
    {
      String path, name, ext;
      DecomposePath(fullPath, &path, &name, &ext);
      int viewIndx = Exist(path);
      if (viewIndx != -1)
      {
        int dirEntry = m_entries[viewIndx].Exist(name, ext);
        if (dirEntry != -1)
        {
          entry = m_entries[viewIndx].m_entries[dirEntry];
          return true;
        }
      }

      return false;
    }

    void FolderWindow::Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      Window::Serialize(doc, parent);
      XmlNode* node = parent->last_node();

      XmlNode* folder =
          doc->allocate_node(rapidxml::node_element, "FolderWindow");
      node->append_node(folder);
      WriteAttr(folder, doc, "activeFolder", std::to_string(m_activeFolder));
      WriteAttr(folder, doc, "showStructure", std::to_string(m_showStructure));

      for (const FolderView& view : m_entries)
      {
        XmlNode* viewNode =
            doc->allocate_node(rapidxml::node_element, "FolderView");
        WriteAttr(viewNode, doc, XmlNodePath.data(), view.GetPath());
        WriteAttr(viewNode, doc, "vis", std::to_string(view.m_visible));
        WriteAttr(viewNode, doc, "active", std::to_string(view.m_active));
        folder->append_node(viewNode);
        XmlNode* setting =
            doc->allocate_node(rapidxml::node_element, "IconSize");
        WriteVec(setting, doc, view.m_iconSize);
        viewNode->append_node(setting);
      }
    }

    void FolderWindow::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      Window::DeSerialize(doc, parent);
      if (XmlNode* node = parent->first_node("FolderWindow"))
      {
        ReadAttr(node, "activeFolder", m_activeFolder);
        ReadAttr(node, "showStructure", m_showStructure);

        if (XmlNode* view = node->first_node("FolderView"))
        {
          do
          {
            String path;
            ReadAttr(view, XmlNodePath.data(), path);

            ViewSettings vs;
            vs.visible = false;
            ReadAttr(view, "vis", vs.visible);

            vs.active = false;
            ReadAttr(view, "active", vs.active);

            vs.size = Vec2(50.0f);
            if (XmlNode* setting = view->first_node("IconSize"))
            {
              ReadVec(setting, vs.size);
            }

            m_viewSettings[path] = vs;
          } while (view = view->next_sibling("FolderView"));
        }
      }
    }

  } // namespace Editor
} // namespace ToolKit
