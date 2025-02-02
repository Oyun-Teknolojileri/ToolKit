/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "FolderWindow.h"

#include "App.h"

#include <Animation.h>
#include <Audio.h>
#include <Material.h>
#include <Mesh.h>
#include <Scene.h>
#include <Shader.h>
#include <Texture.h>
#include <ToolKit.h>

namespace ToolKit
{
  namespace Editor
  {

    String GetRootPath(const String& folder)
    {
      static std::unordered_set<String> rootMap = {"Fonts", "Materials", "Meshes", "Scenes", "Shaders", "Textures"};

      String path                               = folder;
      String subFolder {};
      // traverse parent paths and search a root folder
      while (path.size() > 0ull)
      {
        subFolder.clear();

        while (path.size() > 0ull)
        {
          // pop last character
          char c = path.back();
          path.erase(path.end() - 1ull);

          if (c == '\\' || c == '/')
          {
            break;
          }
          // push character to subFolder
          subFolder.push_back(c);
        }
        // reverse because we pushed reversely
        std::reverse(subFolder.begin(), subFolder.end());
        // if we found a root folder return this path
        if (rootMap.count(subFolder) > 0)
        {
          return ConcatPaths({path, subFolder});
        }
      }
      return DefaultPath();
    }

    TKDefineClass(FolderWindow, Window);

    FolderWindow::FolderWindow() {}

    FolderWindow::~FolderWindow() {}

    // destroy old one and create new tree
    void FolderWindow::ReconstructFolderTree()
    {
      m_folderNodes.clear();
      CreateTreeRec(-1, DefaultPath());
      m_resourcesTreeIndex = (int) m_folderNodes.size();
      CreateTreeRec(int(m_folderNodes.size()) - 1, ResourcePath());
    }

    void FolderWindow::IterateFolders(bool includeEngine) { Iterate(ResourcePath(), true, includeEngine); }

    // parent will start with -1
    int FolderWindow::CreateTreeRec(int parent, const std::filesystem::path& path)
    {
      String folderName = path.filename().u8string();
      int index         = (int) m_folderNodes.size();
      m_folderNodes.emplace_back(index, std::filesystem::absolute(path).u8string(), folderName);

      for (const std::filesystem::directory_entry& directory : std::filesystem::directory_iterator(path))
      {
        if (!directory.is_directory())
        {
          continue;
        }

        int childIdx = CreateTreeRec(parent + 1, directory.path());
        m_folderNodes[index].childs.push_back(childIdx);
      }

      return index;
    }

    extern FolderView* g_dragBeginView;

    void FolderWindow::DrawTreeRec(int index, float depth)
    {
      if (index == -1)
      {
        return; // shouldn't happen
      }

      FolderNode& node        = m_folderNodes[index];
      IntArray ascendantViews = GetAscendants();

      // Check all ascendants to set their icons open.
      bool folderOpen         = false;
      for (int i : ascendantViews)
      {
        folderOpen |= node.path == m_entries[i].GetPath();
      }
      folderOpen             |= node.path == m_entries[m_activeFolder].GetRoot(); // Include current root as well.

      String icon             = folderOpen ? ICON_FA_FOLDER_OPEN_A : ICON_FA_FOLDER_A;
      String nodeHeader       = icon + ICON_SPACE + node.name;
      float headerLen         = ImGui::CalcTextSize(nodeHeader.c_str()).x;
      headerLen              += (depth * 20.0f) + 70.0f; // depth padding + UI start padding

      m_maxTreeNodeWidth      = glm::max(headerLen, m_maxTreeNodeWidth);

      const auto onClickedFn  = [&]() -> void
      {
        // find clicked entity
        int selected = Exist(node.path);
        if (selected != -1)
        {
          SetActiveView(selected);
        }
      };

      const auto acceptDrop = [&]() -> void
      {
        if (ImGui::BeginDragDropTarget())
        {
          if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
          {
            if (g_dragBeginView != nullptr)
            {
              g_dragBeginView->DropFiles(node.path);
            }
          }
          ImGui::EndDragDropTarget();
        }
      };

      ImGuiTreeNodeFlags nodeFlags = g_treeNodeFlags;
      String stdId                 = "##" + std::to_string(index);

      if (node.childs.size() == 0)
      {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        if (ImGui::TreeNodeEx(stdId.c_str(), nodeFlags, nodeHeader.c_str()))
        {
          if (ImGui::IsItemClicked())
          {
            onClickedFn();
          }
        }
        acceptDrop();
      }
      else
      {
        if (ImGui::TreeNodeEx(stdId.c_str(), nodeFlags, nodeHeader.c_str()))
        {
          if (ImGui::IsItemClicked())
          {
            onClickedFn();
          }

          for (int i = 0; i < node.childs.size(); ++i)
          {
            DrawTreeRec(node.childs[i], depth + 1.0f);
          }
          ImGui::TreePop();
        }
        acceptDrop();
      }
    }

    void FolderWindow::ShowFolderTree()
    {
      // Show Resource folder structure.
      ImGui::PushID("##FolderStructure");
      ImGui::BeginGroup();

      ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
      ImGui::TextUnformatted("Resources");

      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_ARROW_LEFT))
      {
        m_showStructure = !m_showStructure;
      }

      ImGui::BeginChild("##Folders", ImVec2(m_maxTreeNodeWidth, 0.0f), true);

      // reset tree node default size
      m_maxTreeNodeWidth = 160.0f;
      // draw tree of folders
      DrawTreeRec(m_resourcesTreeIndex, 0.0f);
      DrawTreeRec(0, 0.0f);

      ImGui::EndChild();

      ImGui::PopStyleVar();
      ImGui::EndGroup();
      ImGui::PopID();
    }

    IntArray FolderWindow::GetAscendants()
    {
      // Find all the sub folders up to the active folder.
      FolderView& activeFolder = GetView(m_activeFolder);
      String fullPath          = activeFolder.GetPath();
      String rootPath          = activeFolder.GetRoot();

      String intermediatePath  = fullPath.substr(rootPath.size());

      StringArray subDirs;
      Split(intermediatePath, GetPathSeparatorAsStr(), subDirs);

      if (subDirs.empty())
      {
        return {m_activeFolder};
      }

      // Construct all sub directory paths.
      StringArray subDirPaths;
      for (int i = 0; i < (int) subDirs.size(); i++)
      {
        StringArray subFolders;
        subFolders.push_back(rootPath);

        for (int ii = 0; ii <= i; ii++)
        {
          subFolders.push_back(subDirs[ii]);
        }

        subDirPaths.push_back(ConcatPaths(subFolders));
      }

      IntArray views;
      for (int i = 0; i < (int) m_entries.size(); i++)
      {
        FolderView& view = m_entries[i];
        String candidate = view.GetPath();

        for (int ii = 0; ii < (int) subDirPaths.size(); ii++)
        {
          String& subDir = subDirPaths[ii];
          if (candidate == subDir)
          {
            views.push_back(i);
            subDirPaths.erase(subDirPaths.begin() + ii);
            break;
          }
        }

        // Break if no next path remains.
        if (subDirPaths.empty())
        {
          break;
        }
      }

      return views;
    }

    IntArray FolderWindow::GetSiblings()
    {
      IntArray siblings;
      if (FolderView* view = GetActiveView())
      {
        String path = view->GetPath();

        StringArray target;
        Split(path, GetPathSeparatorAsStr(), target);

        size_t lastSep  = path.find_last_of(GetPathSeparator());
        String checkStr = path.substr(0, lastSep);

        for (int i = 0; i < m_entries.size(); i++)
        {
          FolderView& candidate = m_entries[i];

          // If candidate and active folder shares same root.
          String candidatePath  = candidate.GetPath();
          if (candidatePath.find(checkStr) != String::npos)
          {
            // And splits have same number of entry
            StringArray src;
            Split(candidatePath, GetPathSeparatorAsStr(), src);
            if (src.size() == target.size())
            {
              // Than they are siblings.
              siblings.push_back(i);
            }
          }
        }
      }

      return siblings;
    }

    void FolderWindow::UpdateCurrentRoot()
    {
      for (int i = 0; i < (int) m_entries.size(); i++)
      {
        FolderView& view = m_entries[i];
        view.m_currRoot  = false;

        if (view.m_folderIndex == m_activeFolder)
        {
          // Find the root that active folder is belong to and set it as current.
          String rootStr = view.GetRoot();
          for (int ii = 0; ii < (int) m_entries.size(); ii++)
          {
            FolderView& rootCandidate = m_entries[ii];
            if (rootCandidate.GetPath() == rootStr)
            {
              rootCandidate.m_currRoot = true;
              return;
            }
          }
        }
      }
    }

    void FolderWindow::Show()
    {
      ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        if (!g_app->m_workspace.GetActiveWorkspace().empty() && g_app->m_workspace.GetActiveProject().name.empty())
        {
          ImGui::Text("Load a project.");
          ImGui::End();
          return;
        }

        if (m_showStructure)
        {
          ShowFolderTree();
        }
        else
        {
          if (ImGui::Button(ICON_FA_ARROW_RIGHT))
          {
            m_showStructure = !m_showStructure;
          }
        }

        ImGui::SameLine();

        UpdateCurrentRoot();

        ImGui::PushID("##FolderContent");
        ImGui::BeginGroup();

        if (ImGui::BeginTabBar("Folders", ImGuiTabBarFlags_None))
        {
          // Draw each tab.
          IntArray views = GetAscendants();
          for (int i = 0; i < (int) views.size(); i++)
          {
            int folderIndex  = views[i];
            FolderView& view = m_entries[folderIndex];
            view.m_active    = m_activeFolder == folderIndex; // Set activation.
            view.Show();
            view.m_visible = view.m_active; // Set visibility due imgui altering tab activity.
          }
          ImGui::EndTabBar();
        }

        ImGui::EndGroup();
        ImGui::PopID();
      }

      ImGui::End();
    }

    void FolderWindow::Iterate(const String& path, bool clear, bool addEngine)
    {
      if (clear)
      {
        m_activeFolder = 0;
        m_entries.clear();
        ReconstructFolderTree();
      }

      String resourceRoot = ResourcePath();
      char pathSep        = GetPathSeparator();
      int baseCount       = CountChar(resourceRoot, pathSep);

      for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path))
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

          view.Iterate();
          AddEntry(view);
          Iterate(view.GetPath(), false, false);
        }
      }

      if (addEngine)
      {
        // Engine folder
        FolderView view(this);
        view.SetPath(DefaultAbsolutePath());

        view.Iterate();
        AddEntry(view);
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

    void FolderWindow::AddEntry(FolderView& view)
    {
      if (Exist(view.GetPath()) == -1)
      {
        view.m_folderIndex = (int) m_entries.size();
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

    FolderView* FolderWindow::GetActiveView()
    {
      if (m_activeFolder == -1)
      {
        return nullptr;
      }

      FolderView& rootView = GetView(m_activeFolder);
      return &rootView;
    }

    void FolderWindow::SetActiveView(int index)
    {
      if (index >= 0 && index < (int) m_entries.size())
      {
        m_activeFolder = index;
      }
    }

    void FolderWindow::SetActiveView(FolderView* view)
    {
      int indx = Exist(view->GetPath());
      SetActiveView(indx);
    }

    int FolderWindow::Exist(const String& path)
    {
      for (size_t i = 0; i < m_entries.size(); i++)
      {
        if (m_entries[i].GetPath() == path)
        {
          return (int) i;
        }
      }

      return -1;
    }

    bool FolderWindow::GetFileEntry(const String& fullPath, DirectoryEntry& entry)
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

    XmlNode* FolderWindow::SerializeImp(XmlDocument* doc, XmlNode* parent) const
    {
      XmlNode* wndNode = Window::SerializeImp(doc, parent);
      XmlNode* folder  = CreateXmlNode(doc, "FolderWindow", wndNode);

      WriteAttr(folder, doc, "activeFolder", std::to_string(m_activeFolder));
      WriteAttr(folder, doc, "showStructure", std::to_string(m_showStructure));

      return folder;
    }

    XmlNode* FolderWindow::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
    {
      Window::DeSerializeImp(info, parent);
      if (XmlNode* node = parent->first_node("FolderWindow"))
      {
        ReadAttr(node, "activeFolder", m_activeFolder);
        ReadAttr(node, "showStructure", m_showStructure);
      }

      Iterate(ResourcePath(), true);

      return nullptr;
    }

  } // namespace Editor
} // namespace ToolKit
