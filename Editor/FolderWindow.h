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

#pragma once

#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {
    class DirectoryEntry
    {
     public:
      DirectoryEntry();
      explicit DirectoryEntry(const String& fullPath);
      String GetFullPath() const;
      ResourceManager* GetManager() const;
      RenderTargetPtr GetThumbnail() const;

     public:
      String m_ext;
      String m_fileName;
      String m_rootPath;
      bool m_isDirectory = false;
      bool m_cutting     = false;

     private:
      MaterialPtr m_tempThumbnailMaterial = nullptr;
    };

    class FolderWindow;
    typedef std::vector<FolderWindow*> FolderWindowRawPtrArray;

    struct FileDragData
    {
      int NumFiles             = 0;
      DirectoryEntry** Entries = nullptr;
    };

    class FolderView
    {
     public:
      FolderView();
      ~FolderView();
      explicit FolderView(FolderWindow* parent);

      void Show();

      void SetDirty() { m_dirty = true; }

      void SetPath(const String& path);
      const String& GetPath() const;
      void Iterate();
      int Exist(const String& file, const String& ext);
      void ShowContextMenu(DirectoryEntry* entry = nullptr);
      void Refresh();
      float GetThumbnailZoomPercent(float thumbnailZoom);
      int SelectFolder(FolderWindow* window, const String& path);

      void DropFiles(const String& dst); //!< drop selectedFiles

      static const FileDragData& GetFileDragData();

     private:
      void HandleCopyPasteDelete();
      static void PasteFiles(const String& path);
      void DrawSearchBar();
      void CreateItemActions();
      void MoveTo(const String& dst); // Imgui Drop target.

     public:
      // Indicates this is a root folder (one level under Resources)
      // and currently selected in the FolderWindow.
      bool m_currRoot        = false;
      // Indicates this is a root folder (one level under Resources)
      bool m_root            = false;
      // States if the tab is visible.
      // Doesnt necesserly mean active, its just a tab in the FolderView.
      bool m_visible         = false;
      bool m_active          = false; // Active tab, whose content is being displayed.
      // Always false. When set to true,
      // actives the view and becomes false again.
      bool m_activateNext    = false;
      bool m_onlyNativeTypes = true;
      Vec2 m_iconSize        = Vec2(50.0f);
      std::vector<DirectoryEntry> m_entries;
      int m_lastClickedEntryIdx = -1;
      String m_folder;
      String m_path;

     private:
      void DeterminateAndSetBackgroundColor(bool isSelected, int index);
      bool IsMultiSelecting();
      /**
       * Selects the files between two entry index(including a and b).
       */
      void SelectFilesInRange(int a, int b);

      FolderWindow* m_parent  = nullptr;
      bool m_dirty            = false;
      ImVec2 m_contextBtnSize = ImVec2(75, 20);
      String m_filter         = "";
      std::unordered_map<String, std::function<void(DirectoryEntry*, FolderView*)>> m_itemActions;

      // If you change this value, change the calculaton of thumbnail zoom
      const float m_thumbnailMaxZoom                 = 300.f;
      class TempMaterialWindow* m_tempMaterialWindow = nullptr;
    };

    class FolderWindow : public Window
    {
     public:
      explicit FolderWindow(XmlNode* node);
      FolderWindow(bool addEngine);
      virtual ~FolderWindow();
      void Show() override;
      Type GetType() const override;
      void UpdateContent();
      FolderView& GetView(int indx);
      // Returns root level active view, if deep is true,
      // returns sub-active / visible view.
      FolderView* GetActiveView(bool deep);
      void SetActiveView(FolderView* view);
      int Exist(const String& folder);
      bool GetFileEntry(const String& fullPath, DirectoryEntry& entry);
      void AddEntry(const FolderView& view);
      void SetViewsDirty();
      void ReconstructFolderTree();

      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

     private:
      // Returns active root's decendend views (tabs).
      IntArray GetVeiws();

      void ShowFolderTree();
      void DeactivateNode(const String& name);
      int CreateTreeRec(int parent, const std::filesystem::path& path);
      void DrawTreeRec(int index, float depth);
      void Iterate(const String& path, bool clear, bool addEngine = true);

     private:
      struct ViewSettings
      {
        Vec2 size;
        bool visible;
        bool active;
      };

      struct FolderNode
      {
        String path = "undefined";
        String name = "undefined";
        std::vector<int> childs;
        int index   = -1;
        bool active = false;

        FolderNode() {}

        FolderNode(int idx, String p, String n) : index(idx), path(std::move(p)), name(std::move(n)) {}
      };

      std::unordered_map<String, ViewSettings> m_viewSettings;
      std::vector<FolderView> m_entries;
      std::vector<FolderNode> m_folderNodes;
      float m_maxTreeNodeWidth   = 160.0f;

      int m_activeFolder         = 0;
      bool m_showStructure       = true;
      int m_resourcesTreeIndex   = 0;
      int m_lastSelectedTreeNode = -1;
    };

  } // namespace Editor
} // namespace ToolKit
