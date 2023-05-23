/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
