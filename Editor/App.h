/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Anchor.h"
#include "ConsoleWindow.h"
#include "DynamicMenu.h"
#include "EditorRenderer.h"
#include "EditorScene.h"
#include "EditorTypes.h"
#include "FolderWindow.h"
#include "Grid.h"
#include "Light.h"
#include "OutlinerWindow.h"
#include "PropInspectorWindow.h"
#include "PublishManager.h"
#include "RenderSettingsWindow.h"
#include "SimulationWindow.h"
#include "StatsWindow.h"
#include "Thumbnail.h"
#include "Workspace.h"

namespace ToolKit
{
  namespace Editor
  {

    typedef std::function<void(int)> SysCommandDoneCallback;
    typedef std::function<int(StringView, bool, bool, SysCommandDoneCallback)> SysCommandExecutionFn;
    typedef std::function<void(const StringView)> ShellOpenDirFn;

    class App : Serializable
    {
     public:
      App(int windowWidth, int windowHeight);
      virtual ~App();

      void Init();
      void Destroy();
      void Frame(float deltaTime);
      void OnResize(uint width, uint height);
      void OnNewScene(const String& name);
      void OnSaveScene();
      void OnSaveAsScene();
      void OnQuit();
      void OnNewProject(const String& name);
      void SetGameMod(const GameMod mod);
      void CompilePlugin();
      void LoadProjectPlugin();
      bool IsCompiling();
      EditorScenePtr GetCurrentScene();
      void SetCurrentScene(const EditorScenePtr& scene);
      void FocusEntity(EntityPtr entity);

      /**
       * Clears all the data cached for current project / scene. Required to clear
       * all referenced objects before switching projects or stopping the play session.
       */
      void ClearSession();

      /**
       * Clears all the objects created in PIE session.
       */
      void ClearPlayInEditorSession();

      /**
       * Executes the given system command.
       * @param cmd utf-8 formatted command string to execute.
       * @param async states that if the command will be run async or not.
       * @param showConsole states that the console that executes the cmd will
       * be shown or not.
       * @param callback function to call when the operation completed.
       * @return 0 if command can be started successfully.
       */
      int ExecSysCommand(StringView cmd,
                         bool async,
                         bool showConsole,
                         /**
                          * Callback function upon completion of the system command.
                          * @param int is the return value of the cmd.
                          */
                         SysCommandDoneCallback callback = nullptr);

      // UI.
      void ResetUI();
      void DeleteWindows();
      void ReconstructDynamicMenus();

      // Import facilities.
      int Import(const String& fullPath, const String& subDir, bool overwrite);
      bool CanImport(const String& fullPath);
      void ManageDropfile(const StringView& fileName);

      // Workspace.
      void OpenScene(const String& fullPath);
      void MergeScene(const String& fullPath);
      void LinkScene(const String& fullPath);
      void ApplyProjectSettings(bool setDefaults);
      void OpenProject(const Project& project);
      void PackResources();
      void SaveAllResources();

      // UI
      WindowPtr GetActiveWindow();
      EditorViewportPtr GetActiveViewport();
      EditorViewportPtr GetViewport(const String& name);
      ConsoleWindowPtr GetConsole();
      FolderWindowRawPtrArray GetAssetBrowsers();
      OutlinerWindowPtr GetOutliner();
      PropInspectorWindowPtr GetPropInspector();
      RenderSettingsWindowPtr GetRenderSettingsWindow();
      StatsWindowPtr GetStatsWindow();

      template <typename T>
      std::shared_ptr<T> GetWindow(const String& name)
      {
        for (WindowPtr wnd : m_windows)
        {
          if (T* casted = wnd->As<T>())
          {
            if (casted->m_name == name)
            {
              return Cast<T>(wnd);
            }
          }
        }

        return nullptr;
      }

      template <typename T>
      std::vector<T*> GetAllWindows(const String& name)
      {
        std::vector<T*> list;
        for (WindowPtr wnd : m_windows)
        {
          if (T* casted = wnd->As<T>())
          {
            String nameWithoutId = casted->m_name.substr(0, casted->m_name.find_first_of('#'));
            if (nameWithoutId == name)
            {
              list.push_back(casted);
            }
          }
        }
        return list;
      }

      template <typename T>
      std::shared_ptr<T> CreateOrRetrieveWindow(const String& name = String())
      {
        if (T::StaticClass()->IsSublcassOf(Window::StaticClass()))
        {
          for (WindowPtr wnd : m_windows)
          {
            if (T* wndCasted = wnd->As<T>())
            {
              if (wnd->m_name == name)
              {
                return Cast<T>(wnd);
              }
            }
          }

          std::shared_ptr<T> wnd = MakeNewPtr<T>();
          wnd->m_name            = name;
          wnd->SetVisibility(false);
          m_windows.push_back(wnd);
          return wnd;
        }

        return nullptr;
      }

      void ReInitViewports();

      void HideGizmos();
      void ShowGizmos();

      // Simulation
      EditorViewportPtr GetSimulationViewport();
      void UpdateSimulation();

      // Last Frame Stats
      float GetDeltaTime();
      uint64 GetLastFrameDrawCallCount();
      uint64 GetLastFrameHWRenderPassCount();

     protected:
      XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
      XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
      void DeserializeWindows(XmlNode* parent);

     private:
      void CreateSimulationViewport();
      void AssignManagerReporters();
      void CreateAndSetNewScene(const String& name);
      void CreateEditorEntities();
      void DestroyEditorEntities();
      void CreateNewScene();

     public:
      // UI elements.
      WindowPtrArray m_windows; //!< Persistent windows that get serialized with editor.
      String m_statusMsg;

      // Editor variables.
      float m_camSpeed         = 8.0; // Meters per sec.
      float m_mouseSensitivity = 0.5f;
      ThumbnailManager m_thumbnailManager;

      // Simulator settings.
      EditorViewportPtr m_simulationViewport;
      SimulationSettings m_simulatorSettings;

      // Publisher.
      PublishManager* m_publishManager = nullptr;
      AndroidBuildWindowPtr m_androidBuildWindow;

      // Editor objects.
      GridPtr m_grid;
      GridPtr m_2dGrid;
      Axis3dPtr m_origin;
      CursorPtr m_cursor;
      GizmoPtr m_gizmo;
      AnchorPtr m_anchor;
      EntityPtrArray m_perFrameDebugObjects;
      Arrow2dPtr m_dbgArrow;
      LineBatchPtr m_dbgFrustum;

      // Editor states.
      int m_fps                                = 0;
      uint m_totalFrameCount                   = 0;
      bool m_showPickingDebug                  = false;
      bool m_showStateTransitionsDebug         = false;
      bool m_showOverlayUI                     = true;
      bool m_showOverlayUIAlways               = true;
      bool m_importSlient                      = false;
      bool m_showSelectionBoundary             = false;
      bool m_showDirectionalLightShadowFrustum = false;
      bool m_selectEffectingLights             = false;
      bool m_windowMaximized                   = false;
      byte m_showGraphicsApiErrors             = 0;
      TransformationSpace m_transformSpace     = TransformationSpace::TS_WORLD;
      GameMod m_gameMod                        = GameMod::Stop;
      SysCommandExecutionFn m_sysComExecFn     = nullptr;
      ShellOpenDirFn m_shellOpenDirFn          = nullptr;
      EditorLitMode m_sceneLightingMode        = EditorLitMode::EditorLit;
      EditorViewportPtr m_lastActiveViewport   = nullptr;
      Workspace m_workspace;
      StringArray m_customObjectMetaValues;    //!< Add menu shows this additional classes.
      DynamicMenuPtrArray m_customObjectsMenu; //!< Constructed menus based on m_customObjectMetaValues.

      // Snap settings.
      bool m_snapsEnabled                 = false; // Delta transforms.
      float m_moveDelta                   = 0.25f;
      float m_rotateDelta                 = 15.0f;
      float m_scaleDelta                  = 0.5f;

      // Last Frame Stats
      uint64 m_lastFrameDrawCallCount     = 0;
      uint64 m_lastFrameHWRenderPassCount = 0;

     private:
      // Internal states.
      bool m_onQuit = false;
      String m_newSceneName;
      float m_deltaTime   = 0.0f;
      bool m_isCompiling  = false;
      bool m_reloadPlugin = false;
    };

  } // namespace Editor
} // namespace ToolKit
