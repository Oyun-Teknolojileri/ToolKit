#include "Game.h"
#include "Common/SDLEventPool.h"
#include <android/asset_manager.h>
#include "EngineSettings.h"
#include "GlErrorReporter.h"
#include "SDL.h"
#include "Scene.h"
#include "MobileSceneRenderPath.h"
#include "ToolKit.h"
#include "Types.h"
#include "UIManager.h"
#include "TKStats.h"

#include <stdio.h>

#include <chrono>
#include <iostream>
#include <GLES3/gl32.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <sys/stat.h>

#include <unistd.h>

#define ANDROID_LOG(format, ...) __android_log_print(ANDROID_LOG_DEBUG, "TK_LOG", format, ##__VA_ARGS__)

#define TK_PLATFORM PLATFORM::TKAndroid

namespace ToolKit
{
  Game* g_game                     = nullptr;
  bool g_running                   = true;
  SDL_Window* g_window             = nullptr;
  Main* g_proxy                    = nullptr;
  Viewport* g_viewport             = nullptr;
  EngineSettings* g_engineSettings = nullptr;
  SDLEventPool<TK_PLATFORM>* g_sdlEventPool     = nullptr;
  AAssetManager* assetManager = nullptr;

  // Setup.
  const char* g_appName            = "ToolKit";
  const uint g_targetFps           = 120;

  void SceneRender(Viewport* viewport)
  {
    if (ScenePtr scene = GetSceneManager()->GetCurrentScene())
    {
      static MobileSceneRenderPath sceneRenderer;
      sceneRenderer.m_params.Cam                        = viewport->GetCamera();
      sceneRenderer.m_params.ClearFramebuffer           = true;
      sceneRenderer.m_params.Gfx.BloomEnabled           = false;
      sceneRenderer.m_params.Gfx.DepthOfFieldEnabled    = false;
      sceneRenderer.m_params.Gfx.FXAAEnabled            = false;
      sceneRenderer.m_params.Gfx.GammaCorrectionEnabled = false;
      sceneRenderer.m_params.Gfx.SSAOEnabled            = false;
      sceneRenderer.m_params.Gfx.TonemappingEnabled     = false;
      sceneRenderer.m_params.Lights                     = scene->GetLights();
      sceneRenderer.m_params.MainFramebuffer            = viewport->m_framebuffer;
      sceneRenderer.m_params.Scene                      = scene;
      GetRenderSystem()->AddRenderTask(&sceneRenderer);
    }

    static uint totalFrameCount = 0;
    GetRenderSystem()->SetFrameCount(totalFrameCount++);

    GetRenderSystem()->ExecuteRenderTasks();
  }

  struct UIRenderTechniqueParams
  {
    Viewport* viewport = nullptr;
  };

  class UIRenderTechnique : public RenderPath
  {
   public:
    UIRenderTechnique() { m_uiPass = std::make_shared<ForwardRenderPass>(); }

    void Render(Renderer* renderer) override
    {
      m_passArray.clear();

      // UI pass.
      UILayerPtrArray layers;
      m_uiRenderJobs.clear();
      GetUIManager()->GetLayers(m_params.viewport->m_viewportId, layers);

      for (const UILayerPtr& layer : layers)
      {
        EntityPtrArray& uiNtties = layer->m_scene->AccessEntityArray();
        RenderJobProcessor::CreateRenderJobs(uiNtties, m_uiRenderJobs);
      }

      m_uiPass->m_params.OpaqueJobs.clear();
      m_uiPass->m_params.TranslucentJobs.clear();

      RenderJobProcessor::SeperateOpaqueTranslucent(m_uiRenderJobs,
                                                    m_uiPass->m_params.OpaqueJobs,
                                                    m_uiPass->m_params.TranslucentJobs);

      m_uiPass->m_params.Cam              = GetUIManager()->GetUICamera();
      m_uiPass->m_params.FrameBuffer      = m_params.viewport->m_framebuffer;
      m_uiPass->m_params.ClearFrameBuffer = false;
      m_uiPass->m_params.ClearDepthBuffer = true;

      m_passArray.push_back(m_uiPass);

      RenderPath::Render(renderer);
    }

    UIRenderTechniqueParams m_params;

   private:
    ForwardRenderPassPtr m_uiPass = nullptr;
    RenderJobArray m_uiRenderJobs;
  };

  UIRenderTechnique m_uiRenderTechnique;

  void UIRender(Viewport* viewport)
  {
    m_uiRenderTechnique.m_params.viewport = viewport;
    GetRenderSystem()->AddRenderTask(&m_uiRenderTechnique);
  }

  struct GammaCorrectionTechniqueParams
  {
    FramebufferPtr framebuffer = nullptr;
    float gamma                = 2.2f;
  };

  class GammaCorrectionTechnique : public RenderPath
  {
   public:
    GammaCorrectionTechnique()
    {
      m_gammaPass = std::make_shared<GammaPass>();

      m_passArray.resize(1);
    }

    void Render(Renderer* renderer) override
    {
      m_gammaPass->m_params.FrameBuffer = m_params.framebuffer;
      m_gammaPass->m_params.Gamma       = m_params.gamma;

      m_passArray[0]                    = m_gammaPass;

      RenderPath::Render(renderer);
    }

    GammaCorrectionTechniqueParams m_params;

   private:
    GammaPassPtr m_gammaPass = nullptr;
  };

  void GammaCorrection(Viewport* viewport)
  {
    static GammaCorrectionTechnique m_gammaCorrectionTechnique;
    m_gammaCorrectionTechnique.m_params.framebuffer = viewport->m_framebuffer;
    GetRenderSystem()->AddRenderTask(&m_gammaCorrectionTechnique);
  }

  bool g_enableGammaPass = false;

  GLuint VAO, VBO, EBO;
  GLuint shaderProgram;

  void InitRender()
  {
    float vertices[] = {
        // positions      // texture coords
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top right
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
        2,
        1,
        0, // first triangle
        2,
        0,
        3 // second triangle
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    const char* vertexShader   = "#version 300 es\n"
                                 "      precision highp float;\n"
                                 "      precision highp sampler2D;\n"
                                 "      precision highp samplerCube;\n"
                                 "      precision highp sampler2DArray;\n"
                                 "      precision highp int;\n"
                                 "      layout(location = 0) in vec3 vPosition;\n"
                                 "      layout(location = 1) in vec2 vTexture;\n"
                                 "      out vec2 vTex;\n"
                                 "       void main()\n"
                                 "       {\n"
                                 "         gl_Position = vec4(vPosition, 1.0);\n"
                                 "         vTex = vTexture;\n"
                                 "       }\n";
    const char* fragmentShader = "#version 300 es\n"
                                 "      precision highp float;\n"
                                 "      precision highp sampler2D;\n"
                                 "      precision highp samplerCube;\n"
                                 "      precision highp sampler2DArray;\n"
                                 "      precision highp int;\n"
                                 "      in vec2 vTex;\n"
                                 "      out vec4 outFragColor;\n"
                                 "       uniform sampler2D tex;\n"
                                 "       void main()\n"
                                 "       {\n"
                                 "         vec3 color = texture(tex, vTex).rgb;\n"
                                 "         //vec3 color = vec3(1.0, 1.0, 1.0);\n"
                                 "         outFragColor = vec4(color, 1.0);\n"
                                 "       }\n";

    GLuint vs                  = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, 0);
    glCompileShader(vs);
    GLint isCompiled = 0;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
      g_proxy->m_logger->Log("No compile vs\n");
      GLint maxLength = 0;
      glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);
      char* infoLog = new char[maxLength];
      glGetShaderInfoLog(vs, maxLength, &maxLength, &infoLog[0]);
      g_proxy->m_logger->Log(infoLog);
      g_proxy->m_logger->Log("\n");
      glDeleteShader(vs);
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, 0);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
      g_proxy->m_logger->Log("No compile fs\n");
      GLint maxLength = 0;
      glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);
      char* infoLog = new char[maxLength];
      glGetShaderInfoLog(fs, maxLength, &maxLength, &infoLog[0]);
      g_proxy->m_logger->Log(infoLog);
      g_proxy->m_logger->Log("\n");
      glDeleteShader(fs);
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    GLint isLinked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, (int*) &isLinked);
    if (isLinked == GL_FALSE)
    {
      GLint maxLength = 0;
      glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
      char* infoLog = new char[maxLength];
      glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
      glDeleteProgram(shaderProgram);
      glDeleteShader(vs);
      glDeleteShader(fs);
    }
  }

  void Render(GLuint tex)
  {
    GLint ct, cf;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &ct);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &cf);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, g_engineSettings->Window.Width, g_engineSettings->Window.Height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(shaderProgram);
    GLint loc = glGetUniformLocation(shaderProgram, "tex");

    glUniform1i(loc, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, ct);
    glBindFramebuffer(GL_FRAMEBUFFER, cf);
  }

  void DeleteRender()
  {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
  }

  class GameViewport : public Viewport
  {
   public:
    GameViewport() { m_contentAreaLocation = Vec2(0.0f); }

    GameViewport(float width, float height) : Viewport(width, height) {}

    void Update(float dt) override
    {
      m_mouseOverContentArea = true;

      for (Event* e : Main::GetInstance()->m_eventPool)
      {
        if (e->m_type == Event::EventType::Mouse)
        {
          MouseEvent* me = static_cast<MouseEvent*>(e);
            m_lastMousePosRelContentArea.x = me->absolute[0];
            m_lastMousePosRelContentArea.y = me->absolute[1];
        }
          if (e->m_type == Event::EventType::Touch)
          {
              TouchEvent* te = static_cast<TouchEvent*>(e);
              m_lastMousePosRelContentArea.x = te->absolute[0] * m_wndContentAreaSize.x;
              m_lastMousePosRelContentArea.y = te->absolute[1] * m_wndContentAreaSize.y;
          }
      }
    }
  };

  // copies all of the engine assets to internal data folder if already not copied
  void CopyAllAssetsToDataPath(String& internalDataPath)
  {
    const char* MinResourcesPak = "MinResources.pak";
    AAsset* asset = AAssetManager_open(assetManager, MinResourcesPak, 0);

    if (!asset)
    {
      ANDROID_LOG("cannot open MinResources.pak!\n");
      return;
    }
    FILE* fileHandle = fopen(ConcatPaths({ internalDataPath, MinResourcesPak }).c_str(), "wb");
    mkdir(ConcatPaths({ internalDataPath, "Resources" }).c_str(), 0777);

    off_t size = AAsset_getLength(asset);
    std::vector<char> buffer;
    buffer.resize(size + 1, '\0');
    AAsset_read(asset, buffer.data(), size);
    fwrite(buffer.data(), 1, size + 1, fileHandle);
    memset(buffer.data(), 0, buffer.capacity());
    fclose(fileHandle);
    AAsset_close(asset);
  }

  void PreInit()
  {
    g_sdlEventPool = new SDLEventPool<TK_PLATFORM>();

    // PreInit Main
    g_proxy        = new Main();
    Main::SetProxy(g_proxy);

    g_proxy->PreInit();
    auto androidWriteConsoleFn = [](LogType logType, const String& msg)
    {
      ANDROID_LOG("%s", msg.c_str());
    };
    GetLogger()->SetWriteConsoleFn(androidWriteConsoleFn);

    g_proxy->m_engineSettings->Window.Height = 2400;
    g_proxy->m_engineSettings->Window.Width = 1080;

    String internalPath = SDL_AndroidGetInternalStoragePath();
    g_proxy->m_resourceRoot = ConcatPaths({internalPath, "Resources"});
    g_proxy->m_cfgPath      = ConcatPaths({internalPath, "Config"});
    CopyAllAssetsToDataPath(internalPath);
  }

  void Init()
  {
    g_proxy->m_engineSettings->Window.Width = 2400;
    g_proxy->m_engineSettings->Window.Height = 1080;
    g_engineSettings = g_proxy->m_engineSettings;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0)
    {
      g_running = false;
    }
    else
    {
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

      // EGL does not support sRGB backbuffer. Need to use an extension
      // https://stackoverflow.com/questions/20396523/android-egl-srgb-default-renderbuffer

      g_window =
          SDL_CreateWindow(g_appName,
                           SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED,
                           g_engineSettings->Window.Width,
                           g_engineSettings->Window.Height,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

      if (g_window == nullptr)
      {
        const char* err = SDL_GetError();
        g_running = false;
      }
      else
      {
        SDL_Renderer* sdlRenderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);

        if (sdlRenderer == nullptr)
        {
          const char* error = SDL_GetError();
          ANDROID_LOG("%s", error);
          g_running = false;
        }
        else
        {
          int rendererWidth,rendererHeight;
          SDL_GetRendererOutputSize(sdlRenderer, &rendererWidth, &rendererHeight);
          g_engineSettings->Window.Width = rendererWidth;
          g_engineSettings->Window.Height = rendererHeight;

          const char* error = SDL_GetError();
          ANDROID_LOG("%s", error);

          // Init OpenGl.
          g_proxy->m_renderSys->InitGl((void*)SDL_GL_GetProcAddress, [](const String& msg) { ANDROID_LOG("%s", msg.c_str()); });
          //gladLoadGLES2((GLADloadfunc) SDL_GL_GetProcAddress);
          glEnable(GL_DEPTH_TEST);

          // Set defaults
          SDL_GL_SetSwapInterval(0);

          // ToolKit Init
          g_proxy->Init();

          // Init game
          g_viewport =
              new GameViewport((float) g_engineSettings->Window.Width, (float) g_engineSettings->Window.Height);
          g_game = new Game();
          g_game->Init(g_proxy);
          g_game->SetViewport(g_viewport);

          g_game->m_currentState = PluginState::Running;

          InitRender();

          g_game->OnPlay();
        }
      }
    }
  }

  float GetMilliSeconds()
  {
    static std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point t2        = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

    return static_cast<float>(time_span.count() * 1000.0);
  }

  void Exit()
  {
    DeleteRender();

    g_game->Destroy();
    Main::GetInstance()->Uninit();
    SafeDel(g_proxy);

    SafeDel(g_sdlEventPool);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    g_running = false;
  }

  void ProcessEvent(const SDL_Event& e)
  {
    if (e.type == SDL_QUIT)
    {
      g_running = false;
    }
  }

  struct CustomTimer
  {
    CustomTimer()
    {
      lastTime    = GetMilliSeconds();
      currentTime = 0.0f;
      deltaTime   = 1000.0f / g_targetFps;
      frameCount  = 0;
      timeAccum   = 0.0f;
    }

    float lastTime    = 0.0f;
    float currentTime = 0.0f;
    float deltaTime   = 0.0f;
    float timeAccum   = 0.0f;
    int frameCount    = 0;
  };

  void TK_Loop()
  {
    static CustomTimer timer;
    SDL_Event sdlEvent;

    g_proxy->m_engineSettings->Graphics.ShadowDistance = 150.0f;

    while (g_game->m_currentState != PluginState::Stop && g_running)
    {
      GetTKStats()->ResetDrawCallCounter();
      GetTKStats()->ResetHWRenderPassCounter();

      float frameStart = GetMilliSeconds();

      while (SDL_PollEvent(&sdlEvent))
      {
        g_sdlEventPool->PoolEvent(sdlEvent);
        ProcessEvent(sdlEvent);
      }

      timer.currentTime = GetMilliSeconds();
      if (timer.currentTime > timer.lastTime + timer.deltaTime)
      {
        float deltaTime = timer.currentTime - timer.lastTime;

        if (ScenePtr scene = GetSceneManager()->GetCurrentScene())
        {
          scene->Update(deltaTime);
        }

        GetAnimationPlayer()->Update(MillisecToSec(deltaTime));

        g_viewport->Update(deltaTime);

        g_game->Frame(deltaTime);

        SceneRender(g_viewport);

        GetUIManager()->UpdateLayers(deltaTime, g_viewport);

        if (GetRenderSystem()->IsSkipFrame())
        {
          GetRenderSystem()->FlushRenderTasks();
        }
        else
        {
          UIRender(g_viewport);

          if (GetRenderSystem()->IsGammaCorrectionNeeded())
          {
            GammaCorrection(g_viewport);
          }
        }

        GetRenderSystem()->ExecuteRenderTasks();
        GetRenderSystem()->DecrementSkipFrame();

        Render(g_viewport->m_framebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0)->m_textureId);

        g_sdlEventPool->ClearPool(); // Clear after consumption.
        SDL_GL_SwapWindow(g_window);

        timer.frameCount++;
        timer.timeAccum += deltaTime;
        if (timer.timeAccum >= 1000.0f)
        {
          timer.timeAccum  = 0;
          timer.frameCount = 0;
        }

        timer.lastTime = timer.currentTime;

        float frameEnd = GetMilliSeconds();
      }
    }
  }

  int ToolKit_Main(int argc, char* argv[])
  {
    PreInit();
    Init();

    TK_Loop();

    Exit();
    return 0;
  }
} // namespace ToolKit

int main(int argc, char* argv[]) { return ToolKit::ToolKit_Main(argc, argv); }

extern "C"
{
  JNIEXPORT void JNICALL
  Java_com_otyazilim_toolkit_ToolKitAndroid_load(JNIEnv *env, jclass clazz, jobject mgr) {
    ToolKit::assetManager = AAssetManager_fromJava(env, mgr);
    if (ToolKit::assetManager == nullptr) {
      __android_log_print(ANDROID_LOG_ERROR, "ToolKit_Android", "error loading asset manager");
    } else {
      __android_log_print(ANDROID_LOG_VERBOSE, "ToolKit_Android", "Asset manager loaded successfully");
    }
  }
}
