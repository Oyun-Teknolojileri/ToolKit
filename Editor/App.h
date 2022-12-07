#pragma once

#include "ConsoleWindow.h"
#include "EditorPass.h"
#include "EditorScene.h"
#include "FolderWindow.h"
#include "Global.h"
#include "Grid.h"
#include "Light.h"
#include "OutlinerWindow.h"
#include "PluginWindow.h"
#include "PropInspector.h"
#include "PublishManager.h"
#include "ToolKit.h"
#include "Workspace.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace ToolKit
{
  namespace Editor
  {

    typedef std::shared_ptr<class Anchor> AnchorPtr;
    typedef std::function<void(int)> SysCommandDoneCallback;
    typedef std::function<int(StringView, bool, bool, SysCommandDoneCallback)>
        SysCommandExecutionFn;

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
      void SetGameMod(GameMod mod);
      void CompilePlugin();
      EditorScenePtr GetCurrentScene();
      void SetCurrentScene(const EditorScenePtr& scene);
      void FocusEntity(Entity* entity);

      /**
       * Executes the given system command.
       * @param cmd utf-8 formatted command string to execute.
       * @param async states that if the command will be run async or not.
       * @param showConsole states that the console that executes the cmd will
       * be shown or not.
       * @param callback function to call when the operation completed.
       * @return 0 if command can be started successfully.
       */
      int ExecSysCommand(
          StringView cmd,
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
            String nameWithoutId =
                casted->m_name.substr(0, casted->m_name.find_first_of('#'));
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

      void ShowSimulationWindow(float deltaTime);

      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
      float GetDeltaTime();

     private:
      void OverrideEntityConstructors();
      void CreateSimulationWindow(float width, float height);
      void AssignManagerReporters();
      void CreateAndSetNewScene(const String& name);
      void CreateEditorEntities();
      void DestroyEditorEntities();

     public:
      // UI elements.
      std::vector<Window*> m_windows;
      String m_statusMsg;

      // Editor variables.
      float m_camSpeed         = 8.0; // Meters per sec.
      float m_mouseSensitivity = 0.5f;
      Vec2 m_thumbnailSize     = Vec2(300.0f, 300.0f);
      std::unordered_map<String, RenderTargetPtr> m_thumbnailCache;

      // Simulator settings.
      EditorViewport* m_simulationWindow = nullptr;
      SimulationSettings m_simulatorSettings;

      // Editor objects.
      Grid* m_grid;
      Grid* m_2dGrid;
      Axis3d* m_origin;
      Cursor* m_cursor;
      Gizmo* m_gizmo = nullptr;
      AnchorPtr m_anchor;
      EntityRawPtrArray m_perFrameDebugObjects;
      std::shared_ptr<Arrow2d> m_dbgArrow;
      std::shared_ptr<LineBatch> m_dbgFrustum;

      // Editor states.
      int m_fps                                = 0;
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
      EditorLitMode m_sceneLightingMode        = EditorLitMode::EditorLit;
      Workspace m_workspace;

      // Snap settings.
      bool m_snapsEnabled = false; // Delta transforms.
      float m_moveDelta   = 0.25f;
      float m_rotateDelta = 15.0f;
      float m_scaleDelta  = 0.5f;

      Renderer* m_renderer;
      bool m_windowCamLoad = true;

     private:
      // Internal states.
      bool m_onQuit = false;
      String m_newSceneName;
      float m_deltaTime = 0.0f;

      MaterialPtr lightModeMat = nullptr;
    };

    extern void DebugMessage(const String& msg);
    extern void DebugMessage(const Vec3& vec);
    extern void DebugMessage(const char* msg, ...);
    extern void DebugCube(const Vec3& p, float size = 0.01f);
    extern void DebugLineStrip(const Vec3Array& pnts);

  } // namespace Editor
} // namespace ToolKit
