#pragma once

#include <vector>

#include "ToolKit.h"
#include "Serialize.h"

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
      XmlNode* GetDefaultWorkspaceNode(XmlDocBundle& bundle);
      String GetDefaultWorkspace();
      bool SetDefaultWorkspace(const String& path);

      // Accessors to workspace
      String GetCodePath();
      String GetPluginPath();
      String GetResourceRoot();
      String GetActiveWorkspace();
      Project GetActiveProject();
      void SetActiveProject(const Project& project);
      void SetScene(const String& scene);

      void RefreshProjects();

      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

     public:
      std::vector<Project> m_projects;

     private:
      App* m_app = nullptr;
      String m_activeWorkspace;
      Project m_activeProject;
    };

  }  // namespace Editor

}  // namespace ToolKit
