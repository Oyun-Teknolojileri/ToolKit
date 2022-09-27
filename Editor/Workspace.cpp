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

    Workspace::Workspace(App* app)
    {
      m_app = app;
    }

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

        bundle.doc  = lclDoc;
        bundle.file = lclFile;

        StringArray path = {"Settings", "Workspace"};
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
        ReadAttr(node, "path", path);
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
          if (XmlAttribute* attr = node->first_attribute("path"))
          {
            attr->value(docBundle.doc->allocate_string(path.c_str(), 0));
          }
          else
          {
            WriteAttr(node, docBundle.doc.get(), "path", path);
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

    String Workspace::GetActiveWorkspace() const
    {
      return m_activeWorkspace;
    }

    Project Workspace::GetActiveProject() const
    {
      return m_activeProject;
    }

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
      for (std::filesystem::directory_entry const& dir :
           std::filesystem::directory_iterator(m_activeWorkspace))
      {
        if (dir.is_directory())
        {
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
        XmlNode* settings =
            lclDoc->allocate_node(rapidxml::node_element, "Settings");
        lclDoc->append_node(settings);

        XmlNode* setNode =
            lclDoc->allocate_node(rapidxml::node_element, "Workspace");
        WriteAttr(setNode, lclDoc.get(), "path", m_activeWorkspace);
        settings->append_node(setNode);

        setNode = lclDoc->allocate_node(rapidxml::node_element, "Project");
        WriteAttr(setNode, lclDoc.get(), "name", m_activeProject.name);
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
            WriteAttr(setNode, lclDoc.get(), "scene", scenePath);
          }
        }

        std::string xml;
        rapidxml::print(std::back_inserter(xml), *lclDoc);

        file << xml;
        file.close();
        lclDoc->clear();
      }
    }

    void Workspace::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      String settingsFile = ConcatPaths({ConfigPath(), g_workspaceFile});

      XmlFilePtr lclFile    = std::make_shared<XmlFile>(settingsFile.c_str());
      XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
      lclDoc->parse<0>(lclFile->data());

      if (XmlNode* settings = lclDoc->first_node("Settings"))
      {
        if (XmlNode* setNode = settings->first_node("Workspace"))
        {
          ReadAttr(setNode, "path", m_activeWorkspace);
        }

        String projectName, sceneName;
        if (XmlNode* setNode = settings->first_node("Project"))
        {
          ReadAttr(setNode, "name", projectName);
          String scene;
          ReadAttr(setNode, "scene", sceneName);
        }

        Project project = {projectName, sceneName};
        SetActiveProject(project);
      }
    }

  } // namespace Editor
} // namespace ToolKit
