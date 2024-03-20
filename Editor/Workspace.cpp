/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Workspace.h"

#include "App.h"

#include <FileManager.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {
    static const StringView XmlNodeWorkspace = "Workspace";
    static const StringView XmlNodeProject   = "Project";
    static const StringView XmlNodeScene     = "scene";

    Workspace::Workspace() {}

    void Workspace::Init()
    {
      m_activeWorkspace = GetDefaultWorkspace();
      DeSerialize(SerializationFileInfo(), nullptr);
    }

    XmlNode* Workspace::GetDefaultWorkspaceNode(XmlDocBundle& bundle) const
    {
      String settingsFile = ConcatPaths({ConfigPath(), g_workspaceFile});

      if (CheckFile(settingsFile))
      {
        XmlFilePtr lclFile    = GetFileManager()->GetXmlFile(settingsFile.c_str());
        XmlDocumentPtr lclDoc = MakeNewPtr<XmlDocument>();
        lclDoc->parse<0>(lclFile->data());

        bundle.doc       = lclDoc;
        bundle.file      = lclFile;

        StringArray path = {XmlNodeSettings.data(), XmlNodeWorkspace.data()};
        return Query(lclDoc.get(), path);
      }

      return nullptr;
    }

    String Workspace::GetDefaultWorkspace() const
    {
      String path;
      XmlDocBundle docBundle;
      if (XmlNode* node = GetDefaultWorkspaceNode(docBundle))
      {
        String foundPath;
        ReadAttr(node, XmlNodePath.data(), foundPath);
        if (CheckFile(foundPath))
        {
          path = foundPath;
        }
      }

      return path;
    }

    bool Workspace::SetDefaultWorkspace(const String& path)
    {
      XmlDocBundle docBundle;
      if (XmlNode* node = GetDefaultWorkspaceNode(docBundle))
      {
        std::ofstream file;
        String settingsPath = ConcatPaths({ConfigPath(), g_workspaceFile});

        file.open(settingsPath.c_str(), std::ios::out);
        if (file.is_open())
        {
          m_activeWorkspace = path;
          RefreshProjects();
          if (XmlAttribute* attr = node->first_attribute(XmlNodePath.data()))
          {
            attr->value(docBundle.doc->allocate_string(path.c_str(), 0));
          }
          else
          {
            WriteAttr(node, docBundle.doc.get(), XmlNodePath.data(), path);
          }

          String xml;
          rapidxml::print(std::back_inserter(xml), *docBundle.doc.get());

          file << xml;
          file.close();

          return true;
        }
      }

      return false;
    }

    String Workspace::GetCodePath() const
    {
      String codePath = ConcatPaths({GetActiveWorkspace(), m_activeProject.name, "Codes"});

      return codePath;
    }

    String Workspace::GetProjectConfigPath() const
    {
      if (m_activeProject.name.empty())
      {
        return m_activeWorkspace;
      }

      return ConcatPaths({m_activeWorkspace, m_activeProject.name, "Config"});
    }

    String Workspace::GetPluginPath() const
    {
      String codePath   = GetCodePath();
      String pluginPath = ConcatPaths({codePath, "Bin", m_activeProject.name});

      return pluginPath;
    }

    String Workspace::GetResourceRoot() const
    {
      if (m_activeProject.name.empty())
      {
        return m_activeWorkspace;
      }

      return ConcatPaths({m_activeWorkspace, m_activeProject.name, "Resources"});
    }

    String Workspace::GetActiveWorkspace() const { return m_activeWorkspace; }

    Project Workspace::GetActiveProject() const { return m_activeProject; }

    void Workspace::SetActiveProject(const Project& project)
    {
      m_activeProject                     = project;
      Main::GetInstance()->m_resourceRoot = GetResourceRoot();
    }

    void Workspace::SetScene(const String& scene) { m_activeProject.scene = scene; }

    void Workspace::RefreshProjects()
    {
      m_projects.clear();
      for (const std::filesystem::directory_entry& dir : std::filesystem::directory_iterator(m_activeWorkspace))
      {
        if (dir.is_directory())
        {
          String dirPath = dir.path().string();
          if (dirPath.find(".git") != String::npos)
          {
            // Skip git directory.
            continue;
          }

          String resourcesPath = ConcatPaths({dirPath, "Resources"});
          String codesPath     = ConcatPaths({dirPath, "Codes"});

          // Skip directory if it doesn't have folders: Resources, Codes
          if (!std::filesystem::directory_entry(resourcesPath).is_directory() ||
              !std::filesystem::directory_entry(codesPath).is_directory())
          {
            continue;
          }

          const StringArray requiredResourceFolders = {"Materials", "Meshes", "Scenes", "Textures"};
          bool foundAllRequiredFolders              = true;
          for (uint i = 0; i < requiredResourceFolders.size(); i++)
          {
            String checkDir = ConcatPaths({resourcesPath, requiredResourceFolders[i]});
            if (!(std::filesystem::directory_entry(checkDir).is_directory()))
            {
              foundAllRequiredFolders = false;
              break;
            }
          }

          if (!foundAllRequiredFolders)
          {
            continue;
          }

          // Don't show hidden folders
          String dirName = dir.path().filename().u8string();
          if (dirName.size() > 1 && dirName[0] != '.')
          {
            Project project = {dirName, ""};
            m_projects.push_back(project);
          }
        }
      }
    }

    XmlNode* Workspace::SerializeImp(XmlDocument* doc, XmlNode* parent) const
    {
      std::ofstream file;
      String fileName = ConcatPaths({ConfigPath(), g_workspaceFile});

      file.open(fileName.c_str(), std::ios::out);
      if (file.is_open())
      {
        XmlDocument* lclDoc = new XmlDocument();
        XmlNode* settings   = CreateXmlNode(lclDoc, XmlNodeSettings.data());
        WriteAttr(settings, lclDoc, XmlVersion, TKVersionStr);

        XmlNode* setNode = CreateXmlNode(lclDoc, XmlNodeWorkspace.data(), settings);
        WriteAttr(setNode, lclDoc, XmlNodePath.data(), m_activeWorkspace);

        setNode = CreateXmlNode(lclDoc, XmlNodeProject.data(), settings);
        WriteAttr(setNode, lclDoc, XmlNodeName.data(), m_activeProject.name);

        String sceneFile = g_app->GetCurrentScene()->GetFile();
        if (GetSceneManager()->Exist(sceneFile))
        {
          String sceneRoot = ScenePath("");
          // Don't save anything as current scene,
          // if its not in scene root folder.
          if (sceneFile.find(sceneRoot) != String::npos)
          {
            String scenePath = GetRelativeResourcePath(sceneFile);
            WriteAttr(setNode, lclDoc, XmlNodeScene.data(), scenePath);
          }
        }

        std::string xml;
        rapidxml::print(std::back_inserter(xml), *lclDoc);

        file << xml;
        file.close();
        lclDoc->clear();
        SafeDel(lclDoc);
      }
      SerializeEngineSettings();

      return nullptr;
    }

    void Workspace::SerializeSimulationWindow(XmlDocumentPtr doc) const
    {
      XmlDocument* pDoc                = doc.get();

      SimulationWindowPtr pluginWindow = g_app->GetWindow<SimulationWindow>("Plugin");
      XmlNode* settings                = CreateXmlNode(pDoc, "Simulation", nullptr);

      int numCustomRes = (int) pluginWindow->m_screenResolutions.size() - pluginWindow->m_numDefaultResNames;

      WriteAttr(settings, pDoc, "NumCustom", std::to_string(numCustomRes));

      for (int i = 0; i < numCustomRes; i++)
      {
        String istr = std::to_string(i);
        int index   = i + pluginWindow->m_numDefaultResNames;

        WriteAttr(settings, pDoc, "name" + istr, pluginWindow->m_emulatorResolutionNames[index]);

        WriteAttr(settings, pDoc, "sizeX" + istr, std::to_string(pluginWindow->m_screenResolutions[index].x));

        WriteAttr(settings, pDoc, "sizeY" + istr, std::to_string(pluginWindow->m_screenResolutions[index].y));
      }
    }

    void Workspace::DeSerializeSimulationWindow(XmlDocumentPtr doc)
    {
      XmlNode* node                    = doc->first_node("Simulation");
      SimulationWindowPtr pluginWindow = g_app->GetWindow<SimulationWindow>("Plugin");
      if (node == nullptr || pluginWindow == nullptr)
      {
        return;
      }

      const int defaultCnt = pluginWindow->m_numDefaultResNames;
      pluginWindow->m_screenResolutions.resize(defaultCnt);
      pluginWindow->m_emulatorResolutionNames.resize(defaultCnt);

      int numCustomRes = 0;
      ReadAttr(node, "NumCustom", numCustomRes);
      pluginWindow->m_screenResolutions.resize(numCustomRes + defaultCnt);
      pluginWindow->m_emulatorResolutionNames.resize(numCustomRes + defaultCnt);

      for (int i = 0; i < numCustomRes; i++)
      {
        String istr   = std::to_string(i);
        const int idx = i + defaultCnt;
        ReadAttr(node, "name" + istr, pluginWindow->m_emulatorResolutionNames[idx]);
        ReadAttr(node, "sizeX" + istr, pluginWindow->m_screenResolutions[idx].x);
        ReadAttr(node, "sizeY" + istr, pluginWindow->m_screenResolutions[idx].y);
      }
    }

    void Workspace::SerializeEngineSettings() const
    {
      String path = ConcatPaths({GetProjectConfigPath(), "Engine.settings"});
      GetEngineSettings().SerializeEngineSettings(path);
    }

    void Workspace::DeSerializeEngineSettings()
    {
      String settingsFile = ConcatPaths({GetProjectConfigPath(), "Engine.settings"});

      // search for project/Engine.settings file,
      // if its not exist pull default Engine.settings file from appdata
      if (!CheckSystemFile(settingsFile))
      {
        settingsFile = ConcatPaths({ConfigPath(), "Engine.settings"});
      }

      GetEngineSettings().DeSerializeEngineSettings(settingsFile);
    }

    XmlNode* Workspace::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
    {
      String settingsFile   = ConcatPaths({ConfigPath(), g_workspaceFile});

      XmlFilePtr lclFile    = MakeNewPtr<XmlFile>(settingsFile.c_str());
      XmlDocumentPtr lclDoc = MakeNewPtr<XmlDocument>();
      lclDoc->parse<0>(lclFile->data());

      if (XmlNode* settings = lclDoc->first_node(XmlNodeSettings.data()))
      {
        ReadAttr(settings, XmlVersion.data(), m_version);

        if (XmlNode* setNode = settings->first_node(XmlNodeWorkspace.data()))
        {
          String foundWorkspacePath;
          ReadAttr(setNode, XmlNodePath.data(), foundWorkspacePath);
          if (CheckFile(foundWorkspacePath))
          {
            m_activeWorkspace = foundWorkspacePath;
          }
        }

        if (m_activeWorkspace.length())
        {
          RefreshProjects();

          String projectName, sceneName;
          if (XmlNode* setNode = settings->first_node(XmlNodeProject.data()))
          {
            ReadAttr(setNode, XmlNodeName.data(), projectName);
            ReadAttr(setNode, XmlNodeScene.data(), sceneName);
          }

          for (const Project& project : m_projects)
          {
            if (project.name == projectName)
            {
              Project project = {projectName, sceneName};
              SetActiveProject(project);
            }
          }
        }
      }

      DeSerializeEngineSettings();
      return nullptr;
    }

  } // namespace Editor
} // namespace ToolKit
