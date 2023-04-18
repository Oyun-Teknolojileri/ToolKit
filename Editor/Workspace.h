#pragma once

#include "Serialize.h"
#include "ToolKit.h"

#include <vector>

namespace ToolKit
{
  namespace Editor
  {

    struct Project
    {
      String name;
      String scene;
    };

    class App;

    class Workspace : public Serializable
    {
     public:
      explicit Workspace(App* app);
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
      void SerializeSimulationWindow(const XmlDocumentPtr& settingsDoc) const;     
      void DeSerializeSimulationWindow(const XmlDocumentPtr& settingsDoc);

    public:
      std::vector<Project> m_projects;

     private:
      App* m_app = nullptr;
      String m_activeWorkspace;
      Project m_activeProject;
    };

  } // namespace Editor
} // namespace ToolKit
