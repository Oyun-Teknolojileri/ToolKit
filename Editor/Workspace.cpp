#include "Workspace.h"

#include "App.h"

#include <filesystem>
#include <memory>
#include <string>

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {
    static const StringView XmlNodeWorkspace = "Workspace";
    static const StringView XmlNodeProject   = "Project";
    static const StringView XmlNodeScene     = "scene";

    Workspace::Workspace(App* app) { m_app = app; }

    void Workspace::Init()
    {
      m_activeWorkspace = GetDefaultWorkspace();
      DeSerialize(nullptr, nullptr);
    }

    XmlNode* Workspace::GetDefaultWorkspaceNode(XmlDocBundle& bundle) const
    {
      String settingsFile = ConcatPaths({ConfigPath(), g_workspaceFile});

      if (CheckFile(settingsFile))
      {
        XmlFilePtr lclFile = GetFileManager()->GetXmlFile(settingsFile.c_str());
        XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
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
      String codePath =
          ConcatPaths({GetActiveWorkspace(), m_activeProject.name, "Codes"});

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

      return ConcatPaths(
          {m_activeWorkspace, m_activeProject.name, "Resources"});
    }

    String Workspace::GetActiveWorkspace() const { return m_activeWorkspace; }

    Project Workspace::GetActiveProject() const { return m_activeProject; }

    void Workspace::SetActiveProject(const Project& project)
    {
      m_activeProject                     = project;
      Main::GetInstance()->m_resourceRoot = GetResourceRoot();
    }

    void Workspace::SetScene(const String& scene)
    {
      m_activeProject.scene = scene;
    }

    void Workspace::RefreshProjects()
    {
      m_projects.clear();
      for (const std::filesystem::directory_entry& dir :
           std::filesystem::directory_iterator(m_activeWorkspace))
      {
        if (dir.is_directory())
        {
          String resourcesPath =
              ConcatPaths({dir.path().string(), "Resources"});
          String codesPath = ConcatPaths({dir.path().string(), "Codes"});
          // Skip directory if it doesn't have folders: Resources, Codes
          if (!std::filesystem::directory_entry(resourcesPath).is_directory() ||
              !std::filesystem::directory_entry(codesPath).is_directory())
          {
            continue;
          }
          const StringArray requiredResourceFolders = {"Materials",
                                                       "Meshes",
                                                       "Scenes",
                                                       "Textures"};
          bool foundAllRequiredFolders              = true;
          for (uint i = 0; i < requiredResourceFolders.size(); i++)
          {
            if (!(std::filesystem::directory_entry(
                      ConcatPaths({resourcesPath, requiredResourceFolders[i]}))
                      .is_directory()))
            {
              foundAllRequiredFolders = false;
              break;
            }
          }
          if (!foundAllRequiredFolders)
          {
            break;
          }
          std::string dirName = dir.path().filename().u8string();

          // Hide hidden folders
          if (dirName.size() > 1 && dirName[0] != '.')
          {
            Project project = {dirName, ""};

            m_projects.push_back(project);
          }
        }
      }
    }

    void Workspace::Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      std::ofstream file;
      String fileName = ConcatPaths({ConfigPath(), g_workspaceFile});

      file.open(fileName.c_str(), std::ios::out);
      if (file.is_open())
      {
        XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
        XmlNode* settings     = lclDoc->allocate_node(rapidxml::node_element,
                                                  XmlNodeSettings.data());
        lclDoc->append_node(settings);

        XmlNode* setNode = lclDoc->allocate_node(rapidxml::node_element,
                                                 XmlNodeWorkspace.data());
        WriteAttr(setNode, lclDoc.get(), XmlNodePath.data(), m_activeWorkspace);
        settings->append_node(setNode);

        setNode = lclDoc->allocate_node(rapidxml::node_element,
                                        XmlNodeProject.data());
        WriteAttr(setNode,
                  lclDoc.get(),
                  XmlNodeName.data(),
                  m_activeProject.name);
        settings->append_node(setNode);

        String sceneFile = m_app->GetCurrentScene()->GetFile();
        if (GetSceneManager()->Exist(sceneFile))
        {
          String sceneRoot = ScenePath("");
          // Don't save anything as current scene,
          // if its not in scene root folder.
          if (sceneFile.find(sceneRoot) != String::npos)
          {
            String scenePath = GetRelativeResourcePath(sceneFile);
            WriteAttr(setNode, lclDoc.get(), XmlNodeScene.data(), scenePath);
          }
        }

        std::string xml;
        rapidxml::print(std::back_inserter(xml), *lclDoc);

        file << xml;
        file.close();
        lclDoc->clear();
      }
      SerializeEngineSettings();
    }

    void Workspace::SerializeEngineSettings() const
    {
      std::ofstream file;
      String path = ConcatPaths({GetProjectConfigPath(), "Engine.settings"});

      file.open(path.c_str(), std::ios::out | std::ios::trunc);
      assert(file.is_open());

      if (file.is_open()) 
      {
        XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
        GetEngineSettings().Serialize(lclDoc.get(), nullptr);

        std::string xml;
        rapidxml::print(std::back_inserter(xml), *lclDoc);
        file << xml;
        file.close();
        lclDoc->clear();
      }
    }

    void Workspace::DeSerializeEngineSettings()
    {
      String settingsFile =
          ConcatPaths({GetProjectConfigPath(), "Engine.settings"});

      // search for project/Engine.settings file,
      // if its not exist pull default Engine.settings file from appdata
      if (!CheckSystemFile(settingsFile))
      {
        settingsFile = ConcatPaths({ConfigPath(), "Engine.settings"});
      }

      XmlFilePtr lclFile    = std::make_shared<XmlFile>(settingsFile.c_str());
      XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
      lclDoc->parse<0>(lclFile->data());

      GetEngineSettings().DeSerialize(lclDoc.get(), nullptr);
    }

    void Workspace::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      String settingsFile   = ConcatPaths({ConfigPath(), g_workspaceFile});

      XmlFilePtr lclFile    = std::make_shared<XmlFile>(settingsFile.c_str());
      XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
      lclDoc->parse<0>(lclFile->data());

      if (XmlNode* settings = lclDoc->first_node(XmlNodeSettings.data()))
      {
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
            String scene;
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
    }

  } // namespace Editor
} // namespace ToolKit
