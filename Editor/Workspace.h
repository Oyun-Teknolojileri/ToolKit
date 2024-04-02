/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

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
      friend class App;

     public:
      Workspace();
      void Init();

      // Defaults read / writes to installment directory.
      XmlNode* GetDefaultWorkspaceNode(XmlDocBundle& bundle) const;
      String GetDefaultWorkspace() const;
      bool SetDefaultWorkspace(const String& path);

      // Accessors to workspace
      String GetCodeDirectory() const;   //!< Returns absolute path to the projects' code files.
      String GetConfigDirectory() const; //!< Returns absolute path to project's config files.
      String GetBinPath() const;         //!< Returns absolute path to the compiled binary file for the project.
      String GetPluginDirectory() const; //!< Returns absolute path to projects' plugin directory.
      String GetResourceRoot() const;    //!< Returns absolute path to projects' Resources directory.
      String GetActiveWorkspace() const;
      Project GetActiveProject() const;
      void SetActiveProject(const Project& project);
      void SetScene(const String& scene);

      void RefreshProjects();

      void SerializeEngineSettings() const;
      void DeSerializeEngineSettings();

     protected:
      XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
      XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

     public:
      std::vector<Project> m_projects;

     private:
      String m_activeWorkspace;
      Project m_activeProject;
    };

  } // namespace Editor
} // namespace ToolKit
