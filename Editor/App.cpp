#include "stdafx.h"

#include "App.h"
#include "Renderer.h"
#include "UI.h"
#include "Viewport.h"
#include "Primative.h"
#include "Node.h"
#include "GlobalDef.h"
#include "OverlayUI.h"
#include "Grid.h"
#include "Directional.h"
#include "Mod.h"
#include "ConsoleWindow.h"
#include "Gizmo.h"
#include "FolderWindow.h"
#include "OutlinerWindow.h"
#include "PropInspector.h"
#include "DebugNew.h"

#include <filesystem>

namespace ToolKit
{
  namespace Editor
  {

    App::App(int windowWidth, int windowHeight)
      : m_workspace(this)
    {
      m_cursor = nullptr;
      m_lightMaster = nullptr;
      m_renderer = new Renderer();
      m_renderer->m_windowWidth = windowWidth;
      m_renderer->m_windowHeight = windowHeight;
      m_statusMsg = "OK";
    }

    App::~App()
    {
      Destroy();

      // Engine components.
      SafeDel(m_renderer);
    }

    void App::Init()
    {
      m_cursor = new Cursor();
      m_origin = new Axis3d();
      m_grid = new Grid(100);
      m_grid->m_mesh->Init(false);

      MaterialPtr solidColorMaterial = GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      m_highLightMaterial = solidColorMaterial->Copy<Material>();
      m_highLightMaterial->m_color = g_selectHighLightPrimaryColor;
      m_highLightMaterial->GetRenderState()->cullMode = CullingType::Front;

      m_highLightSecondaryMaterial = solidColorMaterial->Copy<Material>();
      m_highLightSecondaryMaterial->m_color = g_selectHighLightSecondaryColor;
      m_highLightSecondaryMaterial->GetRenderState()->cullMode = CullingType::Front;

      ModManager::GetInstance()->Init();
      ModManager::GetInstance()->SetMod(true, ModId::Select);
      ActionManager::GetInstance()->Init();

      // Lights and camera.
      m_lightMaster = new Node();

      Light* light = new Light();
      light->Yaw(glm::radians(-45.0f));
      m_lightMaster->AddChild(light->m_node);
      m_sceneLights.push_back(light);

      light = new Light();
      light->m_intensity = 0.5f;
      light->Yaw(glm::radians(60.0f));
      m_lightMaster->AddChild(light->m_node);
      m_sceneLights.push_back(light);

      light = new Light();
      light->m_intensity = 0.3f;
      light->Yaw(glm::radians(-140.0f));
      m_lightMaster->AddChild(light->m_node);
      m_sceneLights.push_back(light);
      
      m_workspace.Init();
      m_scene = std::make_shared<EditorScene>(ScenePath("New Scene" + SCENE));

      ApplyProjectSettings(m_onNewScene);
      if (!CheckFile(m_workspace.GetActiveWorkspace()))
      {
        StringInputWindow* wsDir = new StringInputWindow("Set Workspace Directory##SetWsdir", false);
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

      // Register plugin reporter.
      GetPluginManager()->m_reporterFn = [](const String& msg) -> void
      {
        if (ConsoleWindow* console = g_app->GetConsole())
        {
          console->AddLog(msg, "plugin");
        }
      };
    }

    void App::Destroy()
    {
      // UI.
      DeleteWindows();

      m_scene->Destroy();

      // Editor objects.
      SafeDel(m_grid);
      SafeDel(m_origin);
      SafeDel(m_cursor);
      SafeDel(m_lightMaster);
      for (int i = 0; i < 3; i++)
      {
        SafeDel(m_sceneLights[i]);
      }
      assert(m_sceneLights.size() == 3);
      m_sceneLights.clear();

      GetAnimationPlayer()->m_records.clear();

      ModManager::GetInstance()->UnInit();
      ActionManager::GetInstance()->UnInit();
    }

    void App::Frame(float deltaTime)
    {
      // Update animations.
      GetAnimationPlayer()->Update(MilisecToSec(deltaTime));

      // Update Mods.
      ModManager::GetInstance()->Update(deltaTime);

      if (Window* wnd = GetOutliner())
      {
        wnd->DispatchSignals();
      }

      // Update Viewports.
      for (Window* wnd : m_windows)
      {
        if (wnd->GetType() != Window::Type::Viewport)
        {
          continue;
        }

        wnd->DispatchSignals();

        Viewport* vp = static_cast<Viewport*> (wnd);
        vp->Update(deltaTime);

        // Adjust scene lights.
        Camera* cam = vp->m_camera;
        m_lightMaster->OrphanSelf();
        cam->m_node->AddChild(m_lightMaster);

        m_renderer->SetRenderTarget(vp->m_viewportImage);

        for (Entity* ntt : m_scene->GetEntities())
        {
          if (ntt->IsDrawable())
          {
            if (ntt->GetType() == EntityType::Entity_Billboard)
            {
              Billboard* billboard = static_cast<Billboard*> (ntt);
              billboard->LookAt(cam, vp->m_height);
            }

            m_renderer->Render(static_cast<Drawable*> (ntt), cam, m_sceneLights);
          }
        }

        RenderSelected(vp);

        if (!m_perFrameDebugObjects.empty())
        {
          for (Drawable* d : m_perFrameDebugObjects)
          {
            m_renderer->Render(d, cam);
            SafeDel(d);
          }
          m_perFrameDebugObjects.clear();
        }

        // Scale grid spacing.
        if (cam->IsOrtographic())
        {
          Vec3 pos = cam->m_node->GetTranslation(TransformationSpace::TS_WORLD);
          float dist = glm::distance(Vec3(), pos);
          float scale = glm::max(1.0f, glm::trunc(dist / 100.0f));
          m_grid->Resize(500, 1.0f / scale);
        }
        else
        {
          m_grid->Resize(500, 1.0f);
        }
        m_renderer->Render(m_grid, cam);

        m_origin->LookAt(cam, vp->m_height);
        m_renderer->Render(m_origin, cam);

        // Only draw gizmo in active viewport.
        if (m_gizmo != nullptr && vp->IsActive())
        {
          m_gizmo->LookAt(cam, vp->m_height);
          glClear(GL_DEPTH_BUFFER_BIT);
          if (PolarGizmo* pg = dynamic_cast<PolarGizmo*> (m_gizmo))
          {
            pg->Render(m_renderer, cam);
          }
          else
          {
            m_renderer->Render(m_gizmo, cam);
          }
        }

        float orthScl = 1.0f;
        if (vp->m_orthographic)
        {
          // Magic scale to match Billboards in perspective view with ortoghrapic view.
          orthScl = 1.6f;
        }
        m_cursor->LookAt(cam, vp->m_height * orthScl);
        m_renderer->Render(m_cursor, cam);
      }

      m_renderer->SetRenderTarget(nullptr);

      // Render UI.
      UI::ShowUI();
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
      m_scene = std::make_shared<EditorScene>(ScenePath(name + SCENE));
      m_onNewScene = false;
    }

    void App::OnSaveScene()
    {
      auto saveFn = []() -> void
      {
        g_app->m_scene->Save(false);
        g_app->GetAssetBrowser()->UpdateContent();
      };

      // File existance check.
      String fullPath = m_scene->m_file;
      if (m_scene->m_newScene && CheckFile(fullPath))
      {
        String msg = "Scene " + fullPath + " exist on the disk.\nOverride the existing scene ?";
        YesNoWindow* overrideScene = new YesNoWindow("Override existing file##OvrdScn", msg);
        overrideScene->m_yesCallback = [&saveFn]()
        {
          saveFn();
        };

        overrideScene->m_noCallback = []()
        {
          g_app->GetConsole()->AddLog("Scene has not been saved.\nA scene with the same name exist. Use File->SaveAs.", ConsoleWindow::LogType::Error);
        };

        UI::m_volatileWindows.push_back(overrideScene);
      }
      else
      {
        saveFn();
      }
    }

    void App::OnQuit()
    {
      static bool processing = false;
      if (!processing)
      {
        YesNoWindow* reallyQuit = new YesNoWindow("Quiting... Are you sure?##ClsApp");
        reallyQuit->m_yesCallback = [this]()
        {
          m_workspace.Serialize(nullptr, nullptr);
          Serialize(nullptr, nullptr);
          g_running = false;
        };

        reallyQuit->m_noCallback = []()
        {
          processing = false;
        };

        UI::m_volatileWindows.push_back(reallyQuit);
        processing = true;
      }
    }

    void App::OnNewProject(const String& name)
    {
      if (m_workspace.GetActiveWorkspace().empty())
      {
        GetConsole()->AddLog("No workspace. Project can't be created.", ConsoleWindow::LogType::Error);
        return;
      }

      String fullPath = ConcatPaths({ m_workspace.GetActiveWorkspace(), name });
      if (CheckFile(fullPath))
      {
        GetConsole()->AddLog("Project already exist.", ConsoleWindow::LogType::Error);
        return;
      }

      std::filesystem::create_directories(ConcatPaths({ fullPath, "Resources", "Audio" }));
      std::filesystem::create_directories(ConcatPaths({ fullPath, "Resources", "Fonts" }));
      std::filesystem::create_directories(ConcatPaths({ fullPath, "Resources", "Materials" }));
      std::filesystem::create_directories(ConcatPaths({ fullPath, "Resources", "Meshes" }));
      std::filesystem::create_directories(ConcatPaths({ fullPath, "Resources", "Scenes" }));
      std::filesystem::create_directories(ConcatPaths({ fullPath, "Resources", "Shaders" }));
      std::filesystem::create_directories(ConcatPaths({ fullPath, "Resources", "Sprites" }));
      std::filesystem::create_directories(ConcatPaths({ fullPath, "Resources", "Textures" }));
      OpenProject({ name, "" });
    }

    void App::ResetUI()
    {      
      DeleteWindows();
      if (CheckFile(ConcatPaths({ DefaultPath(), "default.settings" })))
      {
        // Try reading defaults.
        String settingsFile = ConcatPaths({ DefaultPath(), "default.settings" });
        std::shared_ptr<XmlFile> lclFile = std::make_shared<XmlFile>(settingsFile.c_str());
        
        XmlDocumentPtr lclDoc = std::make_shared<XmlDocument>();
        lclDoc->parse<0>(lclFile->data());
        
        XmlNode* app = Query(lclDoc.get(), { "App" });
        CreateWindows(app);

        settingsFile = ConcatPaths({ DefaultPath(), "defaultUI.ini" });
        ImGui::LoadIniSettingsFromDisk(settingsFile.c_str());
      }
      else
      {
        // Perspective.
        Viewport* vp = new Viewport(m_renderer->m_windowWidth * 0.8f, m_renderer->m_windowHeight * 0.8f);
        vp->m_name = "Perspective";
        vp->m_camera->m_node->SetTranslation({ 5.0f, 3.0f, 5.0f });
        vp->m_camera->LookAt(Vec3(0.0f));
        m_windows.push_back(vp);

        // Orthographic.
        vp = new Viewport(m_renderer->m_windowWidth * 0.8f, m_renderer->m_windowHeight * 0.8f);
        vp->m_name = "Orthographic";
        vp->m_camera->m_node->SetTranslation({ 0.0f, 500.0f, 0.0f });
        vp->m_camera->Pitch(glm::radians(-90.0f));
        vp->m_cameraAlignment = 1;
        vp->m_orthographic = true;
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
      }
    }

    void App::DeleteWindows()
    {
      for (Window* wnd : m_windows)
      {
        SafeDel(wnd);
      }
      m_windows.clear();

      for (size_t i = 0; i < Viewport::m_overlays.size(); i++)
      {
        SafeDel(Viewport::m_overlays[i]);
      }
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
            wnd = new Viewport(wndNode);
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
    }

    int App::Import(const String& fullPath, const String& subDir, bool overwrite)
    {
      bool doSearch = !UI::SearchFileData.missingFiles.empty();
      if (!CanImport(fullPath) && !doSearch)
      {
        if (ConsoleWindow* con = GetConsole())
        {
          con->AddLog("Import failed: " + fullPath, ConsoleWindow::LogType::Error);
          con->AddLog("File format is not supported.\nSuported formats are fbx, glb, gltf, obj.", ConsoleWindow::LogType::Error);
        }
        return -1;
      }

      bool importFileExist = CheckFile(fullPath);

      // Set the execute path.
      std::filesystem::path pathBck = std::filesystem::current_path();
      std::filesystem::path path = pathBck.u8string() + ConcatPaths({ ".", "..", "Utils", "Import" });
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
                g_app->GetConsole()->AddLog("Import: " + fullPath + " failed.", ConsoleWindow::LogType::Error);
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
                  String missingFullPath = ConcatPaths({ searchPath, name + ext });
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
            assert(false);
            //mesh = GetSkinMeshManager()->Create(meshFile);
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
      m_scene->Destroy();
      m_scene = GetSceneManager()->Create<EditorScene>(fullPath);
      m_scene->Load(); // Make sure its loaded.
      m_scene->Init(false);
      m_workspace.SetScene(m_scene->m_name);
      m_scene->m_newScene = false;
    }

    void App::MergeScene(const String& fullPath)
    {
      ScenePtr scene = GetSceneManager()->Create<EditorScene>(fullPath);
      scene->Load();
      scene->Init(false);

      EntityRawPtrArray newNtts = scene->GetEntities();
      for (Entity* e : newNtts)
      {
        g_app->m_scene->AddEntity(e);
      }
    }

    void App::ApplyProjectSettings(bool setDefaults)
    {
      if (CheckFile(ConcatPaths({ ResourcePath(), "Editor.settings" })) && !setDefaults)
      {
        DeSerialize(nullptr, nullptr);

        // Restore window.
        SDL_SetWindowSize(g_window, m_renderer->m_windowWidth, m_renderer->m_windowHeight);
        SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        if (m_windowMaximized)
        {
          SDL_MaximizeWindow(g_window);
        }

        UI::InitSettings();
      }
      else
      {
        ResetUI();
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

    Viewport* App::GetActiveViewport()
    {
      for (Window* wnd : m_windows)
      {
        if (wnd->GetType() != Window::Type::Viewport)
        {
          continue;
        }

        if (wnd->IsActive() && wnd->IsVisible())
        {
          return static_cast<Viewport*> (wnd);
        }
      }

      return nullptr;
    }

    Viewport* App::GetViewport(const String& name)
    {
      for (Window* wnd : m_windows)
      {
        if (wnd->m_name == name)
        {
          return dynamic_cast<Viewport*> (wnd);
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

    template<typename T>
    T* App::GetWindow(const String& name)
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

    void App::RenderSelected(Viewport* vp)
    {
      if (m_scene->GetSelectedEntityCount() == 0)
      {
        return;
      }

      auto RenderFn = [this, vp](const EntityRawPtrArray& selection, const Vec3& color) -> void
      {
        if (selection.empty())
        {
          return;
        }

        RenderTarget stencilMask((int)vp->m_width, (int)vp->m_height);
        stencilMask.Init();

        m_renderer->SetRenderTarget(&stencilMask, true, { 0.0f, 0.0f, 0.0f, 1.0 });

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // webgl create problem with depth only drawing with textures.
        static MaterialPtr solidMat = GetMaterialManager()->GetCopyOfSolidMaterial();
        m_renderer->m_overrideMat = solidMat;

        for (Entity* ntt : selection)
        {
          if (ntt->IsDrawable())
          {
            m_renderer->Render(static_cast<Drawable*> (ntt), vp->m_camera);
          }
        }

        m_renderer->m_overrideMat = nullptr;

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glStencilFunc(GL_NOTEQUAL, 0xFF, 0xFF);
        glStencilMask(0x00);
        ShaderPtr solidColor = GetShaderManager()->Create<Shader>(ShaderPath("unlitColorFrag.shader", true));
        m_renderer->DrawFullQuad(solidColor);
        glDisable(GL_STENCIL_TEST);

        m_renderer->SetRenderTarget(vp->m_viewportImage, false);

        // Dilate.
        glBindTexture(GL_TEXTURE_2D, stencilMask.m_textureId);
        ShaderPtr dilate = GetShaderManager()->Create<Shader>(ShaderPath("dilateFrag.shader", true));
        dilate->SetShaderParameter("Color", color);
        m_renderer->DrawFullQuad(dilate);
      };

      EntityRawPtrArray selecteds;
      m_scene->GetSelectedEntities(selecteds);
      Entity* primary = selecteds.back();

      selecteds.pop_back();
      RenderFn(selecteds, g_selectHighLightSecondaryColor);

      selecteds.clear();
      selecteds.push_back(primary);
      RenderFn(selecteds, g_selectHighLightPrimaryColor);

      if (m_showSelectionBoundary && primary->IsDrawable())
      {
        Drawable* dw = static_cast<Drawable*> (primary);
        m_perFrameDebugObjects.push_back(GenerateBoundingVolumeGeometry(dw->GetAABB(true)));
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
        XmlDocumentPtr lclDoc = std::make_shared<XmlDocument> ();
        XmlNode* app = lclDoc->allocate_node(rapidxml::node_element, "App");
        lclDoc->append_node(app);

        XmlNode* settings = lclDoc->allocate_node(rapidxml::node_element, "Settings");
        app->append_node(settings);

        XmlNode* setNode = lclDoc->allocate_node(rapidxml::node_element, "Size");
        WriteAttr(setNode, lclDoc.get(), "width", std::to_string(m_renderer->m_windowWidth));
        WriteAttr(setNode, lclDoc.get(), "height", std::to_string(m_renderer->m_windowHeight));
        WriteAttr(setNode, lclDoc.get(), "maximized", std::to_string(m_windowMaximized));
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
        String settingsFile = ConcatPaths({ ResourcePath(), "Editor.settings" });
        lclFile = std::make_shared<XmlFile> (settingsFile.c_str());

        lclDoc = std::make_shared<XmlDocument> ();
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
        String fullPath = ScenePath(scene + SCENE);
        OpenScene(fullPath);
      }
    }

  }
}
