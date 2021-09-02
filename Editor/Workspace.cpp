#include "stdafx.h"
#include "Workspace.h"
#include "App.h"
#include "DebugNew.h"

#include <filesystem>

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

    XmlNode* Workspace::GetDefaultWorkspaceNode(XmlDocBundle& bundle)
    {
      String settingsFile = ConcatPaths({ DefaultPath(), "workspace.settings" });
      if (CheckFile(settingsFile))
      {
        std::shared_ptr<XmlFile> lclFile = std::make_shared<XmlFile>(settingsFile.c_str());
        XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
        lclDoc->parse<0>(lclFile->data());

        bundle.doc = lclDoc;
        bundle.file = lclFile;

        StringArray path = { "Settings", "Workspace" };
        return Query(lclDoc.get(), path);
      }

      return nullptr;
    }

    String Workspace::GetDefaultWorkspace()
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
        String settingsPath = ConcatPaths({ ResourcePath(), "default.settings" });

        file.open(settingsPath.c_str(), std::ios::out);
        if (file.is_open())
        {
          m_activeWorkspace = path;
          RefreshProjects();
          if (XmlAttribute* attr = node->first_attribute("path"))
          {
            attr->value
            (
              docBundle.doc->allocate_string(path.c_str(), 0)
            );
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

    String Workspace::GetResourceRoot()
    {
      if (m_activeProject.name.empty())
      {
        return m_activeWorkspace;
      }

      return ConcatPaths({ m_activeWorkspace, m_activeProject.name,  "Resources" });
    }

    String Workspace::GetActiveWorkspace()
    {
      if (m_activeWorkspace.empty())
      {
        m_activeWorkspace = GetDefaultWorkspace();
      }

      return m_activeWorkspace;
    }

    Project Workspace::GetActiveProject()
    {
      return m_activeProject;
    }

    void Workspace::SetActiveProject(const Project& project)
    {
      m_activeProject = project;
      Main::GetInstance()->m_resourceRoot = GetResourceRoot();
    }

    void Workspace::RefreshProjects()
    {
      m_projects.clear();
      for (std::filesystem::directory_entry const& dir : std::filesystem::directory_iterator(m_activeWorkspace))
      {
        if (dir.is_directory())
        {
          Project project = 
          { 
            dir.path().filename().u8string(),
            ""
          };

          m_projects.push_back(project);
        }
      }
    }

    void Workspace::Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      std::ofstream file;
      String fileName = ConcatPaths({ m_activeWorkspace, "workspace.settings" });
      file.open(fileName.c_str(), std::ios::out);

      if (file.is_open())
      {
        XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
        XmlNode* settings = lclDoc->allocate_node(rapidxml::node_element, "Settings");
        lclDoc->append_node(settings);

        XmlNode* setNode = lclDoc->allocate_node(rapidxml::node_element, "Workspace");
        WriteAttr(setNode, lclDoc.get(), "path", m_activeWorkspace);
        settings->append_node(setNode);

        setNode = lclDoc->allocate_node(rapidxml::node_element, "Project");
        WriteAttr(setNode, lclDoc.get(), "name", m_activeProject.name);
        settings->append_node(setNode);

        if (!m_app->m_scene->m_newScene)
        {
          WriteAttr(setNode, lclDoc.get(), "scene", m_app->m_scene->m_name);
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
      String settingsFile = ConcatPaths({ m_activeWorkspace, "workspace.settings" });
      if (!CheckFile(settingsFile))
      {
        settingsFile = ConcatPaths({ DefaultPath(), "workspace.settings" });
      }

      XmlFilePtr lclFile = std::make_shared<XmlFile> (settingsFile.c_str());

      XmlDocumentPtr lclDoc = std::make_shared<XmlDocument> ();
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

        Project project = { projectName, sceneName };
        SetActiveProject(project);
      }
    }

  }
}
