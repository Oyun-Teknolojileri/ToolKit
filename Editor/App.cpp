#include "App.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>

#include "Gizmo.h"
#include "Renderer.h"
#include "UI.h"
#include "EditorViewport.h"
#include "Primative.h"
#include "Node.h"
#include "GlobalDef.h"
#include "OverlayUI.h"
#include "Grid.h"
#include "Camera.h"
#include "Mod.h"
#include "ConsoleWindow.h"
#include "FolderWindow.h"
#include "OutlinerWindow.h"
#include "PropInspector.h"
#include "MaterialInspector.h"
#include "PluginWindow.h"
#include "EditorViewport2d.h"
#include "DirectionComponent.h"
#include "Action.h"
#include "GL/glew.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    App::App(int windowWidth, int windowHeight)
      : m_workspace(this)
    {
      m_cursor = nullptr;
      m_renderer = Main::GetInstance()->m_renderer;
      m_renderer->m_windowWidth = windowWidth;
      m_renderer->m_windowHeight = windowHeight;
      m_statusMsg = "OK";
    }

    App::~App()
    {
      Destroy();
    }

    void App::Init()
    {
      AssignManagerReporters();

      // Create editor objects.
      m_cursor = new Cursor();
      m_origin = new Axis3d();

      uint gridSize = 100000;
      m_grid = new Grid(UVec2(gridSize));
      m_grid->Resize(UVec2(gridSize), AxisLabel::ZX, 0.025f);

      m_2dGrid = new Grid
      (
        UVec2
        (
          g_app->m_emulatorSettings.playWidth,
          g_app->m_emulatorSettings.playHeight
        )
      );
      m_2dGrid->Resize
      (
        UVec2
        (
          g_app->m_emulatorSettings.playWidth,
          g_app->m_emulatorSettings.playHeight
        ),
        AxisLabel::XY, 10.0
      );  // Generate grid cells 10 x 10

      // Lights and camera.
      m_lightMaster = new Node();

      float intensity = 1.5f;
      DirectionalLight* light = new DirectionalLight();
      light->SetColorVal(Vec3(0.267f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(180.0f));
      m_lightMaster->AddChild(light->m_node);
      m_sceneLights.push_back(light);

      light = new DirectionalLight();
      light->SetColorVal(Vec3(0.55f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(-20.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(-20.0f));
      m_lightMaster->AddChild(light->m_node);
      m_sceneLights.push_back(light);

      light = new DirectionalLight();
      light->SetColorVal(Vec3(0.15f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(90.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(-45.0f));
      m_lightMaster->AddChild(light->m_node);
      m_sceneLights.push_back(light);

      light = new DirectionalLight();
      light->SetColorVal(Vec3(0.1f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(120.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(60.0f));
      m_lightMaster->AddChild(light->m_node);
      m_sceneLights.push_back(light);

      ModManager::GetInstance()->Init();
      ModManager::GetInstance()->SetMod(true, ModId::Select);
      ActionManager::GetInstance()->Init();

      m_workspace.Init();
      String sceneName = "New Scene" + SCENE;
      EditorScenePtr scene = std::make_shared<EditorScene>
      (
        ScenePath(sceneName)
      );
      scene->m_name = sceneName;
      scene->m_newScene = true;
      SetCurrentScene(scene);
      ApplyProjectSettings(m_onNewScene);


      if (!CheckFile(m_workspace.GetActiveWorkspace()))
      {
        StringInputWindow* wsDir = new StringInputWindow
        (
          "Set Workspace Directory##SetWsdir",
          false
        );
        wsDir->m_hint = "User/Documents/ToolKit";
        wsDir->m_inputLabel = "Workspace Directory";
        wsDir->m_name = "Set Workspace Directory";
        wsDir->m_taskFn = [](const String& val) -> void
        {
          String cmd = "SetWorkspaceDir --path \"" + val + "\"";
          g_app->GetConsole()->ExecCommand(cmd);
        };
      }
      else
      {
        m_workspace.RefreshProjects();
      }

      m_emulatorSettings.emuRes = EmulatorResolution::Custom;
      m_publishManager = new PublishManager();
    }

    void App::Destroy()
    {
      // UI.
      DeleteWindows();

      GetCurrentScene()->Destroy(false);

      SafeDel(m_publishManager);

      // Editor objects.
      SafeDel(m_2dGrid);
      SafeDel(m_grid);
      SafeDel(m_origin);
      SafeDel(m_cursor);

      for (Light* light : m_sceneLights)
      {
        SafeDel(light);
      }
      SafeDel(m_lightMaster);
      m_sceneLights.clear();

      for (Entity* dbgObj : m_perFrameDebugObjects)
      {
        SafeDel(dbgObj);
      }
      m_perFrameDebugObjects.clear();

      GetAnimationPlayer()->m_records.clear();

      ModManager::GetInstance()->UnInit();
      ActionManager::GetInstance()->UnInit();
    }

    void App::Frame(float deltaTime)
    {
      UI::BeginUI();
      UI::ShowUI();

      // Update animations.
      GetAnimationPlayer()->Update(MillisecToSec(deltaTime));

      // Update Mods.
      ModManager::GetInstance()->Update(deltaTime);
      std::vector<EditorViewport*> viewports;
      for (Window* wnd : m_windows)
      {
        if (EditorViewport* vp = dynamic_cast<EditorViewport*> (wnd))
        {
          viewports.push_back(vp);
          GetCurrentScene()->UpdateBillboardTransforms(vp);
        }
        wnd->DispatchSignals();
      }

      ShowPlayWindow(deltaTime);

      // Selected entities
      EntityRawPtrArray selecteds;
      GetCurrentScene()->GetSelectedEntities(selecteds);

      LightRawPtrArray allLights = GetCurrentScene()->GetLights();

      // Enable light gizmos
      for (Light* light : allLights)
      {
        bool found = false;
        for (Entity* ntt : selecteds)
        {
          if (light->GetIdVal() == ntt->GetIdVal())
          {
            EnableLightGizmo(light, true);
            found = true;
            break;
          }
        }

        if (!found)
        {
          EnableLightGizmo(light, false);
        }
      }

      // Take all lights in an array
      LightRawPtrArray gameLights = GetCurrentScene()->GetLights();

      if (m_studioLightsActive)
      {
        gameLights.insert
        (
          gameLights.end(),
          m_sceneLights.begin(),
          m_sceneLights.end()
        );
      }

      // Sort lights by type
      auto lightSortFn = [](Light* light1, Light* light2) -> bool
      {
        return
        (
          light1->GetLightType() == LightTypeEnum::LightDirectional
          &&
          (
            light2->GetLightType() == LightTypeEnum::LightSpot
            || light2->GetLightType() == LightTypeEnum::LightPoint
          )
        );
      };
      std::stable_sort(gameLights.begin(), gameLights.end(), lightSortFn);

      // Render Viewports.
      for (EditorViewport* viewport : viewports)
      {
        // Update scene lights for the current view.
        Camera* viewCam = viewport->GetCamera();
        m_lightMaster->OrphanSelf();
        if (m_studioLightsActive)
        {
          viewCam->m_node->AddChild(m_lightMaster);
        }

        // PlayWindow is drawn on perspective. Thus, skip perspective.
        if (m_gameMod != GameMod::Stop && !m_emulatorSettings.runWindowed)
        {
          if (viewport->m_name == g_3dViewport)
          {
            continue;
          }
        }

        viewport->Update(deltaTime);

        GetCurrentScene()->UpdateBillboardTransforms(viewport);

        if (viewport->IsVisible())
        {
          // Render scene.
          m_renderer->RenderScene(GetCurrentScene(), viewport, gameLights);

          // Render grid.
          Camera* cam = viewport->GetCamera();
          auto gridDrawFn = [this, &cam](Grid* grid) -> void
          {
            m_renderer->m_gridCellSize = grid->m_gridCellSize;
            m_renderer->m_gridHorizontalAxisColor = grid->m_horizontalAxisColor;
            m_renderer->m_gridVerticalAxisColor = grid->m_verticalAxisColor;
            m_renderer->Render(grid, cam);
          };

          Grid* grid = viewport->GetType() == Window::Type::Viewport2d ?
            m_2dGrid : m_grid;

          gridDrawFn(grid);

          // Render fixed scene objects.
          if (viewport->GetType() != Window::Type::Viewport2d)
          {
            m_origin->LookAt(cam, viewport->m_zoom);
            m_renderer->Render(m_origin, cam);

            m_cursor->LookAt(cam, viewport->m_zoom);
            m_renderer->Render(m_cursor, cam);
          }

          // Render gizmo.
          RenderGizmo(viewport, m_gizmo);

          RenderComponentGizmo(viewport, selecteds);
        }

        // Render debug objects.
        if (!m_perFrameDebugObjects.empty())
        {
          for (Entity* dbgObj : m_perFrameDebugObjects)
          {
            m_renderer->Render(dbgObj, viewCam);
            SafeDel(dbgObj);
          }
          m_perFrameDebugObjects.clear();
        }
      }

      if (m_gameMod != GameMod::Playing)
      {
        for (EditorViewport* viewport : viewports)
        {
          RenderSelected(viewport, selecteds);
        }
      }

      // Viewports set their own render target.
      // Set the app framebuffer back for UI.
      m_renderer->SetRenderTarget(nullptr);

      // Render UI.
      UI::EndUI();

      // Remove editor lights
      m_lightMaster->OrphanSelf();
    }

    void App::OnResize(uint width, uint height)
    {
      m_renderer->m_windowWidth = width;
      m_renderer->m_windowHeight = height;
      glViewport(0, 0, width, height);
    }

    void App::OnNewScene(const String& name)
    {
      m_onNewScene = true;

      Destroy();
      Init();
      CreateAndSetNewScene(name);
      m_workspace.SetScene(name);
      m_onNewScene = false;
    }

    void App::OnSaveScene()
    {
      // Prevent overriding default scene.
      EditorScenePtr currScene = GetCurrentScene();
      if
      (
        GetSceneManager()->GetDefaultResource(ResourceType::Scene)
        == currScene->GetFile()
      )
      {
        currScene->SetFile(ScenePath("New Scene" + SCENE));
        return OnSaveAsScene();
      }

      auto saveFn = []() -> void
      {
        g_app->GetCurrentScene()->Save(false);
        g_app->m_statusMsg = "Scene saved";
        g_app->GetAssetBrowser()->UpdateContent();
      };

      // File existance check.
      String fullPath = currScene->GetFile();
      if (currScene->m_newScene && CheckFile(fullPath))
      {
        String msg = "Scene " + fullPath
          + " exist on the disk.\nOverride the existing scene ?";
        YesNoWindow* overrideScene = new YesNoWindow
        (
          "Override existing file##OvrdScn",
          msg
        );
        overrideScene->m_yesCallback = [&saveFn]()
        {
          saveFn();
        };

        overrideScene->m_noCallback = []()
        {
          g_app->GetConsole()->AddLog
          (
            "Scene has not been saved.\n"
            "A scene with the same name exist. Use File->SaveAs.",
            LogType::Error
          );
        };

        UI::m_volatileWindows.push_back(overrideScene);
      }
      else
      {
        saveFn();
      }
    }

    void App::OnSaveAsScene()
    {
      StringInputWindow* inputWnd = new StringInputWindow
      (
        "SaveScene##SvScn1",
        true
      );
      inputWnd->m_inputLabel = "Name";
      inputWnd->m_hint = "Scene name";
      inputWnd->m_taskFn = [](const String& val)
      {
        String path;
        EditorScenePtr currScene = g_app->GetCurrentScene();
        DecomposePath(currScene->GetFile(), &path, nullptr, nullptr);
        String fullPath = ConcatPaths({ path, val + SCENE });
        currScene->SetFile(fullPath);
        currScene->m_name = val;
        g_app->OnSaveScene();
      };
    }

    void App::OnQuit()
    {
      if (m_gameMod != GameMod::Stop)
      {
        SetGameMod(GameMod::Stop);
        return;
      }

      if (!m_onQuit)
      {
        YesNoWindow* reallyQuit = new YesNoWindow
        (
          "Quiting... Are you sure?##ClsApp"
        );
        reallyQuit->m_yesCallback = [this]()
        {
          m_workspace.Serialize(nullptr, nullptr);
          Serialize(nullptr, nullptr);
          g_running = false;
        };

        reallyQuit->m_noCallback = [this]()
        {
          m_onQuit = false;
        };

        UI::m_volatileWindows.push_back(reallyQuit);
        m_onQuit = true;
      }
    }

    void App::OnNewProject(const String& name)
    {
      if (m_workspace.GetActiveWorkspace().empty())
      {
        GetConsole()->AddLog
        (
          "No workspace. Project can't be created.",
          LogType::Error
        );
        return;
      }

      String fullPath = ConcatPaths
      (
        { m_workspace.GetActiveWorkspace(), name }
      );
      if (CheckFile(fullPath))
      {
        GetConsole()->AddLog
        (
          "Project already exist.",
          LogType::Error
        );
        return;
      }

      std::filesystem::create_directories
      (
        ConcatPaths({ fullPath, "Resources", "Audio" })
      );
      std::filesystem::create_directories
      (
        ConcatPaths({ fullPath, "Resources", "Fonts" })
      );
      std::filesystem::create_directories
      (
        ConcatPaths({ fullPath, "Resources", "Materials" })
      );
      std::filesystem::create_directories
      (
        ConcatPaths({ fullPath, "Resources", "Meshes" })
      );
      std::filesystem::create_directories
      (
        ConcatPaths({ fullPath, "Resources", "Scenes" })
      );
      std::filesystem::create_directories
      (
        ConcatPaths({ fullPath, "Resources", "Prefabs" })
      );
      std::filesystem::create_directories
      (
        ConcatPaths({ fullPath, "Resources", "Shaders" })
      );
      std::filesystem::create_directories
      (
        ConcatPaths({ fullPath, "Resources", "Sprites" })
      );
      std::filesystem::create_directories
      (
        ConcatPaths({ fullPath, "Resources", "Textures" })
      );

      // Create project files.
      String codePath = ConcatPaths({ fullPath, "Codes" });
      std::filesystem::create_directories(codePath);

      constexpr int count = 4;
      String source[count] =
      {
        "../Template/Game.h",
        "../Template/Game.cpp",
        "../Template/CMakeLists.txt",
        "../Template/CMakeHotReload.cmake"
      };

      for (int i = 0; i < count; i++)
      {
        std::filesystem::copy
        (
          source[i], codePath,
          std::filesystem::copy_options::overwrite_existing
        );
      }

      // Update cmake.
      String currentPath =
        std::filesystem::current_path().parent_path().u8string();
      UnixifyPath(currentPath);

      std::fstream cmakelist;
      cmakelist.open
      (
        ConcatPaths({ codePath, "CMakeLists.txt" }), std::ios::in
      );
      if (cmakelist.is_open())
      {
        std::stringstream buffer;
        buffer << cmakelist.rdbuf();
        String content = buffer.str();
        ReplaceFirstStringInPlace(content, "__projectname__", name);
        ReplaceFirstStringInPlace(content, "__tkdir__", currentPath);
        cmakelist.close();

        // Override the content.
        cmakelist.open(ConcatPaths
        (
          { codePath, "CMakeLists.txt" }),
          std::ios::out | std::ios::trunc
        );
        if (cmakelist.is_open())
        {
          cmakelist << content;
          cmakelist.close();
        }
      }

      OpenProject({ name, "" });
    }

    void App::SetGameMod(GameMod mod)
    {
      if (mod == m_gameMod)
      {
        return;
      }

      if (mod == GameMod::Playing)
      {
        if (m_gameMod == GameMod::Stop)
        {
          // Save to catch any changes in the editor.
          GetCurrentScene()->Save(true);
        }

        String pluginPath = m_workspace.GetPluginPath();
        if (GetPluginManager()->Load(pluginPath))
        {
          m_statusMsg = "Game is playing";
          m_gameMod = mod;

          if (m_emulatorSettings.runWindowed)
          {
            m_playWindow->SetVisibility(true);
          }
        }
        else
        {
          GetConsole()->AddLog
          (
            "Expecting a game plugin with the same name of the project.",
            LogType::Error
          );
        }
      }

      if (mod == GameMod::Paused)
      {
        m_statusMsg = "Game is paused";
        m_gameMod = mod;
      }

      if (mod == GameMod::Stop)
      {
        GetPluginManager()->UnloadGamePlugin();
        m_statusMsg = "Game is stopped";
        m_gameMod = mod;

        // Set the editor scene back.
        GetCurrentScene()->Reload();
        GetCurrentScene()->Init();
        m_playWindow->SetVisibility(false);
      }
    }

    EditorScenePtr App::GetCurrentScene()
    {
      EditorScenePtr eScn = std::static_pointer_cast<EditorScene>
      (
        GetSceneManager()->GetCurrentScene()
      );

      return eScn;
    }

    void App::SetCurrentScene(const EditorScenePtr& scene)
    {
      GetSceneManager()->SetCurrentScene(scene);
    }

    void App::ResetUI()
    {
      DeleteWindows();
      if (CheckFile(ConcatPaths({ DefaultPath(), "Editor.settings" })))
      {
        // Try reading defaults.
        String settingsFile = ConcatPaths
        (
          { DefaultPath(), "Editor.settings" }
        );

        std::shared_ptr<XmlFile> lclFile = std::make_shared<XmlFile>
        (
          settingsFile.c_str()
        );

        XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
        lclDoc->parse<0>(lclFile->data());

        // Prevent loading last scene.
        Project pj = m_workspace.GetActiveProject();
        m_workspace.SetScene("");

        DeSerialize(lclDoc.get(), nullptr);
        m_workspace.SetScene(pj.scene);

        settingsFile = ConcatPaths({ DefaultPath(), "defaultUI.ini" });
        ImGui::LoadIniSettingsFromDisk(settingsFile.c_str());
      }
      else
      {
        // 3d viewport.
        EditorViewport* vp = new EditorViewport
        (
          m_renderer->m_windowWidth * 0.8f,
          m_renderer->m_windowHeight * 0.8f
        );
        vp->m_name = g_3dViewport;
        vp->GetCamera()->m_node->SetTranslation({ 5.0f, 3.0f, 5.0f });
        vp->GetCamera()->GetComponent<DirectionComponent>()->LookAt
        (
          Vec3(0.0f)
        );
        m_windows.push_back(vp);

        // 2d viewport.
        vp = new EditorViewport2d
        (
          m_renderer->m_windowWidth * 0.8f,
          m_renderer->m_windowHeight * 0.8f
        );
        vp->m_name = g_2dViewport;
        vp->GetCamera()->m_node->SetTranslation(Z_AXIS);
        m_windows.push_back(vp);

        // Isometric viewport.
        vp = new EditorViewport
        (
          m_renderer->m_windowWidth * 0.8f,
          m_renderer->m_windowHeight * 0.8f
        );
        vp->m_name = g_IsoViewport;
        vp->GetCamera()->m_node->SetTranslation({ 0.0f, 10.0f, 0.0f });
        vp->GetCamera()->SetLens(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 1000.0f);
        vp->m_zoom = 0.02f;
        vp->GetCamera()->GetComponent<DirectionComponent>()->Pitch
        (
          glm::radians(-90.0f)
        );
        vp->m_cameraAlignment = CameraAlignment::Top;
        vp->m_orbitLock = true;
        m_windows.push_back(vp);

        ConsoleWindow* console = new ConsoleWindow();
        m_windows.push_back(console);

        FolderWindow* assetBrowser = new FolderWindow();
        assetBrowser->m_name = g_assetBrowserStr;
        assetBrowser->Iterate(ResourcePath(), true);
        m_windows.push_back(assetBrowser);

        OutlinerWindow* outliner = new OutlinerWindow();
        outliner->m_name = g_outlinerStr;
        m_windows.push_back(outliner);

        PropInspector* inspector = new PropInspector();
        inspector->m_name = g_propInspector;
        m_windows.push_back(inspector);

        MaterialInspector* matInspect = new MaterialInspector();
        matInspect->m_name = g_matInspector;
        m_windows.push_back(matInspect);

        PluginWindow* plugWindow = new PluginWindow();
        m_windows.push_back(plugWindow);

        CreateSimulationWindow
        (
          g_app->m_emulatorSettings.playWidth,
          g_app->m_emulatorSettings.playHeight
        );
      }
    }

    void App::DeleteWindows()
    {
      for (Window* wnd : m_windows)
      {
        SafeDel(wnd);
      }
      m_windows.clear();

      for (size_t i = 0; i < EditorViewport::m_overlays.size(); i++)
      {
        SafeDel(EditorViewport::m_overlays[i]);
      }

      SafeDel(m_playWindow);
    }

    void App::CreateWindows(XmlNode* parent)
    {
      if (XmlNode* wndNode = parent->first_node("Window"))
      {
        do
        {
          int type;
          ReadAttr(wndNode, "type", type);

          Window* wnd = nullptr;
          switch ((Window::Type)type)
          {
            case Window::Type::Viewport:
              wnd = new EditorViewport(wndNode);
            break;
            case Window::Type::Console:
              wnd = new ConsoleWindow(wndNode);
            break;
            case Window::Type::Outliner:
              wnd = new OutlinerWindow(wndNode);
            break;
            case Window::Type::Browser:
              wnd = new FolderWindow(wndNode);
            break;
            case Window::Type::Inspector:
              wnd = new PropInspector(wndNode);
            break;
            case Window::Type::MaterialInspector:
              wnd = new MaterialInspector(wndNode);
            break;
            case Window::Type::PluginWindow:
              wnd = new PluginWindow(wndNode);
            break;
            case Window::Type::Viewport2d:
              wnd = new EditorViewport2d(wndNode);
            break;
            default:
              assert(false);
            break;
          }

          if (wnd)
          {
            m_windows.push_back(wnd);
          }
        } while ((wndNode = wndNode->next_sibling("Window")));
      }

      CreateSimulationWindow
      (
          g_app->m_emulatorSettings.playWidth,
          g_app->m_emulatorSettings.playHeight
      );
    }

    int App::Import
    (
      const String& fullPath,
      const String& subDir,
      bool overwrite
    )
    {
      bool doSearch = !UI::SearchFileData.missingFiles.empty();
      if (!CanImport(fullPath) && !doSearch)
      {
        if (ConsoleWindow* con = GetConsole())
        {
          con->AddLog
          (
            "Import failed: " + fullPath,
            LogType::Error
          );
          con->AddLog
          (
            "File format is not supported.\n"
            "Suported formats are fbx, glb, gltf, obj.",
            LogType::Error
          );
        }
        return -1;
      }

      bool importFileExist = CheckFile(fullPath);

      // Set the execute path.
      std::filesystem::path pathBck = std::filesystem::current_path();
      std::filesystem::path path = pathBck.u8string() + ConcatPaths
      (
        { ".", "..", "Utils", "Import" }
      );
      std::filesystem::current_path(path);

      std::filesystem::path cpyDir = ".";
      if (!subDir.empty())
      {
        cpyDir += GetPathSeparator() + subDir;
      }

      // Try reimport after search paths provided.
      bool reImport = doSearch || UI::SearchFileData.showSearchFileWindow;
      if (importFileExist || reImport)
      {
        int result = -1;  // execution result.
        if (!doSearch)
        {
          String name, ext;
          DecomposePath(fullPath, nullptr, &name, &ext);
          String finalPath = fullPath;

          if (name == "importList" && ext == ".txt")
          {
            finalPath = "importList.txt";
          }

          String cmd = "Import \"";
          if (!subDir.empty())
          {
            cmd += finalPath + "\" -t \"" + subDir;
          }
          else
          {
            cmd += finalPath;
          }

          cmd += "\" -s " + std::to_string(UI::ImportData.scale);

          // Execute command
          result = std::system(cmd.c_str());
          assert(result != -1);
        }

        // Move assets.
        String meshFile;
        if (result != -1 || doSearch)
        {
          std::ifstream copyList("out.txt");
          if (copyList.is_open())
          {
            // Check files.
            StringArray missingFiles;
            for (String line; std::getline(copyList, line); )
            {
              if (!CheckFile(line))
              {
                missingFiles.push_back(line);
              }
            }

            if (!missingFiles.empty())
            {
              if (g_app->m_importSlient)
              {
                g_app->GetConsole()->AddLog
                (
                  "Import: " + fullPath + " failed.",
                  LogType::Error
                );
                goto Fail;
              }

              // Try search.
              size_t numFound = 0;
              for (String& searchPath : UI::SearchFileData.searchPaths)
              {
                for (String& missingFile : missingFiles)
                {
                  String name, ext;
                  DecomposePath(missingFile, nullptr, &name, &ext);
                  String missingFullPath = ConcatPaths
                  (
                    { searchPath, name + ext }
                  );
                  if (CheckFile(missingFullPath))
                  {
                    numFound++;
                    std::filesystem::copy
                    (
                      missingFullPath, cpyDir,
                      std::filesystem::copy_options::overwrite_existing
                    );
                  }
                }
              }

              if (numFound < missingFiles.size())
              {
                // Retry.
                UI::SearchFileData.missingFiles = missingFiles;
                goto Retry;
              }
              else
              {
                UI::SearchFileData.missingFiles.clear();
              }
            }

            copyList.clear();
            copyList.seekg(0, std::ios::beg);
            for (String line; std::getline(copyList, line); )
            {
              String ext;
              DecomposePath(line, nullptr, nullptr, &ext);
              const String selfDir = '.' + GetPathSeparatorAsStr();
              if (line.rfind(selfDir) == 0)
              {
                line = line.substr(2, -1);
              }

              String fullPath;
              if (ext == SCENE)
              {
                fullPath = ScenePath(line);
              }

              if (ext == MESH || ext == SKINMESH)
              {
                fullPath = MeshPath(line);
                meshFile = fullPath;
              }

              if (ext == SKELETON)
              {
                fullPath = SkeletonPath(line);
              }

              if (ext == ANIM)
              {
                fullPath = AnimationPath(line);
              }

              if
              (
                ext == PNG ||
                ext == JPG ||
                ext == JPEG ||
                ext == TGA ||
                ext == BMP ||
                ext == PSD
              )
              {
                fullPath = TexturePath(line);
              }

              if (ext == MATERIAL)
              {
                fullPath = MaterialPath(line);
              }

              String path, name;
              DecomposePath(fullPath, &path, &name, &ext);
              std::filesystem::create_directories(path);
              std::filesystem::copy
              (
                line, fullPath,
                std::filesystem::copy_options::overwrite_existing
              );
            }
          }
        }
        else
        {
          goto Fail;
        }

        std::filesystem::current_path(pathBck);
        if (!meshFile.empty())
        {
          String ext;
          DecomposePath(meshFile, nullptr, nullptr, &ext);
          MeshPtr mesh;
          if (ext == SKINMESH)
          {
            mesh = GetMeshManager()->Create<SkinMesh>(meshFile);
          }
          else
          {
            mesh = GetMeshManager()->Create<Mesh>(meshFile);
          }

          if (FolderWindow* browser = GetAssetBrowser())
          {
            browser->UpdateContent();
          }
        }

        UI::SearchFileData.showSearchFileWindow = false;
        return 0;
      }
      else
      {
        goto Fail;
      }

Retry:
      UI::SearchFileData.showSearchFileWindow = true;

Fail:
      std::filesystem::current_path(pathBck);
      return -1;
    }

    bool App::CanImport(const String& fullPath)
    {
      String ext;
      DecomposePath(fullPath, nullptr, nullptr, &ext);

      if (SupportedMeshFormat(ext))
      {
        return true;
      }

      if (ext == ".txt")
      {
        // Hopefully, list of valid objects. Not a poem.
        return true;
      }

      return false;
    }

    void App::OpenScene(const String& fullPath)
    {
      GetCurrentScene()->Destroy(false);
      GetSceneManager()->Remove(GetCurrentScene()->GetFile());
      EditorScenePtr scene = GetSceneManager()->Create<EditorScene>(fullPath);
      SetCurrentScene(scene);
      scene->Init();

      m_workspace.SetScene(scene->m_name);
    }

    void App::MergeScene(const String& fullPath)
    {
      ScenePtr scene = GetSceneManager()->Create<EditorScene>(fullPath);
      scene->Load();
      scene->Init(false);
      GetCurrentScene()->Merge(scene);
    }

    void App::ApplyProjectSettings(bool setDefaults)
    {
      if
      (
        CheckFile
        (
          ConcatPaths({ ResourcePath(), "Editor.settings" })
        )
        && !setDefaults
      )
      {
        DeSerialize(nullptr, nullptr);
        UI::InitSettings();
      }
      else
      {
        ResetUI();
      }

      // Restore app window.
      SDL_SetWindowSize
      (
        g_window,
        m_renderer->m_windowWidth,
        m_renderer->m_windowHeight
      );

      SDL_SetWindowPosition
      (
        g_window,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED
      );

      if (m_windowMaximized)
      {
        SDL_MaximizeWindow(g_window);
      }
    }

    void App::OpenProject(const Project& project)
    {
      UI::m_postponedActions.push_back
      (
        [this, project]() -> void
        {
          m_workspace.SetActiveProject(project);
          m_workspace.Serialize(nullptr, nullptr);
          OnNewScene("New Scene");
        }
      );
    }

    void App::PackResources()
    {
      String projectName = m_workspace.GetActiveProject().name;
      if (projectName.empty())
      {
        GetLogger()->WriteConsole(LogType::Error, "No project is loaded!");
        return;
      }

      String path = ConcatPaths
      (
        {
          m_workspace.GetActiveWorkspace(),
          projectName,
          "Resources",
          "Scenes"
        }
      );

      GetFileManager()->PackResources(path);
    }

    Window* App::GetActiveWindow()
    {
      for (Window* wnd : m_windows)
      {
        if (wnd->IsActive() && wnd->IsVisible())
        {
          return wnd;
        }
      }

      return nullptr;
    }

    EditorViewport* App::GetActiveViewport()
    {
      for (Window* wnd : m_windows)
      {
        if
        (
          wnd->GetType() != Window::Type::Viewport &&
          wnd->GetType() != Window::Type::Viewport2d
        )
        {
          continue;
        }

        if (wnd->IsActive() && wnd->IsVisible())
        {
          return static_cast<EditorViewport*> (wnd);
        }
      }

      return nullptr;
    }

    EditorViewport* App::GetViewport(const String& name)
    {
      for (Window* wnd : m_windows)
      {
        if (wnd->m_name == name)
        {
          return dynamic_cast<EditorViewport*> (wnd);
        }
      }

      return nullptr;
    }

    ConsoleWindow* App::GetConsole()
    {
      for (Window* wnd : m_windows)
      {
        if (wnd->GetType() == Window::Type::Console)
        {
          return static_cast<ConsoleWindow*> (wnd);
        }
      }

      return nullptr;
    }

    FolderWindow* App::GetAssetBrowser()
    {
      return GetWindow<FolderWindow>(g_assetBrowserStr);
    }

    OutlinerWindow* App::GetOutliner()
    {
      return GetWindow<OutlinerWindow>(g_outlinerStr);
    }

    PropInspector* App::GetPropInspector()
    {
      return GetWindow<PropInspector>(g_propInspector);
    }

    MaterialInspector* App::GetMaterialInspector()
    {
      return GetWindow<MaterialInspector>(g_matInspector);
    }

    void App::RenderSelected
    (
      EditorViewport* viewport,
      EntityRawPtrArray selecteds
    )
    {
      if (GetCurrentScene()->GetSelectedEntityCount() == 0)
      {
        return;
      }

      auto RenderFn = [this, viewport]
      (const EntityRawPtrArray& selection, const Vec4& color) -> void
      {
        if (selection.empty())
        {
          return;
        }

        RenderTargetSettigs rtSet;
        rtSet.WarpS = rtSet.WarpT = GraphicTypes::UVClampToEdge;
        RenderTarget stencilMask
        (
          static_cast<int>(viewport->m_width),
          static_cast<int>(viewport->m_height),
          rtSet
        );
        stencilMask.Init();

        m_renderer->SetRenderTarget
        (
          &stencilMask,
          true,
          { 0.0f, 0.0f, 0.0f, 1.0 }
        );

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // webgl create problem with depth only drawing with textures.
        static MaterialPtr solidMat =
        GetMaterialManager()->GetCopyOfSolidMaterial();
        solidMat->GetRenderState()->cullMode = CullingType::TwoSided;
        m_renderer->m_overrideMat = solidMat;

        bool isLight = false;
        for (Entity* ntt : selection)
        {
          // Add billboard
          Entity* bb = GetCurrentScene()->GetBillboardOfEntity(ntt);
          if (bb != nullptr)
          {
            static_cast<Billboard*>(bb)->LookAt
            (
              viewport->GetCamera(),
              viewport->m_zoom
            );
            m_renderer->Render(bb, viewport->GetCamera());
          }

          if (ntt->IsDrawable())
          {
            isLight = false;
            if (ntt->GetType() == EntityType::Entity_Light)
            {
              isLight = true;
            }

            // Disable all gizmos
            if (isLight)
            {
              Light* light = static_cast<Light*>(ntt);
              EnableLightGizmo(light, false);
              m_renderer->Render(ntt, viewport->GetCamera());
              EnableLightGizmo(light, true);
            }
            else
            {
              m_renderer->Render(ntt, viewport->GetCamera());
            }
          }
        }

        m_renderer->m_overrideMat = nullptr;

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glStencilFunc(GL_NOTEQUAL, 0xFF, 0xFF);
        glStencilMask(0x00);
        ShaderPtr solidColor = GetShaderManager()->Create<Shader>
        (
          ShaderPath("unlitColorFrag.shader", true)
        );
        m_renderer->DrawFullQuad(solidColor);
        glDisable(GL_STENCIL_TEST);

        m_renderer->SetRenderTarget(viewport->m_viewportImage, false);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Dilate.
        glBindTexture(GL_TEXTURE_2D, stencilMask.m_textureId);
        ShaderPtr dilate = GetShaderManager()->Create<Shader>
        (
          ShaderPath("dilateFrag.shader", true)
        );
        dilate->SetShaderParameter("Color", ParameterVariant(color));
        m_renderer->DrawFullQuad(dilate);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      };

      Entity* primary = selecteds.back();

      selecteds.pop_back();
      RenderFn(selecteds, g_selectHighLightSecondaryColor);

      selecteds.clear();
      selecteds.push_back(primary);
      RenderFn(selecteds, g_selectHighLightPrimaryColor);

      if (m_showSelectionBoundary && primary->IsDrawable())
      {
        m_perFrameDebugObjects.push_back
        (
          CreateBoundingBoxDebugObject(primary->GetAABB(true))
        );
      }
    }

    void App::RenderGizmo
    (
      EditorViewport* viewport,
      Gizmo* gizmo
    )
    {
      if (gizmo == nullptr)
      {
        return;
      }

      gizmo->LookAt(viewport->GetCamera(), viewport->m_zoom);

      glClear(GL_DEPTH_BUFFER_BIT);
      if (PolarGizmo* pg = dynamic_cast<PolarGizmo*> (gizmo))
      {
        pg->Render(m_renderer, viewport->GetCamera());
      }
      else
      {
        Entity* nttGizmo = dynamic_cast<Entity*>(gizmo);
        if (nttGizmo != nullptr)
        {
          m_renderer->Render(gizmo, viewport->GetCamera());
        }
      }
    }

    void App::RenderComponentGizmo
    (
      EditorViewport* viewport,
      EntityRawPtrArray selecteds
    )
    {
      // Entity billboards
      for (Billboard* bb : GetCurrentScene()->GetBillboards())
      {
        m_renderer->Render(bb, viewport->GetCamera());
      }

      // Selected gizmos
      for (Entity* ntt : selecteds)
      {
        // Environment Component
        EnvironmentComponentPtr envCom =
        ntt->GetComponent<EnvironmentComponent>();
        if (envCom != nullptr && ntt->GetType() != EntityType::Entity_Sky)
        {
          // Bounding box
          m_perFrameDebugObjects.push_back
          (
            CreateBoundingBoxDebugObject
            (
              *envCom->GetBBox(),
              g_environmentGizmoColor,
              1.0f
          )
          );
        }
      }
    }

    void App::ShowPlayWindow(float deltaTime)
    {
      if (GamePlugin* plugin = GetPluginManager()->GetGamePlugin())
      {
        if (plugin->m_quit)
        {
          SetGameMod(GameMod::Stop);
        }

        if (m_gameMod != GameMod::Stop)
        {
          m_playWindow->SetVisibility(m_emulatorSettings.runWindowed);

          EditorViewport* playWindow = GetWindow<EditorViewport>(g_3dViewport);
          if (m_emulatorSettings.runWindowed)
          {
            if (m_windowCamLoad)
            {
              Mat4 camTs = playWindow->GetCamera()->m_node->GetTransform
              (
                TransformationSpace::TS_WORLD
              );
              m_playWindow->GetCamera()->m_node->SetTransform(camTs);
              m_windowCamLoad = false;
            }
            playWindow = m_playWindow;
          }
          m_renderer->SwapRenderTarget(&playWindow->m_viewportImage);
          plugin->Frame(deltaTime, playWindow);
          m_renderer->RenderUI(GetUIManager()->GetCurrentLayers(), playWindow);
          m_renderer->SwapRenderTarget(&playWindow->m_viewportImage);
        }
      }
    }

    void App::Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      m_workspace.Serialize(nullptr, nullptr);

      std::ofstream file;
      String fileName = ConcatPaths({ ResourcePath(), "Editor.settings" });

      file.open(fileName.c_str(), std::ios::out);
      if (file.is_open())
      {
        XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
        XmlNode* app = lclDoc->allocate_node(rapidxml::node_element, "App");
        lclDoc->append_node(app);

        XmlNode* settings = lclDoc->allocate_node
        (
          rapidxml::node_element,
          "Settings"
        );
        app->append_node(settings);

        XmlNode* setNode = lclDoc->allocate_node
        (
          rapidxml::node_element,
          "Size"
        );
        WriteAttr
        (
          setNode,
          lclDoc.get(),
          "width",
          std::to_string(m_renderer->m_windowWidth)
        );
        WriteAttr
        (
          setNode,
          lclDoc.get(),
          "height",
          std::to_string(m_renderer->m_windowHeight)
        );
        WriteAttr
        (
          setNode,
          lclDoc.get(),
          "maximized",
          std::to_string(m_windowMaximized)
        );
        settings->append_node(setNode);

        for (Window* w : m_windows)
        {
          w->Serialize(lclDoc.get(), app);
        }

        std::string xml;
        rapidxml::print(std::back_inserter(xml), *lclDoc, 0);

        file << xml;
        file.close();
        lclDoc->clear();
      }
    }

    void App::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      XmlFilePtr lclFile = nullptr;
      XmlDocumentPtr lclDoc = nullptr;

      if (doc == nullptr)
      {
        String settingsFile = ConcatPaths
        (
          { ResourcePath(), "Editor.settings" }
        );
        lclFile = std::make_shared<XmlFile>(settingsFile.c_str());
        lclDoc = std::make_shared<XmlDocument>();
        lclDoc->parse<0>(lclFile->data());
        doc = lclDoc.get();
      }

      if (XmlNode* root = doc->first_node("App"))
      {
        if (XmlNode* settings = root->first_node("Settings"))
        {
          if (XmlNode* setNode = settings->first_node("Size"))
          {
            uint width = 0;
            ReadAttr(setNode, "width", width);
            uint height = 0;
            ReadAttr(setNode, "height", height);
            ReadAttr(setNode, "maximized", m_windowMaximized);

            if (width > 0 && height > 0)
            {
              OnResize(width, height);
            }
          }
        }

        CreateWindows(root);
      }

      String scene = m_workspace.GetActiveProject().scene;
      if (!scene.empty())
      {
        String fullPath = ScenePath(scene);
        OpenScene(fullPath);
      }
    }

    void App::CreateSimulationWindow(float width, float height)
    {
      m_playWindow = new EditorViewport
      (
        m_emulatorSettings.playWidth,
        m_emulatorSettings.playHeight
      );
      m_playWindow->m_name = g_simulationViewport;
      m_playWindow->m_additionalWindowFlags =
        ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoDocking
        |ImGuiWindowFlags_NoCollapse;
      m_playWindow->SetVisibility(false);
    }

    void App::AssignManagerReporters()
    {
      // Register manager reporters
      auto genericReporterFn = [](LogType logType, String msg) -> void
      {
        if (ConsoleWindow* console = g_app->GetConsole())
        {
          console->AddLog(msg, logType);
        }
      };
      GetLogger()->SetWriteConsoleFn(genericReporterFn);
    }

    void App::CreateAndSetNewScene(const String& name)
    {
      EditorScenePtr scene = std::make_shared<EditorScene>
      (
        ScenePath(name + SCENE)
      );

      scene->m_name = name;
      scene->m_newScene = true;
      GetSceneManager()->Manage(scene);
      SetCurrentScene(scene);
    }

    void DebugMessage(const String& msg)
    {
      g_app->GetConsole()->AddLog(msg, "Debug");
    }

    void DebugMessage(const Vec3& vec)
    {
      g_app->GetConsole()->AddLog(glm::to_string(vec), "Debug");
    }

    void DebugMessage(const char* msg, ...)
    {
      va_list args;
      va_start(args, msg);

      static char buff[2048];
      vsprintf(buff, msg, args);
      DebugMessage(String(buff));

      va_end(args);
    }

    void DebugCube(const Vec3& p, float size)
    {
      g_app->m_perFrameDebugObjects.push_back
      (
        CreateBoundingBoxDebugObject
        (
          {
            p - Vec3(size),
            p + Vec3(size)
          }
        )
      );
    }

    void DebugLineStrip(const Vec3Array& pnts)
    {
      g_app->m_perFrameDebugObjects.push_back
      (
        CreateLineDebugObject(pnts)
      );
    }

  }  // namespace Editor
}  // namespace ToolKit
