#pragma once

#include "Serialize.h"
#include "ToolKit.h"

namespace ToolKit
{
  namespace Editor
  {

    struct Project
    {
      String name;
      String scene;
    };

    class Workspace : public Serializable
    {
     public:
      Workspace();
      void Init();

      // Defaults read / writes to installment directory.
      XmlNode* GetDefaultWorkspaceNode(XmlDocBundle& bundle) const;
      String GetDefaultWorkspace() const;
      bool SetDefaultWorkspace(const String& path);

      // Accessors to workspace
      String GetCodePath() const;
      String GetProjectConfigPath() const;
      String GetPluginPath() const;
      String GetResourceRoot() const;
      String GetActiveWorkspace() const;
      Project GetActiveProject() const;
      void SetActiveProject(const Project& project);
      void SetScene(const String& scene);

      void RefreshProjects();

      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

      void SerializeEngineSettings() const;
      void DeSerializeEngineSettings();

     private:
      void SerializeSimulationWindow(XmlDocument* doc) const;
      void DeSerializeSimulationWindow(XmlDocument* doc);

     public:
      std::vector<Project> m_projects;

     private:
      String m_activeWorkspace;
      Project m_activeProject;
    };

  } // namespace Editor
} // namespace ToolKit
