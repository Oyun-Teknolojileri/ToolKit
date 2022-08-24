#pragma once

#include "ToolKit.h"
#include "UI.h"

namespace ToolKit
{

  namespace Editor
  {
    enum class GameMod
    {
      Playing,
      Paused,
      Stop
    };

    enum class EmulatorResolution
    {
      Custom,
      Iphone_SE,
      Iphone_XR,
      Iphone_12_Pro,
      Pixel_5,
      Galaxy_S20_Ultra,
      Galaxy_Note20,
      Galaxy_Note20_Ultra,
      Ipad_Air,
      Ipad_Mini,
      Surface_Pro_7,
      Surface_Duo,
      Galaxy_A51_A71
    };

    struct EmulatorSettings
    {
      bool runWindowed = false;
      bool landscape = false;
      float playWidth = 640.0f;
      float playHeight = 480.0f;
      float zoomAmount = 1.0f;
      EmulatorResolution emuRes = EmulatorResolution::Custom;
    };

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
      void ShowHeader();
      void ShowSimButtons();
      void ShowSettings();
      String EmuResToString(EmulatorResolution emuRes);
      bool m_simulationModeDisabled = false;
    };

  }  // namespace Editor

}  // namespace ToolKit
