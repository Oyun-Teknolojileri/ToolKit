#pragma once

#include "ToolKit.h"
#include "UI.h"

namespace ToolKit
{

  namespace Editor
  {

    class PluginWindow : public Window
    {
     public:
      PluginWindow();
      explicit PluginWindow(XmlNode* node);
      virtual ~PluginWindow();

      void Show() override;
      Type GetType() const override;

      virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
      virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);

     private:
      bool m_simulationModeDisabled = false;
    };

  }  // namespace Editor

}  // namespace ToolKit
