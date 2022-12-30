#include "App.h"

#include "Action.h"
#include "Anchor.h"
#include "Camera.h"
#include "ConsoleWindow.h"
#include "DirectionComponent.h"
#include "EditorCamera.h"
#include "EditorPass.h"
#include "EditorViewport.h"
#include "EditorViewport2d.h"
#include "FolderWindow.h"
#include "Framebuffer.h"
#include "GL/glew.h"
#include "Gizmo.h"
#include "Global.h"
#include "Grid.h"
#include "Mod.h"
#include "Node.h"
#include "OutlinerWindow.h"
#include "OverlayUI.h"
#include "Pass.h"
#include "PluginWindow.h"
#include "PopupWindows.h"
#include "Primative.h"
#include "PropInspector.h"
#include "Renderer.h"
#include "UI.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "DebugNew.h"

namespace ToolKit
{
  OutlinePass* myOutlineTechnique          = nullptr;
  Editor::EditorRenderer* myEditorRenderer = nullptr;

  namespace Editor
  {
    App::App(int windowWidth, int windowHeight) : m_workspace(this)
    {
      m_cursor                   = nullptr;
      m_renderer                 = Main::GetInstance()->m_renderer;
      m_renderer->m_windowSize.x = windowWidth;
      m_renderer->m_windowSize.y = windowHeight;
      m_statusMsg                = "OK";

      myEditorRenderer           = new EditorRenderer();

      OverrideEntityConstructors();

      lightModeMat = std::make_shared<Material>();
    }

    App::~App()
    {
      Destroy();
      SafeDel(myEditorRenderer);
      SafeDel(myOutlineTechnique);
    }

    void App::Init()
    {
      AssignManagerReporters();
      CreateEditorEntities();

      ModManager::GetInstance()->Init();
      ModManager::GetInstance()->SetMod(true, ModId::Select);
      ActionManager::GetInstance()->Init();

      m_workspace.Init();
      String sceneName = "New Scene" + SCENE;
      if (!m_newSceneName.empty())
      {
        sceneName = m_newSceneName;
      }

      EditorScenePtr scene =
          std::make_shared<EditorScene>(ScenePath(sceneName));

      scene->m_name     = sceneName;
      scene->m_newScene = true;
      SetCurrentScene(scene);
      ApplyProjectSettings(false);

      if (!CheckFile(m_workspace.GetActiveWorkspace()))
      {
        StringInputWindow* wsDir =
            new StringInputWindow("Set Workspace Directory##SetWsdir", false);
        wsDir->m_hint       = "User/Documents/ToolKit";
        wsDir->m_inputLabel = "Workspace Directory";
        wsDir->m_name       = "Set Workspace Directory";
        wsDir->m_taskFn     = [](const String& val) -> void
        {
          String cmd = "SetWorkspaceDir --path \"" + val + "\"";
          g_app->GetConsole()->ExecCommand(cmd);
        };
      }
      else
      {
        m_workspace.RefreshProjects();
      }

      m_simulatorSettings.Resolution = EmulatorResolution::Custom;
      m_publishManager               = new PublishManager();
      GetRenderer()->m_clearColor    = g_wndBgColor;
    }

    void App::DestroyEditorEntities()
    {
      SafeDel(m_publishManager);

      // Editor objects.
      SafeDel(m_2dGrid);
      SafeDel(m_grid);
      SafeDel(m_origin);
      SafeDel(m_cursor);
      if (m_dbgArrow)
      {
        GetCurrentScene()->RemoveEntity(m_dbgArrow->GetIdVal());
        m_dbgArrow = nullptr;
      }
      if (m_dbgFrustum)
      {
        GetCurrentScene()->RemoveEntity(m_dbgFrustum->GetIdVal());
        m_dbgFrustum = nullptr;
      }

      for (Entity* dbgObj : m_perFrameDebugObjects)
      {
        SafeDel(dbgObj);
      }
      m_perFrameDebugObjects.clear();
    }

    void App::Destroy()
    {
      // UI.
      DeleteWindows();

      DestroyEditorEntities();

      GetCurrentScene()->Destroy(false);

      GetAnimationPlayer()->m_records.clear();

      GetUIManager()->DestroyLayers();

      ModManager::GetInstance()->UnInit();
      ActionManager::GetInstance()->UnInit();
    }

    void App::Frame(float deltaTime)
    {
      m_deltaTime = deltaTime;
      UI::BeginUI();
      UI::ShowUI();

      // Update Mods.
      ModManager::GetInstance()->Update(deltaTime);
      std::vector<EditorViewport*> viewports;
      for (Window* wnd : m_windows)
      {
        if (wnd->IsViewport())
        {
          viewports.push_back((EditorViewport*) wnd);
        }
        wnd->DispatchSignals();
      }

      EditorScenePtr scene = GetCurrentScene();
      scene->Update(deltaTime);
      ShowSimulationWindow(deltaTime);

      // Render Viewports.
      for (EditorViewport* viewport : viewports)
      {
        viewport->Update(deltaTime);

        // PlayWindow is drawn on perspective. Thus, skip perspective.
        if (m_gameMod != GameMod::Stop && !m_simulatorSettings.Windowed)
        {
          if (viewport->m_name == g_3dViewport)
          {
            continue;
          }
        }

        if (viewport->IsVisible())
        {
          myEditorRenderer->m_params.App      = this;
          myEditorRenderer->m_params.LitMode  = m_sceneLightingMode;
          myEditorRenderer->m_params.Viewport = viewport;
          myEditorRenderer->m_params.tonemapping =
              Main::GetInstance()->m_engineSettings.Graphics.TonemapperMode;
          myEditorRenderer->Render();
        }
      }

      // Render UI.
      UI::EndUI();

      m_renderer->m_totalFrameCount++;
    }

    void App::OnResize(uint width, uint height)
    {
      m_renderer->m_windowSize.x = width;
      m_renderer->m_windowSize.y = height;
    }

    void App::OnNewScene(const String& name)
    {
      Destroy();
      m_newSceneName = name;
      Init();
      m_newSceneName.clear();
      m_workspace.SetScene(name);
    }

    void App::OnSaveScene()
    {
      // Prevent overriding default scene.
      EditorScenePtr currScene = GetCurrentScene();
      if (GetSceneManager()->GetDefaultResource(ResourceType::Scene) ==
          currScene->GetFile())
      {
        currScene->SetFile(ScenePath("New Scene" + SCENE));
        return OnSaveAsScene();
      }

      auto saveFn = []() -> void
      {
        g_app->GetCurrentScene()->Save(false);
        g_app->m_statusMsg                    = "Scene saved";
        FolderWindowRawPtrArray folderWindows = g_app->GetAssetBrowsers();
        for (FolderWindow* folderWnd : folderWindows)
        {
          folderWnd->UpdateContent();
        }
      };

      // File existance check.
      String fullPath = currScene->GetFile();
      if (currScene->m_newScene && CheckFile(fullPath))
      {
        String msg = "Scene " + fullPath +
                     " exist on the disk.\nOverride the existing scene ?";
        YesNoWindow* overrideScene =
            new YesNoWindow("Override existing file##OvrdScn", msg);
        overrideScene->m_yesCallback = [&saveFn]() { saveFn(); };

        overrideScene->m_noCallback  = []()
        {
          g_app->GetConsole()->AddLog(
              "Scene has not been saved.\n"
              "A scene with the same name exist. Use File->SaveAs.",
              LogType::Error);
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
      StringInputWindow* inputWnd =
          new StringInputWindow("SaveScene##SvScn1", true);
      inputWnd->m_inputLabel = "Name";
      inputWnd->m_hint       = "Scene name";
      inputWnd->m_taskFn     = [](const String& val)
      {
        String path;
        EditorScenePtr currScene = g_app->GetCurrentScene();
        DecomposePath(currScene->GetFile(), &path, nullptr, nullptr);
        String fullPath = ConcatPaths({path, val + SCENE});
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
        YesNoWindow* reallyQuit =
            new YesNoWindow("Quiting... Are you sure?##ClsApp");

        reallyQuit->m_yesCallback = [this]()
        {
          m_workspace.Serialize(nullptr, nullptr);
          Serialize(nullptr, nullptr);
          g_running = false;
        };

        reallyQuit->m_noCallback = [this]() { m_onQuit = false; };

        UI::m_volatileWindows.push_back(reallyQuit);
        m_onQuit = true;
      }
    }

    void App::OnNewProject(const String& name)
    {
      if (m_workspace.GetActiveWorkspace().empty())
      {
        GetConsole()->AddLog("No workspace. Project can't be created.",
                             LogType::Error);
        return;
      }

      String fullPath = ConcatPaths({m_workspace.GetActiveWorkspace(), name});
      if (CheckFile(fullPath))
      {
        GetConsole()->AddLog("Project already exist.", LogType::Error);
        return;
      }

      std::filesystem::create_directories(
          ConcatPaths({fullPath, "Resources", "Audio"}));
      std::filesystem::create_directories(
          ConcatPaths({fullPath, "Resources", "Fonts"}));
      std::filesystem::create_directories(
          ConcatPaths({fullPath, "Resources", "Layers"}));
      std::filesystem::create_directories(
          ConcatPaths({fullPath, "Resources", "Materials"}));
      std::filesystem::create_directories(
          ConcatPaths({fullPath, "Resources", "Meshes"}));
      std::filesystem::create_directories(
          ConcatPaths({fullPath, "Resources", "Scenes"}));
      std::filesystem::create_directories(
          ConcatPaths({fullPath, "Resources", "Prefabs"}));
      std::filesystem::create_directories(
          ConcatPaths({fullPath, "Resources", "Shaders"}));
      std::filesystem::create_directories(
          ConcatPaths({fullPath, "Resources", "Sprites"}));
      std::filesystem::create_directories(
          ConcatPaths({fullPath, "Resources", "Textures"}));

      // Create project files.
      String codePath = ConcatPaths({fullPath, "Codes"});
      std::filesystem::create_directories(codePath);

      constexpr int count  = 4;
      String source[count] = {"../Template/Game.h",
                              "../Template/Game.cpp",
                              "../Template/CMakeLists.txt",
                              "../Template/CMakeHotReload.cmake"};

      for (int i = 0; i < count; i++)
      {
        std::filesystem::copy(
            source[i],
            codePath,
            std::filesystem::copy_options::overwrite_existing);
      }

      // Update cmake.
      String currentPath =
          std::filesystem::current_path().parent_path().u8string();
      UnixifyPath(currentPath);

      std::fstream cmakelist;
      cmakelist.open(ConcatPaths({codePath, "CMakeLists.txt"}), std::ios::in);
      if (cmakelist.is_open())
      {
        std::stringstream buffer;
        buffer << cmakelist.rdbuf();
        String content = buffer.str();
        ReplaceFirstStringInPlace(content, "__projectname__", name);
        ReplaceFirstStringInPlace(content, "__tkdir__", currentPath);
        cmakelist.close();

        // Override the content.
        cmakelist.open(ConcatPaths({codePath, "CMakeLists.txt"}),
                       std::ios::out | std::ios::trunc);
        if (cmakelist.is_open())
        {
          cmakelist << content;
          cmakelist.close();
        }
      }

      OpenProject({name, ""});
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
          m_gameMod   = mod;

          if (m_simulatorSettings.Windowed)
          {
            m_simulationWindow->SetVisibility(true);
          }
        }
        else
        {
          GetConsole()->AddLog(
              "Expecting a game plugin with the same name of the project.",
              LogType::Error);
        }
      }

      if (mod == GameMod::Paused)
      {
        m_statusMsg = "Game is paused";
        m_gameMod   = mod;
      }

      if (mod == GameMod::Stop)
      {
        GetPluginManager()->UnloadGamePlugin();
        m_statusMsg = "Game is stopped";
        m_gameMod   = mod;

        // Set the editor scene back.
        GetCurrentScene()->Reload();
        GetCurrentScene()->Init();
        m_simulationWindow->SetVisibility(false);
      }
    }

    void App::CompilePlugin()
    {
      String codePath = m_workspace.GetCodePath();
      String buildDir = ConcatPaths({codePath, "build"});

      // create a build dir if not exist.
      std::filesystem::create_directories(buildDir);

      // Update project files in case of change.
#ifdef TK_DEBUG
      static const StringView buildConfig = "Debug";
#else
      static const StringView buildConfig = "Release";
#endif
      String cmd  = "cmake -S " + codePath + " -B " + buildDir;
      m_statusMsg = "Compiling ..." + g_statusNoTerminate;
      ExecSysCommand(
          cmd,
          true,
          false,
          [this, buildDir](int res) -> void
          {
            String cmd =
                "cmake --build " + buildDir + " --config " + buildConfig.data();
            ExecSysCommand(cmd,
                           false,
                           false,
                           [=](int res) -> void
                           {
                             if (res)
                             {
                               m_statusMsg = "Compile Failed.";

                               String detail;
                               if (res == 1)
                               {
                                 detail = "CMake Build Failed.";
                               }

                               if (res == -1)
                               {
                                 detail = "CMake Generate Failed.";
                               }

                               GetLogger()->WriteConsole(LogType::Error,
                                                         "%s %s",
                                                         m_statusMsg.c_str(),
                                                         detail.c_str());
                             }
                             else
                             {
                               m_statusMsg = "Compiled.";
                               GetLogger()->WriteConsole(LogType::Memo,
                                                         "%s",
                                                         m_statusMsg.c_str());
                             }
                           });
          });
    }

    EditorScenePtr App::GetCurrentScene()
    {
      EditorScenePtr eScn = std::static_pointer_cast<EditorScene>(
          GetSceneManager()->GetCurrentScene());

      return eScn;
    }

    void App::SetCurrentScene(const EditorScenePtr& scene)
    {
      GetSceneManager()->SetCurrentScene(scene);
    }

    void App::FocusEntity(Entity* entity)
    {
      Camera* cam = nullptr;
      if (Viewport* viewport = GetActiveViewport())
      {
        cam = viewport->GetCamera();
      }
      else if (Viewport* viewport = GetViewport(g_3dViewport))
      {
        cam = viewport->GetCamera();
      }
      else
      {
        m_statusMsg = "No 3D viewport !";
        return;
      }

      if (!GetCurrentScene()->GetBillboardOfEntity(entity))
      {
        cam->FocusToBoundingBox(entity->GetAABB(true), 1.1f);
      }
      else
      {
        BoundingBox defaultBBox = {Vec3(-1.0f), Vec3(1.0f)};
        Vec3 pos =
            entity->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        defaultBBox.max += pos;
        defaultBBox.min += pos;
        cam->FocusToBoundingBox(defaultBBox, 1.1f);
      }
    }

    int App::ExecSysCommand(StringView cmd,
                            bool async,
                            bool showConsole,
                            SysCommandDoneCallback callback)
    {
      if (m_sysComExecFn)
      {
        return m_sysComExecFn(cmd, async, showConsole, callback);
      }

      return -1;
    }

    void App::ResetUI()
    {
      DeleteWindows();

      String defEditSet = ConcatPaths({ConfigPath(), g_editorSettingsFile});
      if (CheckFile(defEditSet) && CheckFile(m_workspace.GetActiveWorkspace()))
      {
        // Try reading defaults.
        String settingsFile = defEditSet;

        std::shared_ptr<XmlFile> lclFile =
            std::make_shared<XmlFile>(settingsFile.c_str());

        XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
        lclDoc->parse<0>(lclFile->data());

        // Prevent loading last scene.
        Project pj = m_workspace.GetActiveProject();
        m_workspace.SetScene("");

        DeSerialize(lclDoc.get(), nullptr);
        m_workspace.SetScene(pj.scene);

        settingsFile = ConcatPaths({ConfigPath(), g_uiLayoutFile});
        ImGui::LoadIniSettingsFromDisk(settingsFile.c_str());
      }
      else
      {
        // 3d viewport.
        Vec2 vpSize        = Vec2(m_renderer->m_windowSize) * 0.8f;
        EditorViewport* vp = new EditorViewport(vpSize);
        vp->m_name         = g_3dViewport;
        vp->GetCamera()->m_node->SetTranslation({5.0f, 3.0f, 5.0f});
        vp->GetCamera()->GetComponent<DirectionComponent>()->LookAt(Vec3(0.0f));
        m_windows.push_back(vp);

        // 2d viewport.
        vp         = new EditorViewport2d(vpSize);
        vp->m_name = g_2dViewport;
        vp->GetCamera()->m_node->SetTranslation(Z_AXIS);
        m_windows.push_back(vp);

        // Isometric viewport.
        vp         = new EditorViewport(vpSize);
        vp->m_name = g_IsoViewport;
        vp->GetCamera()->m_node->SetTranslation({0.0f, 10.0f, 0.0f});
        vp->GetCamera()->SetLens(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 1000.0f);
        vp->GetCamera()->m_orthographicScale = 0.02f;
        vp->GetCamera()->GetComponent<DirectionComponent>()->Pitch(
            glm::radians(-90.0f));
        vp->m_cameraAlignment = CameraAlignment::Top;
        vp->m_orbitLock       = true;
        m_windows.push_back(vp);

        ConsoleWindow* console = new ConsoleWindow();
        m_windows.push_back(console);

        FolderWindow* assetBrowser = new FolderWindow(true);
        assetBrowser->m_name       = g_assetBrowserStr;
        m_windows.push_back(assetBrowser);

        OutlinerWindow* outliner = new OutlinerWindow();
        outliner->m_name         = g_outlinerStr;
        m_windows.push_back(outliner);

        PropInspector* inspector = new PropInspector();
        inspector->m_name        = g_propInspector;
        m_windows.push_back(inspector);

        PluginWindow* plugWindow = new PluginWindow();
        m_windows.push_back(plugWindow);

        CreateSimulationWindow(m_simulatorSettings.Width,
                               m_simulatorSettings.Height);
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

      SafeDel(m_simulationWindow);
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
          switch ((Window::Type) type)
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
          case Window::Type::PluginWindow:
            wnd = new PluginWindow(wndNode);
            break;
          case Window::Type::Viewport2d:
            wnd = new EditorViewport2d(wndNode);
            break;
          }

          if (wnd)
          {
            m_windows.push_back(wnd);
          }
        } while ((wndNode = wndNode->next_sibling("Window")));
      }

      CreateSimulationWindow(m_simulatorSettings.Width,
                             m_simulatorSettings.Height);
    }

    int App::Import(const String& fullPath,
                    const String& subDir,
                    bool overwrite)
    {
      bool doSearch = !UI::SearchFileData.missingFiles.empty();
      if (!CanImport(fullPath) && !doSearch)
      {
        if (ConsoleWindow* con = GetConsole())
        {
          con->AddLog("Import failed: " + fullPath, LogType::Error);
          con->AddLog("File format is not supported.\n"
                      "Suported formats are fbx, glb, gltf, obj.",
                      LogType::Error);
        }
        return -1;
      }

      bool importFileExist          = CheckFile(fullPath);

      // Set the execute path.
      std::filesystem::path pathBck = std::filesystem::current_path();
      std::filesystem::path path =
          pathBck.u8string() + ConcatPaths({"", "..", "Utils", "Import"});
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
        int result = -1; // execution result.
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

          cmd    += "\" -s " + std::to_string(UI::ImportData.Scale);

          // Execute command
          result = ExecSysCommand(cmd.c_str(), false, false);
          if (result != 0)
          {
            GetLogger()->WriteConsole(LogType::Error, "Import failed!");
          }
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
            for (String line; std::getline(copyList, line);)
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
                g_app->GetConsole()->AddLog("Import: " + fullPath + " failed.",
                                            LogType::Error);
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
                  String missingFullPath =
                      ConcatPaths({searchPath, name + ext});
                  if (CheckFile(missingFullPath))
                  {
                    numFound++;
                    std::filesystem::copy(
                        missingFullPath,
                        cpyDir,
                        std::filesystem::copy_options::overwrite_existing);
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
            for (String line; std::getline(copyList, line);)
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
                fullPath = PrefabPath(line);
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

              if (SupportedImageFormat(ext))
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
              std::filesystem::copy(
                  line,
                  fullPath,
                  std::filesystem::copy_options::overwrite_existing);
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

          FolderWindowRawPtrArray folderWindows = g_app->GetAssetBrowsers();
          for (FolderWindow* folderWnd : folderWindows)
          {
            folderWnd->UpdateContent();
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

      if (SupportedImageFormat(ext))
      {
        return true;
      }

      return false;
    }

    void App::ManageDropfile(const StringView& fileName)
    {
      UI::m_postponedActions.push_back(
          [fileName]() -> void
          {
            const FolderWindowRawPtrArray& assetBrowsers =
                g_app->GetAssetBrowsers();

            String log = "File isn't imported because it's not dropped onto "
                         "Asset Browser";

            for (FolderWindow* folderWindow : assetBrowsers)
            {
              if (folderWindow->MouseHovers())
              {
                FolderView* activeView = folderWindow->GetActiveView(true);
                if (activeView == nullptr)
                {
                  log = "Activate a resource folder by selecting it from the "
                        "Asset "
                        "Browser.";
                }
                else
                {
                  UI::ImportData.ActiveView = activeView;
                  UI::ImportData.Files.push_back(fileName.data());
                  UI::ImportData.ShowImportWindow = true;
                }
              }
            }

            if (!UI::ImportData.ShowImportWindow)
            {
              g_app->m_statusMsg = "Drop discarded";
              GetLogger()->WriteConsole(LogType::Warning, log.c_str());
            }
          });
    }

    void App::OpenScene(const String& fullPath)
    {
      GetCurrentScene()->Destroy(false);
      GetSceneManager()->Remove(GetCurrentScene()->GetFile());
      EditorScenePtr scene = GetSceneManager()->Create<EditorScene>(fullPath);
      if (IsLayer(fullPath))
      {
        if (EditorViewport2d* viewport =
                GetWindow<EditorViewport2d>(g_2dViewport))
        {
          UILayer* layer = new UILayer(scene);
          GetUIManager()->AddLayer(viewport->m_viewportId, layer);
        }
        else
        {
          g_app->m_statusMsg = "Layer creation failed. No 2d viewport.";
        }
      }

      SetCurrentScene(scene);
      scene->Init();
      m_workspace.SetScene(scene->m_name);
    }

    void App::MergeScene(const String& fullPath)
    {
      EditorScenePtr scene = GetSceneManager()->Create<EditorScene>(fullPath);
      scene->Load();
      scene->Init();
      GetCurrentScene()->Merge(scene);
    }

    void App::LinkScene(const String& fullPath)
    {
      GetSceneManager()->GetCurrentScene()->LinkPrefab(fullPath);
    }

    void App::ApplyProjectSettings(bool setDefaults)
    {
      if (CheckFile(ConcatPaths(
              {m_workspace.GetProjectConfigPath(), g_editorSettingsFile})) &&
          !setDefaults)
      {
        DeSerialize(nullptr, nullptr);
        UI::InitSettings();
      }
      else
      {
        ResetUI();
      }

      // Restore app window.
      SDL_SetWindowSize(g_window,
                        m_renderer->m_windowSize.x,
                        m_renderer->m_windowSize.y);

      SDL_SetWindowPosition(g_window,
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED);

      if (m_windowMaximized)
      {
        SDL_MaximizeWindow(g_window);
      }
    }

    void App::OpenProject(const Project& project)
    {
      UI::m_postponedActions.push_back(
          [this, project]() -> void
          {
            m_workspace.SetActiveProject(project);
            m_workspace.Serialize(nullptr, nullptr);
            OnNewScene("New Scene");
          });
    }

    void App::PackResources()
    {
      String projectName = m_workspace.GetActiveProject().name;
      if (projectName.empty())
      {
        GetLogger()->WriteConsole(LogType::Error, "No project is loaded!");
        return;
      }

      String path = ConcatPaths({m_workspace.GetActiveWorkspace(),
                                 projectName,
                                 "Resources",
                                 "Scenes"});

      GetFileManager()->PackResources(path);
    }

    void App::SaveAllResources()
    {
      ResourceType types[] = {ResourceType::Material,
                              ResourceType::Mesh,
                              ResourceType::SkinMesh,
                              ResourceType::Animation};

      for (ResourceType t : types)
      {
        for (auto resource : GetResourceManager(t)->m_storage)
        {
          if (!resource.second->IsDynamic())
          {
            String file = resource.second->GetFile();
            if (!IsDefaultResource(file))
            {
              resource.second->m_dirty = true;
              resource.second->Save(true);
            }
          }
        }
      }
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
        if (wnd->GetType() != Window::Type::Viewport &&
            wnd->GetType() != Window::Type::Viewport2d)
        {
          continue;
        }

        if (wnd->IsActive() && wnd->IsVisible())
        {
          return static_cast<EditorViewport*>(wnd);
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
          return dynamic_cast<EditorViewport*>(wnd);
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
          return static_cast<ConsoleWindow*>(wnd);
        }
      }

      return nullptr;
    }

    FolderWindowRawPtrArray App::GetAssetBrowsers()
    {
      return GetAllWindows<FolderWindow>(g_assetBrowserStr);
    }

    OutlinerWindow* App::GetOutliner()
    {
      return GetWindow<OutlinerWindow>(g_outlinerStr);
    }

    PropInspector* App::GetPropInspector()
    {
      return GetWindow<PropInspector>(g_propInspector);
    }

    void App::HideGizmos()
    {
      for (Entity* ntt : GetCurrentScene()->GetEntities())
      {
        // Light and camera gizmos
        if (ntt->IsLightInstance() ||
            ntt->GetType() == EntityType::Entity_Camera)
        {
          ntt->SetVisibility(false, false);
        }
      }
    }

    void App::ShowGizmos()
    {
      for (Entity* ntt : GetCurrentScene()->GetEntities())
      {
        // Light and camera gizmos
        if (ntt->IsLightInstance() ||
            ntt->GetType() == EntityType::Entity_Camera)
        {
          ntt->SetVisibility(true, false);
        }
      }
    }

    void App::ShowSimulationWindow(float deltaTime)
    {
      if (GamePlugin* plugin = GetPluginManager()->GetGamePlugin())
      {
        if (plugin->m_quit)
        {
          SetGameMod(GameMod::Stop);
        }

        if (m_gameMod != GameMod::Stop)
        {
          m_simulationWindow->SetVisibility(m_simulatorSettings.Windowed);

          EditorViewport* playWindow = GetWindow<EditorViewport>(g_3dViewport);
          if (m_simulatorSettings.Windowed)
          {
            if (m_windowCamLoad)
            {
              Mat4 camTs = playWindow->GetCamera()->m_node->GetTransform(
                  TransformationSpace::TS_WORLD);
              m_simulationWindow->GetCamera()->m_node->SetTransform(camTs);
              m_windowCamLoad = false;
            }
            playWindow = m_simulationWindow;
          }
          HideGizmos();
          plugin->Frame(deltaTime, playWindow);
          ShowGizmos();
        }
      }
    }

    void App::Serialize(XmlDocument* doc, XmlNode* parent) const
    {
      m_workspace.Serialize(nullptr, nullptr);

      std::ofstream file;
      String cfgPath  = m_workspace.GetProjectConfigPath();
      String fileName = ConcatPaths({cfgPath, g_editorSettingsFile});

      // File or Config folder is missing.
      std::ios::openmode openMode = std::ios::out;
      if (!CheckSystemFile(fileName))
      {
        std::filesystem::create_directories(cfgPath);
        openMode = std::ios::app;
      }

      file.open(fileName.c_str(), openMode);
      if (file.is_open())
      {
        XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
        XmlNode* app = lclDoc->allocate_node(rapidxml::node_element, "App");
        lclDoc->append_node(app);

        XmlNode* settings =
            lclDoc->allocate_node(rapidxml::node_element, "Settings");
        app->append_node(settings);

        XmlNode* setNode =
            lclDoc->allocate_node(rapidxml::node_element, "Size");

        WriteAttr(setNode,
                  lclDoc.get(),
                  "width",
                  std::to_string(m_renderer->m_windowSize.x));

        WriteAttr(setNode,
                  lclDoc.get(),
                  "height",
                  std::to_string(m_renderer->m_windowSize.y));

        WriteAttr(setNode,
                  lclDoc.get(),
                  "maximized",
                  std::to_string(m_windowMaximized));
        settings->append_node(setNode);

        for (Window* wnd : m_windows)
        {
          wnd->Serialize(lclDoc.get(), app);
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
      XmlFilePtr lclFile    = nullptr;
      XmlDocumentPtr lclDoc = nullptr;

      if (doc == nullptr)
      {
        String settingsFile = ConcatPaths(
            {m_workspace.GetProjectConfigPath(), g_editorSettingsFile});

        if (!CheckFile(settingsFile))
        {
          settingsFile = ConcatPaths({ConfigPath(), g_editorSettingsFile});

          assert(CheckFile(settingsFile) &&
                 "ToolKit/Config/Editor.settings must exist.");
        }

        lclFile = std::make_shared<XmlFile>(settingsFile.c_str());
        lclDoc  = std::make_shared<XmlDocument>();
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

    void App::OverrideEntityConstructors()
    {
      GetEntityFactory()->OverrideEntityConstructor(
          EntityType::Entity_Camera,
          []() -> Entity* { return new EditorCamera(); });

      GetEntityFactory()->OverrideEntityConstructor(
          EntityType::Entity_DirectionalLight,
          []() -> Entity* { return new EditorDirectionalLight(); });

      GetEntityFactory()->OverrideEntityConstructor(
          EntityType::Entity_PointLight,
          []() -> Entity* { return new EditorPointLight(); });

      GetEntityFactory()->OverrideEntityConstructor(
          EntityType::Entity_SpotLight,
          []() -> Entity* { return new EditorSpotLight(); });
    }

    void App::CreateSimulationWindow(float width, float height)
    {
      m_simulationWindow         = new EditorViewport(m_simulatorSettings.Width,
                                              m_simulatorSettings.Height);

      m_simulationWindow->m_name = g_simulationViewport;
      m_simulationWindow->m_additionalWindowFlags = ImGuiWindowFlags_NoResize |
                                                    ImGuiWindowFlags_NoDocking |
                                                    ImGuiWindowFlags_NoCollapse;

      m_simulationWindow->SetVisibility(false);
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
      EditorScenePtr scene =
          std::make_shared<EditorScene>(ScenePath(name + SCENE));

      scene->m_name     = name;
      scene->m_newScene = true;
      GetSceneManager()->Manage(scene);
      SetCurrentScene(scene);
    }

    void App::CreateEditorEntities()
    {
      // Create editor objects.
      m_cursor = new Cursor();
      m_origin = new Axis3d();

      m_grid   = new Grid(g_max2dGridSize, AxisLabel::ZX, 0.020f, 3.0, false);

      m_2dGrid = new Grid(g_max2dGridSize,
                          AxisLabel::XY,
                          10.0f,
                          4.0,
                          true); // Generate grid cells 10 x 10
    }

    float App::GetDeltaTime() { return m_deltaTime; }

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
      g_app->m_perFrameDebugObjects.push_back(
          CreateBoundingBoxDebugObject({p - Vec3(size), p + Vec3(size)}));
    }

    void DebugLineStrip(const Vec3Array& pnts)
    {
      g_app->m_perFrameDebugObjects.push_back(CreateLineDebugObject(pnts));
    }

  } // namespace Editor
} // namespace ToolKit
