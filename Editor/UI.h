#pragma once

#include <vector>
#include <functional>

#include "ImGui/imgui.h"
#include "Types.h"
#include "Serialize.h"


namespace ToolKit
{

  // Global Style Decelerations
  static const ImGuiTreeNodeFlags g_treeNodeFlags =
  ImGuiTreeNodeFlags_OpenOnArrow
  | ImGuiTreeNodeFlags_OpenOnDoubleClick
  | ImGuiTreeNodeFlags_SpanAvailWidth
  | ImGuiTreeNodeFlags_AllowItemOverlap
  | ImGuiTreeNodeFlags_FramePadding;

  const float g_indentSpacing = 6.0f;

  namespace Editor
  {
    class Window : public Serializable
    {
     public:
      enum class Type
      {
        Viewport,
        Console,
        InputPopup,
        Browser,
        Outliner,
        Inspector,
        MaterialInspector,
        PluginWindow,
        Viewport2d
      };

     public:
      Window();
      virtual ~Window();
      virtual void Show() = 0;
      virtual Type GetType() const = 0;
      void SetVisibility(bool visible);

      // Window queries.
      bool IsActive() const;
      bool IsVisible() const;
      bool MouseHovers() const;
      bool CanDispatchSignals() const;  // If active & visible & mouse hovers.

      // System calls.
      virtual void DispatchSignals() const;

      virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
      virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);

     protected:
      // Internal window handling.
      void HandleStates();
      void SetActive();
      void ModShortCutSignals(const IntArray& mask = {}) const;

     protected:
      // States.
      bool m_visible = true;
      bool m_active = false;
      bool m_mouseHover = false;

     public:
      String m_name;
      uint m_id;

     private:
      // Internal unique id generator.
      static uint m_baseId;
    };

    class StringInputWindow : public Window
    {
     public:
      StringInputWindow(const String& name, bool showCancel);
      void Show() override;
      Type GetType() const override { return Window::Type::InputPopup; }

     public:
      std::function<void(const String& val)> m_taskFn;
      String m_inputVal;
      String m_inputLabel;
      String m_hint;

     private:
      bool m_showCancel;
    };

    class YesNoWindow : public Window
    {
     public:
      explicit YesNoWindow(const String& name, const String& msg = "");
      YesNoWindow
      (
        const String& name,
        const String& yesBtnText,
        const String& noBtnText,
        const String& msg,
        bool showCancel
      );
      void Show() override;
      Type GetType() const override { return Window::Type::InputPopup; }

     public:
      std::function<void()> m_yesCallback;
      std::function<void()> m_noCallback;
      String m_msg;
      String m_yesText;
      String m_noText;
      bool m_showCancel = false;
    };

    class UI
    {
     public:
      static void Init();
      static void UnInit();
      static void ShowDock();
      static void InitIcons();
      static void InitTheme();
      static void InitSettings();
      static void ShowUI();
      static void BeginUI();
      static void EndUI();
      static void ShowAppMainMenuBar();
      static void ShowMenuFile();
      static void ShowMenuWindows();
      static void ShowMenuProjects();
      static void ShowImportWindow();
      static void ShowSearchForFilesWindow();
      static void HelpMarker
      (
        const String& key,
        const char* desc,
        float wait = m_hoverTimeForHelp
      );
      static void ShowNewSceneWindow();

      // Custom widgets.
      static bool ImageButtonDecorless
      (
        uint textureID,
        const Vec2& size,
        bool flipImage
      );
      static bool ToggleButton
      (
        uint textureID,
        const Vec2& size,
        bool pushState
      );
      static bool ToggleButton
      (
       const String& text,
        const Vec2& size,
        bool pushState
      );
      static bool BeginCenteredTextButton
      (
        const String& text,
        const String& id = ""
      );
      static void EndCenteredTextButton();

     public:
      static bool m_showNewSceneWindow;
      static bool m_imguiSampleWindow;
      static bool m_windowMenushowMetrics;
      static float m_hoverTimeForHelp;

      // Volatile windows. (Pop-ups etc.)
      static std::vector<Window*> m_volatileWindows;

      static struct Import
      {
        bool showImportWindow = false;
        bool overwrite = false;
        StringArray files;
        String subDir;
        float scale = 1.0f;
      } ImportData;

      static struct SearchFile
      {
        bool showSearchFileWindow = false;
        StringArray missingFiles;
        StringArray searchPaths;
      } SearchFileData;

      // Some actions needed to be run after ui rendered.
      static std::vector<std::function<void()>> m_postponedActions;

      // Toolbar Icons.
      static TexturePtr m_selectIcn;
      static TexturePtr m_cursorIcn;
      static TexturePtr m_moveIcn;
      static TexturePtr m_rotateIcn;
      static TexturePtr m_scaleIcn;
      static TexturePtr m_appIcon;
      static TexturePtr m_snapIcon;
      static TexturePtr m_audioIcon;
      static TexturePtr m_cameraIcon;
      static TexturePtr m_clipIcon;
      static TexturePtr m_fileIcon;
      static TexturePtr m_folderIcon;
      static TexturePtr m_imageIcon;
      static TexturePtr m_lightIcon;
      static TexturePtr m_materialIcon;
      static TexturePtr m_meshIcon;
      static TexturePtr m_armatureIcon;
      static TexturePtr m_codeIcon;
      static TexturePtr m_boneIcon;
      static TexturePtr m_worldIcon;
      static TexturePtr m_axisIcon;
      static TexturePtr m_playIcon;
      static TexturePtr m_pauseIcon;
      static TexturePtr m_stopIcon;
      static TexturePtr m_vsCodeIcon;
      static TexturePtr m_collectionIcon;
      static TexturePtr m_arrowsIcon;
      static TexturePtr m_lockIcon;
      static TexturePtr m_visibleIcon;
      static TexturePtr m_invisibleIcon;
      static TexturePtr m_lockedIcon;
      static TexturePtr m_unlockedIcon;
      static TexturePtr m_viewZoomIcon;
      static TexturePtr m_gridIcon;
      static TexturePtr m_skyIcon;
      static TexturePtr m_closeIcon;
      static TexturePtr m_studioLightsToggleIcon;
    };
  }  // namespace Editor
}  // namespace ToolKit
