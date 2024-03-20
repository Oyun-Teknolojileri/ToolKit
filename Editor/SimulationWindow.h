/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

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
      FullHD,
      QHD,
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
      Galaxy_A51_A71,
      Count
    };

    struct SimulationSettings
    {
      bool Windowed                 = false;
      bool Landscape                = false;
      float Width                   = 500.0f;
      float Height                  = 500.0f;
      float Scale                   = 1.0f;
      EmulatorResolution Resolution = EmulatorResolution::Custom;
    };

    class SimulationWindow : public Window
    {
     public:
      TKDeclareClass(SimulationWindow, Window);

      SimulationWindow();
      virtual ~SimulationWindow();

      void Show() override;

     private:
      void UpdateSimulationWndSize();
      void ShowHeader();
      void ShowActionButtons();
      void ShowSettings();
      String EmuResToString(EmulatorResolution emuRes);
      void UpdateCanvas(uint width, uint heigth);

      void AddResolutionName(const String& name);
      void RemoveResolutionName(const String& name);
      void RemoveResolutionName(size_t index);

     public:
      int m_numDefaultResNames               = 0;

      std::vector<IVec2> m_screenResolutions = {
          IVec2(480, 667),   // default
          IVec2(1920, 1080), // FullHD
          IVec2(2048, 1080), // QHD
          IVec2(375, 667),   // Iphone_SE,
          IVec2(414, 896),   // Iphone_XR,
          IVec2(390, 844),   // Iphone_12_Pro,
          IVec2(393, 851),   // Pixel_5,
          IVec2(412, 915),   // Galaxy_S20_Ultra,
          IVec2(412, 915),   // Galaxy_Note20,
          IVec2(390, 844),   // Galaxy_Note20_Ultra,
          IVec2(820, 118),   // Ipad_Air,
          IVec2(768, 102),   // Ipad_Mini,
          IVec2(912, 139),   // Surface_Pro_7,
          IVec2(540, 720),   // Surface_Duo,
          IVec2(412, 914)    // Galaxy_A51_A71
      };

      std::vector<String> m_emulatorResolutionNames = {"Custom Resolutions\0",
                                                       "Full HD (1080p)\0",
                                                       "QHD (1440p)\0",
                                                       "iPhone SE (375x667)\0",
                                                       "iPhone XR (414x896)\0",
                                                       "iPhone 12 Pro (390x844)\0",
                                                       "Pixel 5 (393x851)\0",
                                                       "Galaxy S20 Ultra (412x915)\0",
                                                       "Galaxy Note 20 (412x915)\0",
                                                       "Galaxy Note 20 Ultra (390x844)\0",
                                                       "Ipad Air  (820x118)\0",
                                                       "Ipad Mini (768x102)\0",
                                                       "Surface Pro 7 (912x139)\0",
                                                       "Surface Duo (540x720)\0",
                                                       "Galaxy A51 / A71 (412x914)\0"};

     private:
      SimulationSettings* m_settings         = nullptr;
      bool m_simulationModeDisabled          = false;
      bool m_resolutionSettingsWindowEnabled = false;
      ;
    };

  } // namespace Editor
} // namespace ToolKit
