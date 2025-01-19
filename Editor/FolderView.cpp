/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "App.h"
#include "MaterialView.h"
#include "PopupWindows.h"

#include <Material.h>
#include <Mesh.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    FolderView::FolderView() { CreateItemActions(); }

    FolderView::~FolderView() {}

    FolderView::FolderView(class FolderWindow* parent) : FolderView() { m_parent = parent; }

    static FileDragData g_fileDragData {};
    static std::vector<DirectoryEntry*> g_selectedFiles {};
    static std::vector<DirectoryEntry*> g_coppiedFiles {};
    static bool g_carryingFiles = false;
    static bool g_copyingFiles = false, g_cuttingFiles = false;

    FolderView* g_dragBeginView = nullptr;

    const FileDragData& FolderView::GetFileDragData() { return g_fileDragData; }

    void FolderView::DrawSearchBar()
    {
      // Handle Item Icon size.
      ImGuiIO io                 = ImGui::GetIO();
      float delta                = io.MouseWheel;

      // Initial zoom value
      static float thumbnailZoom = m_thumbnailMaxZoom / 6.f;

      // Zoom in and out
      if (io.KeyCtrl && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
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
      m_iconSize = Vec2(thumbnailZoom);

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
      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_viewZoomIcon), ImVec2(20.0f, 20.0f)))
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
      if (ImGui::ImageButton(Convert2ImGuiTexture(UI::m_diskDriveIcon), ImVec2(20.0f, 20.0f)))
      {
        g_app->SaveAllResources();
      }
      UI::HelpMarker(TKLoc, "Saves all resources.");

      ImGui::EndTable();
    }

    void FolderView::PasteFiles(const String& path)
    {
      for (size_t i = 0ull; i < g_coppiedFiles.size(); ++i)
      {
        DirectoryEntry* entry = g_coppiedFiles[i];
        String src            = entry->GetFullPath();
        String dst            = ConcatPaths({path, entry->m_fileName + entry->m_ext});

        if (g_copyingFiles)
        {
          std::error_code err;
          std::filesystem::copy(src, dst, err);
          if (err)
          {
            TK_ERR("Copy failed: %s", err.message().c_str());
            g_app->SetStatusMsg(g_statusFailed);
          }
        }
        else if (g_cuttingFiles)
        {
          // move file to its new position
          if (std::rename(src.c_str(), dst.c_str()))
          {
            TK_ERR("File cut & paste failed!");
            g_app->SetStatusMsg(g_statusFailed);
          }
        }
      }
      g_copyingFiles = g_cuttingFiles = false;
      g_coppiedFiles.clear();

      // refresh all folder views
      for (FolderWindow* window : g_app->GetAssetBrowsers())
      {
        window->SetViewsDirty();
      }
    }

    void FolderView::HandleCopyPasteDelete()
    {
      bool ctrlDown = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
      if (ctrlDown && ImGui::IsKeyPressed(ImGuiKey_C))
      {
        g_dragBeginView = this;
        g_copyingFiles  = true;
        g_coppiedFiles  = g_selectedFiles;
      }

      if (ctrlDown && ImGui::IsKeyPressed(ImGuiKey_X))
      {
        g_dragBeginView = this;
        g_cuttingFiles  = true;
        g_coppiedFiles  = g_selectedFiles;
      }

      if (ImGui::IsKeyPressed(ImGuiKey_Delete))
      {
        for (size_t i = 0ull; i < g_selectedFiles.size(); ++i)
        {
          String path = g_selectedFiles[i]->GetFullPath();
          if (!CheckFile(path))
          {
            continue;
          }
          std::filesystem::remove(path);
        }
        g_selectedFiles.clear();
        // refresh all folder views
        for (FolderWindow* window : g_app->GetAssetBrowsers())
        {
          window->SetViewsDirty();
        }
      }

      if (ImGui::IsKeyPressed(ImGuiKey_V) && g_dragBeginView != nullptr &&
          g_dragBeginView != this) // be sure we are not dropping to same file
      {
        PasteFiles(m_path);
      }
    }

    int FolderView::SelectFolder(FolderWindow* parent, const String& path)
    {
      int selected = parent->Exist(path);
      if (selected == -1)
      {
        FolderView view(parent);
        view.SetPath(path);
        view.Iterate();
        view.Refresh();
        parent->AddEntry(view);
        selected = parent->Exist(path);
      }

      parent->SetActiveView(selected);

      return selected;
    }

    void FolderView::SelectFilesInRange(int a, int b)
    {
      // return if folders same or indices invalid
      if (a >= m_entries.size() || b >= m_entries.size() || a < 0 || b < 0 ||
          m_entries[a].GetFullPath() == m_entries[b].GetFullPath())
      {
        return;
      }

      // swap if a is grater than b.
      // we want a to be smaller bacause of for loop
      if (a > b)
      {
        std::swap(a, b);
      }

      for (int i = a; i <= b; i++)
      {
        if (!contains(g_selectedFiles, m_entries.data() + i))
        {
          g_selectedFiles.push_back(m_entries.data() + i);
        }
      }
    }

    void FolderView::DetermineAndSetBackgroundColor(bool isSelected, int i)
    {
      ImVec4 hoverColor     = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
      ImVec4 buttonColor    = ImGui::GetStyleColorVec4(ImGuiCol_Button);

      bool copyingOrCutting = contains(g_coppiedFiles, m_entries.data() + i);

      // determinate background color of the file
      // cutting files will shown red,
      // coppying files will shown orange,
      // selected files will shown as blue
      if (copyingOrCutting)
      {
        if (g_cuttingFiles)
        {
          buttonColor = ImVec4(0.72f, 0.2f, 0.2f, 0.82f);
          hoverColor  = ImVec4(0.51f, 0.1f, 0.1f, 0.81f);
        }
        else if (g_copyingFiles)
        {
          buttonColor = ImVec4(0.72f, 0.72f, 0.2f, 0.82f);
          hoverColor  = ImVec4(0.51f, 0.51f, 0.1f, 0.81f);
        }
      }
      else if (isSelected == true)
      {
        buttonColor = ImVec4(0.3f, 0.4f, 0.7f, 0.5f);
        hoverColor  = ImVec4(0.4f, 0.5f, 0.8f, 1.0f);
      }

      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
      ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    }

    void FolderView::Show()
    {
      ImGuiTabItemFlags flags = m_active ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
      if (ImGui::BeginTabItem(m_folder.c_str(), nullptr, flags))
      {
        if (!m_active) // If this view is not active and imgui try to show it,
        {
          if (m_visible) // Either this window lost its activity via tree view
          {
            m_visible = false;
            ImGui::EndTabItem();
            return;
          }
          else // Or Activated via clicking on tab
          {
            m_parent->SetActiveView(this);
          }
        }

        m_visible = true;

        DrawSearchBar();

        if (m_dirty)
        {
          Iterate();
          m_dirty = false;
        }

        // Item dropped to tab.
        MoveTo(m_path);

        // Start drawing folder items.
        const float footerHeightReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("##Content");

        HandleCopyPasteDelete();

        if (m_entries.empty())
        {
          // Handle context menu based on path / content type of the folder.
          ShowContextMenu();
          ImGui::EndChild();
          ImGui::EndTabItem();
          return;
        }

        bool anyButtonClicked = false;
        // Draw folder items.
        for (int i = 0; i < (int) m_entries.size(); i++)
        {
          // Prepare Item Icon.
          ImGuiStyle& style      = ImGui::GetStyle();
          float visX2            = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

          DirectoryEntry& dirEnt = m_entries[i];

          if (m_filter.size() > 0 && !Utf8CaseInsensitiveSearch(dirEnt.m_fileName, m_filter))
          {
            continue;
          }

          bool flipRenderTarget = false;
          uint iconId           = UI::m_fileIcon->m_textureId;

          std::unordered_map<String, uint> extensionIconMap {
              {SCENE,    UI::m_worldIcon->m_textureId},
              {LAYER,    UI::m_worldIcon->m_textureId},
              {ANIM,     UI::m_clipIcon->m_textureId },
              {WAW,      UI::m_audioIcon->m_textureId},
              {MP3,      UI::m_audioIcon->m_textureId},
              {SHADER,   UI::m_codeIcon->m_textureId },
              {LAYER,    UI::m_worldIcon->m_textureId},
              {SKELETON, UI::m_boneIcon->m_textureId }
          };

          static std::unordered_set<String>
              thumbExtensions {PNG, JPG, JPEG, TGA, BMP, PSD, HDR, MESH, SKINMESH, MATERIAL, WAW, MP3};

          if (dirEnt.m_isDirectory)
          {
            iconId = UI::m_folderIcon->m_textureId;
          }
          else if (extensionIconMap.count(dirEnt.m_ext) > 0)
          {
            iconId = extensionIconMap[dirEnt.m_ext];
          }
          else if (thumbExtensions.count(dirEnt.m_ext) > 0)
          {
            if (g_app->m_thumbnailManager.TryGetThumbnail(iconId, dirEnt))
            {
              flipRenderTarget = true;
            }
            else
            {
              iconId = UI::m_imageIcon->m_textureId;
            }
          }
          else if (m_onlyNativeTypes)
          {
            continue;
          }

          ImGui::PushID(i);
          ImGui::BeginGroup();
          ImVec2 texCoords         = flipRenderTarget ? ImVec2(1.0f, -1.0f) : ImVec2(1.0f, 1.0f);

          DirectoryEntry* entryPtr = m_entries.data() + i;

          bool isSelected          = contains(g_selectedFiles, entryPtr);
          // this function will push color. we are popping it down below this if block.
          DetermineAndSetBackgroundColor(isSelected, i);

          // Draw Item Icon.
          if (ImGui::ImageButton(ConvertUIntImGuiTexture(iconId), m_iconSize, ImVec2(0.0f, 0.0f), texCoords))
          {
            anyButtonClicked |= true;
            bool shiftDown    = ImGui::IsKeyDown(ImGuiKey_LeftShift);
            bool ctrlDown     = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
            // handle multi selection and input
            if (!shiftDown && !ctrlDown)
            {
              // this means not multi selecting so select only this.
              g_selectedFiles.clear();
              g_selectedFiles.push_back(entryPtr);
            }
            else if (ctrlDown && isSelected)
            {
              erase_if(g_selectedFiles, [entryPtr](DirectoryEntry* other) -> bool { return other == entryPtr; });
            }
            else if (shiftDown && m_lastClickedEntryIdx != -1)
            {
              SelectFilesInRange(m_lastClickedEntryIdx, i);
            }
            else
            {
              g_selectedFiles.push_back(entryPtr);
            }

            m_lastClickedEntryIdx = i;
          }

          if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
          {
            if (ResourceManager* rm = dirEnt.GetManager())
            {
              if (rm->m_baseType == Material::StaticClass())
              {
                MaterialPtr mat                  = rm->Create<Material>(dirEnt.GetFullPath());
                MaterialWindowPtr materialWindow = MakeNewPtr<MaterialWindow>();
                materialWindow->SetMaterial(mat);
                materialWindow->AddToUI();
              }
              else if (rm->m_baseType == Mesh::StaticClass())
              {
                g_app->GetPropInspector()->SetMeshView(rm->Create<Mesh>(dirEnt.GetFullPath()));
              }
              else if (rm->m_baseType == SkinMesh::StaticClass())
              {
                g_app->GetPropInspector()->SetMeshView(rm->Create<SkinMesh>(dirEnt.GetFullPath()));
              }
            }
          }

          // pop colors that comming from DeterminateAndSetBackgroundColor
          // function
          ImGui::PopStyleColor(2);

          // Handle context menu.
          ShowContextMenu(&dirEnt);

          // Handle if item is directory.
          if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
          {
            if (ImGui::IsItemHovered() && dirEnt.m_isDirectory && m_parent != nullptr)
            {
              String path = ConcatPaths({dirEnt.m_rootPath, dirEnt.m_fileName});
              SelectFolder(m_parent, path);
            }
          }

          // Handle mouse hover tips.
          String fullName = dirEnt.m_fileName + dirEnt.m_ext;
          UI::HelpMarker(TKLoc + fullName, fullName.c_str());

          // Handle drag - drop to scene / inspector.
          if (!dirEnt.m_isDirectory)
          {
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
              // if the file that we are holding is not selected
              if (!isSelected)
              {
                // add to selection
                g_selectedFiles.push_back(&dirEnt);
              }
              g_fileDragData.Entries  = g_selectedFiles.data();
              g_fileDragData.NumFiles = (int) g_selectedFiles.size();
              g_dragBeginView         = this;
              g_carryingFiles         = true;

              ImGui::SetDragDropPayload("BrowserDragZone", &g_fileDragData, sizeof(FileDragData));
              ImGui::ImageButton(ConvertUIntImGuiTexture(iconId), m_iconSize, ImVec2(0.0f, 0.0f), texCoords);
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
          ImGui::TextWrapped("%s", dirEnt.m_fileName.c_str());

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
        } // Tab item handling ends(for loop).

        bool mouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

        // mouse released on empty position in this window
        if (!anyButtonClicked && mouseReleased)
        {
          if (g_carryingFiles == true && g_dragBeginView != nullptr)
          {
            g_dragBeginView->DropFiles(m_path);
          }
          if (!ImGui::IsKeyDown(ImGuiKey_LeftShift) && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
          {
            g_selectedFiles.clear();
          }
        }

        g_carryingFiles = mouseReleased ? false : g_carryingFiles;

        ImGui::EndChild();

        ImGui::EndTabItem();
      }
    }

    void FolderView::SetDirty() { m_dirty = true; }

    void FolderView::SetPath(const String& path)
    {
      m_path = path;
      StringArray splits;
      Split(path, GetPathSeparatorAsStr(), splits);
      m_folder = splits.back();
    }

    const String& FolderView::GetPath() const { return m_path; }

    String FolderView::GetRoot() const
    {
      String root;
      String searchStr   = "Resources" + GetPathSeparatorAsStr();
      size_t resourceLoc = m_path.find(searchStr);
      if (resourceLoc != String::npos)
      {
        resourceLoc += searchStr.length();
        root         = m_path.substr(0, resourceLoc);
        resourceLoc  = m_path.find(GetPathSeparator(), resourceLoc);
        root         = m_path.substr(0, resourceLoc);
      }

      return root;
    }

    void FolderView::Iterate()
    {
      // Temporary vectors that holds DirectoryEntry's
      std::vector<DirectoryEntry> m_temp_dirs;
      std::vector<DirectoryEntry> m_temp_files;

      m_entries.clear();
      for (const std::filesystem::directory_entry& e : std::filesystem::directory_iterator(m_path))
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

    bool FolderView::IsMultiSelecting()
    {
      bool shiftDown = ImGui::IsKeyDown(ImGuiKey_LeftShift);
      bool ctrlDown  = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
      return shiftDown || ctrlDown;
    }

    void FolderView::ShowContextMenu(DirectoryEntry* entry)
    {
      StringArray commands;
      String path = m_path + GetPathSeparatorAsStr();

      if (ImGui::IsKeyPressed(ImGuiKey_Escape))
      {
        g_selectedFiles.clear();
        g_coppiedFiles.clear();
        g_dragBeginView = nullptr;
        g_carryingFiles = g_copyingFiles = g_cuttingFiles = false;
      }

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
        bool isSelected = contains(g_selectedFiles, entry);

        if (IsMultiSelecting())
        {
          g_selectedFiles.push_back(entry);
        }
        else if (!isSelected)
        {
          // if not multiselecting clear all selected files and push this
          g_selectedFiles.clear();
          g_selectedFiles.push_back(entry);
        }

        m_itemActions["FileSystem/Cut"](entry, this);
        m_itemActions["FileSystem/Copy"](entry, this);
        m_itemActions["FileSystem/Duplicate"](entry, this);
        m_itemActions["FileSystem/Delete"](entry, this);
        m_itemActions["FileSystem/Rename"](entry, this);

        ImGui::EndPopup();
      }

      if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
      {
        for (const String& cmd : commands)
        {
          m_itemActions[cmd](entry, this);
        }
        if (g_copyingFiles || g_cuttingFiles)
        {
          m_itemActions["FileSystem/Paste"](nullptr, this);
        }
        m_itemActions["FileSystem/MakeDir"](nullptr, this);
        m_itemActions["Refresh"](nullptr, this);
        m_itemActions["FileSystem/Show In Explorer"](nullptr, this);

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
          if (FolderView* activeView = folder->GetActiveView())
          {
            if (activeView->GetPath() == thisView->GetPath())
            {
              list.push_back(activeView);
            }
          }
        }

        return list;
      };

      auto deleteDirFn = [getSameViewsFn](const String& path, FolderView* thisView) -> void
      {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
        if (ec)
        {
          TK_ERR("Delete failed: %s", ec.message().c_str());
          g_app->SetStatusMsg(g_statusFailed);
        }

        for (FolderView* view : getSameViewsFn(thisView))
        {
          view->m_dirty = true;
        }
      };

      // Copy file path.
      m_itemActions["FileSystem/Show In Explorer"] = [getSameViewsFn](DirectoryEntry* entry,
                                                                      FolderView* thisView) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::MenuItem("Show In Explorer"))
        {
          g_app->m_shellOpenDirFn(views[0]->m_path.c_str());
          ImGui::CloseCurrentPopup();
        }
      };

      // Refresh.
      m_itemActions["Refresh"] = [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
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
          thisView->m_parent->ReconstructFolderTree();
          ImGui::CloseCurrentPopup();
        }
      };

      // FileSystem/MakeDir.
      m_itemActions["FileSystem/MakeDir"] = [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::MenuItem("MakeDir"))
        {
          StringInputWindowPtr inputWnd = MakeNewPtr<StringInputWindow>("New Directory##NwDirName", true);
          inputWnd->m_inputLabel        = "Name";
          inputWnd->m_hint              = "Directory name...";
          inputWnd->m_illegalChars      = {'/', ':', '*', '?', '"', '<', '>', '|', '\\'};

          inputWnd->m_taskFn            = [views, inputWnd](const String& val)
          {
            String file = ConcatPaths({views[0]->m_path, val});
            std::filesystem::create_directories(file);
            for (FolderView* view : views)
            {
              view->m_dirty = true;
            }
          };
          inputWnd->AddToUI();

          thisView->m_parent->ReconstructFolderTree();
          ImGui::CloseCurrentPopup();
        }
      };

      // FileSystem/Rename.
      m_itemActions["FileSystem/Rename"] = [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
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

            StringInputWindowPtr inputWnd = MakeNewPtr<StringInputWindow>("New Name##NwName", true);
            inputWnd->m_inputVal          = oldName;
            inputWnd->m_inputLabel        = "New Name";
            inputWnd->m_hint              = "New name...";
            inputWnd->AddToUI();

            inputWnd->m_taskFn = [views, oldFile](const String& val)
            {
              String path, ext;
              DecomposePath(oldFile, &path, nullptr, &ext);

              String file = ConcatPaths({path, val + ext});
              if (CheckFile(file))
              {
                g_app->GetConsole()->AddLog("Can't rename. A file with the same name exist", LogType::Error);
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
      m_itemActions["FileSystem/Delete"] = [getSameViewsFn, deleteDirFn](DirectoryEntry* entry,
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
            thisView->m_parent->ReconstructFolderTree();
          }
          else
          {
            for (size_t i = 0ull; i < g_selectedFiles.size(); ++i)
            {
              DirectoryEntry* selected = g_selectedFiles[i];
              if (ResourceManager* rm = selected->GetManager())
              {
                rm->Remove(selected->GetFullPath());
              }

              std::filesystem::remove(selected->GetFullPath());
            }

            for (FolderView* view : views)
            {
              view->SetDirty();
            }
          }

          ImGui::CloseCurrentPopup();
        }
      };

      // FileSystem/Copy.
      m_itemActions["FileSystem/Duplicate"] = [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
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
          thisView->m_parent->ReconstructFolderTree();

          for (FolderView* view : views)
          {
            view->m_dirty = true;
          }
          ImGui::CloseCurrentPopup();
        }
      };

      m_itemActions["FileSystem/Cut"] = [](DirectoryEntry* entry, FolderView* thisView) -> void
      {
        if (ImGui::MenuItem("Cut"))
        {
          entry->m_cutting = true;
          g_coppiedFiles   = g_selectedFiles;
          g_cuttingFiles   = true;
          ImGui::CloseCurrentPopup();
        }
      };

      m_itemActions["FileSystem/Copy"] = [](DirectoryEntry* entry, FolderView* thisView) -> void
      {
        if (ImGui::MenuItem("Copy"))
        {
          g_coppiedFiles = g_selectedFiles;
          g_copyingFiles = true;
          ImGui::CloseCurrentPopup();
        }
      };

      m_itemActions["FileSystem/Paste"] = [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
      {
        if (ImGui::MenuItem("Paste"))
        {
          PasteFiles(thisView->m_path);
          thisView->m_parent->ReconstructFolderTree();
          ImGui::CloseCurrentPopup();
        }
      };

      auto sceneCreateFn =
          [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView, const String& extention) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::MenuItem("Create"))
        {
          StringInputWindowPtr inputWnd = MakeNewPtr<StringInputWindow>("Scene Name##ScnMat", true);
          inputWnd->m_inputVal          = "New Scene";
          inputWnd->m_inputLabel        = "Name";
          inputWnd->m_hint              = "New scene name...";
          inputWnd->AddToUI();

          inputWnd->m_taskFn = [views, extention](const String& val)
          {
            String file = ConcatPaths({views[0]->m_path, val + extention});
            if (CheckFile(file))
            {
              g_app->GetConsole()->AddLog("Can't create. A scene with the same name exist", LogType::Error);
            }
            else
            {
              ScenePtr scene = MakeNewPtr<Scene>();
              scene->SetFile(file);
              scene->Save(false);
              for (FolderView* view : views)
              {
                view->m_dirty = true;
              }
            }
          };
          thisView->m_parent->ReconstructFolderTree();
          ImGui::CloseCurrentPopup();
        }
      };

      // Scene/Create.
      m_itemActions["Scene/Create"] = [sceneCreateFn](DirectoryEntry* entry, FolderView* thisView) -> void
      { sceneCreateFn(entry, thisView, SCENE); };

      // Layer/Create.
      m_itemActions["Layer/Create"] = [sceneCreateFn](DirectoryEntry* entry, FolderView* thisView) -> void
      { sceneCreateFn(entry, thisView, LAYER); };

      // Material/Create.
      m_itemActions["Material/Create"] = [getSameViewsFn](DirectoryEntry* entry, FolderView* thisView) -> void
      {
        FolderViewRawPtrArray views = getSameViewsFn(thisView);
        if (views.size() == 0)
        {
          return;
        }

        if (ImGui::BeginMenu("Create"))
        {
          auto inputWindowFn = [&views, &thisView]()
          {
            StringInputWindowPtr inputWnd = MakeNewPtr<StringInputWindow>("Material Name##NwMat", true);
            inputWnd->m_inputVal          = "New Material";
            inputWnd->m_inputLabel        = "Name";
            inputWnd->m_hint              = "New material name";
            inputWnd->AddToUI();

            inputWnd->m_taskFn = [views](const String& val)
            {
              String file = ConcatPaths({views[0]->m_path, val + MATERIAL});
              if (CheckFile(file))
              {
                g_app->GetConsole()->AddLog("Can't create. A material with the same name exist", LogType::Error);
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
            thisView->m_parent->ReconstructFolderTree();
            ImGui::CloseCurrentPopup();
          };

          if (ImGui::MenuItem("PBR Material"))
          {
            inputWindowFn();
          }
          ImGui::EndMenu();
        }
      };
    }

    void FolderView::DropFiles(const String& dst)
    {
      for (int i = 0; i < g_fileDragData.NumFiles; ++i)
      {
        DirectoryEntry& entry = *g_fileDragData.Entries[i];
        if (!CheckFile(entry.GetFullPath()))
        {
          continue;
        }

        String newPath = ConcatPaths({dst, entry.m_fileName + entry.m_ext});
        std::error_code errCode;
        std::filesystem::rename(entry.GetFullPath(), newPath, errCode);
        if (errCode)
        {
          TK_ERR("Rename failed: %s", errCode.message().c_str());
          g_app->SetStatusMsg(g_statusFailed);
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

      g_dragBeginView = nullptr;
      g_carryingFiles = false;
      g_selectedFiles.clear();
    }

    void FolderView::MoveTo(const String& dst)
    {
      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
        {
          DropFiles(dst);
        }
        ImGui::EndDragDropTarget();
      }
    }

  } // namespace Editor
} // namespace ToolKit
