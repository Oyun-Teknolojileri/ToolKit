#pragma once

#include <unordered_map>
#include <vector>

#include "ToolKit.h"
#include "EditorScene.h"
#include "Workspace.h"
#include "GlobalDef.h"
#include "Light.h"
#include "PublishManager.h"

namespace ToolKit
{
  class Renderer;
  class Light;
  class Cube;
  class Sphere;
  class Camera;
  class Animation;
  class Billboard;

  namespace Editor
  {
    class EditorViewport;
    class Grid;
    class Axis3d;
    class Cursor;
    class ConsoleWindow;
    class FolderWindow;
    class OutlinerWindow;
    class PropInspector;
    class MaterialInspector;
    class Window;
    class Gizmo;
    class PublishManager;

    class App : Serializable
    {
     public:
      enum class GameMod
      {
        Playing,
        Paused,
        Stop
      };

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
      EditorScenePtr GetCurrentScene();
      void SetCurrentScene(const EditorScenePtr& scene);

      // UI.
      void ResetUI();
      void DeleteWindows();
      void CreateWindows(XmlNode* parent);

      // Import facilities.
      int Import(const String& fullPath, const String& subDir, bool overwrite);
      bool CanImport(const String& fullPath);

      // Workspace.
      void OpenScene(const String& fullPath);
      void MergeScene(const String& fullPath);
      void ApplyProjectSettings(bool setDefaults);
      void OpenProject(const Project& project);
      void PackResources();

      // UI
      Window* GetActiveWindow();
      EditorViewport* GetActiveViewport();
      EditorViewport* GetViewport(const String& name);
      ConsoleWindow* GetConsole();
      FolderWindow* GetAssetBrowser();
      OutlinerWindow* GetOutliner();
      PropInspector* GetPropInspector();
      MaterialInspector* GetMaterialInspector();

      template<typename T>
      T* GetWindow(const String& name)
      {
        for (Window* wnd : m_windows)
        {
          T* casted = dynamic_cast<T*> (wnd);
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

      // Quick selected render implementation.
      void RenderSelected
      (
        EditorViewport* viewport,
        EntityRawPtrArray selecteds
      );
      void RenderGizmo
      (
        EditorViewport* viewport,
        Gizmo* gizmo
      );
      void RenderComponentGizmo
      (
        EditorViewport* viewport,
        EntityRawPtrArray selecteds
      );
      void ShowPlayWindow(float deltaTime);

      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

     private:
      void CreateSimulationWindow();
      void AssignManagerReporters();
      void CreateAndSetNewScene(const String& name);

     public:
      // UI elements.
      std::vector<Window*> m_windows;
      String m_statusMsg;

      // Editor variables.
      float m_camSpeed = 8.0;  // Meters per sec.
      float m_mouseSensitivity = 0.5f;
      Vec2 m_thumbnailSize = Vec2(300.0f, 300.0f);
      std::unordered_map<String, RenderTargetPtr> m_thumbnailCache;

      // Emulator settings.
      bool m_runWindowed = false;
      float m_playWidth = 640.0f;
      float m_playHeight = 480.0f;
      EditorViewport* m_playWindow = nullptr;

      // Editor objects.
      Grid* m_grid;
      Grid* m_2dGrid;
      Axis3d* m_origin;
      Cursor* m_cursor;
      Gizmo* m_gizmo = nullptr;
      Billboard* m_environmentBillboard = nullptr;
      std::vector<Entity*> m_perFrameDebugObjects;

      // 3 point lighting system.
      Node* m_lightMaster = nullptr;
      LightRawPtrArray m_sceneLights;  // { 0:key 1:fill, 2:back }

      // Editor states.
      int m_fps = 0;
      bool m_showPickingDebug = false;
      bool m_showStateTransitionsDebug = false;
      bool m_showOverlayUI = true;
      bool m_showOverlayUIAlways = true;
      bool m_importSlient = false;
      bool m_showSelectionBoundary = false;
      bool m_windowMaximized = false;
      byte m_showGraphicsApiErrors = 0;
      TransformationSpace m_transformSpace = TransformationSpace::TS_WORLD;
      Workspace m_workspace;
      PublishManager* m_publishManager = nullptr;
      GameMod m_gameMod = GameMod::Stop;

      // Snap settings.
      bool m_snapsEnabled = false;  // Delta transforms.
      float m_moveDelta = 0.25f;
      float m_rotateDelta = 15.0f;
      float m_scaleDelta = 0.5f;

      Renderer* m_renderer;

     private:
      // Internal states.
      bool m_onNewScene = false;
      bool m_onQuit = false;

      bool m_windowCamLoad = true;
    };

    extern void DebugMessage(const String& msg);
    extern void DebugMessage(const Vec3& vec);
    extern void DebugMessage(const char* msg, ...);
    extern void DebugCube(const Vec3& p, float size = 0.01f);
    extern void DebugLineStrip(const Vec3Array& pnts);

  }  // namespace Editor
}  // namespace ToolKit
