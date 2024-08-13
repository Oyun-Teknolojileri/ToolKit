/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "DirectoryEntry.h"
#include "MaterialView.h"
#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {

    struct FileDragData
    {
      int NumFiles             = 0;
      DirectoryEntry** Entries = nullptr;
    };

    class FolderWindow;

    class FolderView
    {
     public:
      FolderView();
      ~FolderView();
      FolderView(FolderWindow* parent);

      void Show();
      void SetDirty();
      void SetPath(const String& path);
      const String& GetPath() const;

      /**
       * Root part of the path is returned.
       * Root is one level after Resource. Ex: ".../Resource/Audio"
       */
      String GetRoot() const;

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

      void DetermineAndSetBackgroundColor(bool isSelected, int index);
      bool IsMultiSelecting();

      /** Selects the files between two entry index(including a and b) */
      void SelectFilesInRange(int a, int b);

     public:
      int m_folderIndex      = -1;

      // Indicates this is a root folder (one level under Resources)
      // and currently selected in the FolderWindow.
      bool m_currRoot        = false;
      // Indicates this is a root folder (one level under Resources)
      bool m_root            = false;
      // States if the tab is visible.
      // Doesn't necessarily mean active, its just a tab in the FolderView.
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
      FolderWindow* m_parent  = nullptr;
      bool m_dirty            = false;
      ImVec2 m_contextBtnSize = ImVec2(75, 20);
      String m_filter         = "";
      std::unordered_map<String, std::function<void(DirectoryEntry*, FolderView*)>> m_itemActions;

      // If you change this value, change the calculaton of thumbnail zoom
      const float m_thumbnailMaxZoom = 300.f;
    };

    class FolderWindow : public Window
    {
     public:
      TKDeclareClass(FolderWindow, Window);

      FolderWindow();
      virtual ~FolderWindow();
      void Show() override;
      void UpdateContent();
      FolderView& GetView(int indx);
      // Returns root level active view, if deep is true,
      // returns sub-active / visible view.
      FolderView* GetActiveView(bool deep);
      void SetActiveView(FolderView* view);
      int Exist(const String& folder);
      bool GetFileEntry(const String& fullPath, DirectoryEntry& entry);
      void AddEntry(FolderView& view);
      void SetViewsDirty();
      void ReconstructFolderTree();
      void IterateFolders(bool includeEngine);

     protected:
      XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
      XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

     private:
      IntArray GetViews();      // Returns active root's descended views (tabs).
      void UpdateCurrentRoot(); // Based on selected folder, updates the current root folder.

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
        IntArray childs;
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

    typedef std::shared_ptr<FolderWindow> FolderWindowPtr;
    typedef std::vector<FolderWindow*> FolderWindowRawPtrArray;

  } // namespace Editor
} // namespace ToolKit
