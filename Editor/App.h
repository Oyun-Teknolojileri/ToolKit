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
#include "FolderWindow.h"
#include "Global.h"
#include "Grid.h"
#include "Light.h"
#include "OutlinerWindow.h"
#include "PluginWindow.h"
#include "PropInspector.h"
#include "PublishManager.h"
#include "RenderSettingsView.h"
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
       * all referenced objects before switching projects or stoping the play session.
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
      void CreateWindows(XmlNode* parent);
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
      Window* GetActiveWindow();
      EditorViewport* GetActiveViewport();
      EditorViewport* GetViewport(const String& name);
      ConsoleWindow* GetConsole();
      FolderWindowRawPtrArray GetAssetBrowsers();
      OutlinerWindow* GetOutliner();
      PropInspector* GetPropInspector();
      RenderSettingsView* GetRenderSettingsView();
      void AddRenderSettingsView();

      template <typename T>
      T* GetWindow(const String& name)
      {
        for (Window* wnd : m_windows)
        {
          T* casted = dynamic_cast<T*>(wnd);
          if (casted)
          {
            if (casted->m_name == name)
            {
              return casted;
            }
          }
        }

        return nullptr;
      }

      template <typename T>
      std::vector<T*> GetAllWindows(const String& name)
      {
        std::vector<T*> list;
        for (Window* wnd : m_windows)
        {
          T* casted = dynamic_cast<T*>(wnd);
          if (casted)
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

      void HideGizmos();
      void ShowGizmos();

      // Simulation
      EditorViewport* GetSimulationWindow();
      void UpdateSimulation();
      float GetDeltaTime();

     protected:
      XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
      XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

     private:
      void CreateSimulationWindow(float width, float height);
      void AssignManagerReporters();
      void CreateAndSetNewScene(const String& name);
      void CreateEditorEntities();
      void DestroyEditorEntities();

     public:
      // UI elements.
      WindowRawPtrArray m_windows;
      String m_statusMsg;

      // Editor variables.
      float m_camSpeed         = 8.0; // Meters per sec.
      float m_mouseSensitivity = 0.5f;
      ThumbnailManager m_thumbnailManager;

      // Simulator settings.
      EditorViewport* m_simulationWindow = nullptr;
      SimulationSettings m_simulatorSettings;

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
      PublishManager* m_publishManager         = nullptr;
      GameMod m_gameMod                        = GameMod::Stop;
      SysCommandExecutionFn m_sysComExecFn     = nullptr;
      ShellOpenDirFn m_shellOpenDirFn          = nullptr;
      EditorLitMode m_sceneLightingMode        = EditorLitMode::EditorLit;
      EditorViewport* m_lastActiveViewport     = nullptr;
      Workspace m_workspace;
      StringArray m_customObjectMetaValues;    //!< Add menu shows this additional classes.
      DynamicMenuPtrArray m_customObjectsMenu; //!< Constructed menus based on m_customObjectMetaValues.

      // Snap settings.
      bool m_snapsEnabled = false; // Delta transforms.
      float m_moveDelta   = 0.25f;
      float m_rotateDelta = 15.0f;
      float m_scaleDelta  = 0.5f;

     private:
      // Internal states.
      bool m_onQuit = false;
      String m_newSceneName;
      float m_deltaTime   = 0.0f;
      bool m_isCompiling  = false;
      bool m_reloadPlugin = false;
    };

    extern void DebugMessage(const String& msg);
    extern void DebugMessage(const Vec3& vec);
    extern void DebugMessage(const char* msg, ...);
    extern void DebugCube(const Vec3& p, float size = 0.01f);
    extern void DebugLineStrip(const Vec3Array& pnts);

  } // namespace Editor
} // namespace ToolKit
