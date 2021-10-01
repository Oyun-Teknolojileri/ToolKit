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
      PluginWindow(XmlNode* node);
      virtual ~PluginWindow();
      
      virtual void Show() override;
      virtual Type GetType() const override;

      virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
      virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);
    };

  }

}