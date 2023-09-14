
#include "AndroidMain.h"
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <memory>
#include <vector>
#include <chrono>
#include <filesystem>
#include <sys/stat.h>

#include "ToolKit.h"
#include "Logger.h"
#include "Util.h"
#include "Game.h"
#include "SceneRenderer.h"

#ifndef _NDEBUG
#define CHECK_GL_ERROR() checkGLError(__FILE_NAME__, __LINE__)
#else
#define CHECK_GL_ERROR()
#endif

#include <android/log.h>

#define ANDROID_LOG(format, ...) __android_log_print(ANDROID_LOG_DEBUG, "TK_LOG", format, ##__VA_ARGS__)

static void checkGLError(const char* file, int line)
{
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR)
  {
    const char* errorString;
    switch (error) {
      case GL_INVALID_ENUM: errorString = "GL_INVALID_ENUM"; break;
      case GL_INVALID_VALUE: errorString = "GL_INVALID_VALUE"; break;
      case GL_INVALID_OPERATION: errorString = "GL_INVALID_OPERATION"; break;
      case GL_OUT_OF_MEMORY: errorString = "GL_OUT_OF_MEMORY"; break;
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: errorString = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: errorString = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
      default: errorString = "UNKNOWN_ERROR"; break;
    }
    ANDROID_LOG("OpenGL Error: %s in file %s at line %d\n", errorString, file, line);
  }
}

ToolKit::AndroidDevice* g_proxy = nullptr;
android_app* g_android_app      = nullptr;
AAssetManager* g_asset_manager  = nullptr;

namespace ToolKit
{
  class GameViewport : public Viewport
  {
  public:
      GameViewport()
      {
          m_contentAreaLocation = Vec2(0.0f);
      }
      GameViewport(float width, float height) :Viewport(width, height){}

      void ContentResize(float width, float height)
      {
          OnResizeContentArea(width, height);
      }

      void Update(float dt) override { }
  };

  void AndroidDevice::InitEGL()
  {
      // Choose your render attributes
      constexpr EGLint attribs[] = {
              EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
              EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
              EGL_BLUE_SIZE, 8,
              EGL_GREEN_SIZE, 8,
              EGL_RED_SIZE, 8,
              EGL_DEPTH_SIZE, 24,
              EGL_NONE
      };

      EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      eglInitialize(display, nullptr, nullptr);

      EGLint numConfigs;
      eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

      EGLConfig supportedConfigs[16]{};
      eglChooseConfig(display, attribs, supportedConfigs, numConfigs, &numConfigs);

      void* config = 0;
      for (int i = 0; i < numConfigs; i++)
      {
          EGLint red, green, blue, depth;
          config = supportedConfigs[i];
          if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
              && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
              && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
              && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

              if(red == 8 && green == 8 && blue == 8 && depth == 24)
              {
                  ANDROID_LOG( "Found config with %i, %i, %i, %i", red, green, blue, depth);
                  break;
              }
          }
      }

      ANDROID_LOG("Found: %i configs", numConfigs);
      // create the proper window surface
      EGLint format;
      eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
      EGLSurface surface = eglCreateWindowSurface(display, config, g_android_app->window, nullptr);

      // Create a GLES 3 context
      const EGLint contextAttribs[] = { EGL_CONTEXT_MAJOR_VERSION, 3,
                                        EGL_CONTEXT_MINOR_VERSION, 0,
                                        EGL_NONE, EGL_NONE };
      EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
      // get some window metrics
      EGLBoolean madeCurrent = eglMakeCurrent(display, surface, surface, context);
      assert(madeCurrent);

      display_ = display;
      surface_ = surface;
      context_ = context;
  }

  void AndroidDevice::DestroyRenderer()
  {
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);
      glDeleteBuffers(1, &EBO);

      eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      eglDestroyContext(display_, context_);
      eglDestroySurface(display_, surface_);
      eglTerminate(display_);
  }

  void AndroidDevice::CheckShaderError(GLuint shader)
  {
      GLint isCompiled = 0;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
      if(isCompiled == GL_FALSE)
      {
          GLint maxLength = 2048;
          char infoLog[2048];
          glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
          ANDROID_LOG("No compile vs %s", infoLog);
          glDeleteShader(shader);
          DestroyRenderer();
          assert(0);
      }
  }

  void AndroidDevice::InitRender()
  {
      const float vertices[] = {
              // positions      // texture coords
               1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // top right
               1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom right
              -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom left
              -1.0f,  1.0f, 0.0f,  0.0f, 1.0f  // top left
      };
      const unsigned int indices[] = {
              2, 1, 0, // first triangle
              2, 0, 3  // second triangle
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
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);
      // texture coord attribute
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
      glEnableVertexAttribArray(1);

      const char* vertexShader =
              "       #version 300 es\n"
              "       precision highp float;\n"
              "       layout(location = 0) in vec3 vPosition;\n"
              "       layout(location = 1) in vec2 vTexture;\n"
              "       out vec2 vTex;\n"
              "       void main()\n"
              "       {\n"
              "         gl_Position = vec4(vPosition, 1.0);\n"
              "         vTex = vTexture;\n"
              "       }\n";
      const char* fragmentShader =
              "       #version 300 es\n"
              "       precision highp float;\n"
              "       precision highp sampler2D;\n"
              "       in vec2 vTex;\n"
              "       out vec4 outFragColor;\n"
              "       uniform sampler2D tex;\n"
              "       void main()\n"
              "       {\n"
              "         vec3 color = texture(tex, vTex).rgb;\n"
              "         // vec3 color = vec3(0.7, 0.0, 0.0);\n"
              "         outFragColor = vec4(color, 1.0);\n"
              "       }\n";

      GLuint vs = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vs, 1, &vertexShader, 0);
      glCompileShader(vs);
      CheckShaderError(vs);

      GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fs, 1, &fragmentShader, 0);
      glCompileShader(fs);
      CheckShaderError(fs);

      shaderProgram = glCreateProgram();
      glAttachShader(shaderProgram, vs);
      glAttachShader(shaderProgram, fs);
      glLinkProgram(shaderProgram);
      GLint isLinked = 0;
      glGetProgramiv(shaderProgram, GL_LINK_STATUS, (int *)&isLinked);
      if (isLinked == GL_FALSE)
      {
          GLint maxLength = 0;
          glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
          char* infoLog = new char[maxLength];
          ANDROID_LOG("linker failed %s", infoLog);
          glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
          glDeleteProgram(shaderProgram);
          glDeleteShader(vs);
          glDeleteShader(fs);
          assert(0);
      }
  }

  void AndroidDevice::Render(GLuint tex)
  {
      GLint ct, cf, cp;
      glGetIntegerv(GL_TEXTURE_BINDING_2D, &ct);
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &cf);
      glGetIntegerv(GL_CURRENT_PROGRAM, &cp);
      CHECK_GL_ERROR();

      ToolKitMainWindowResize();
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      glValidateProgram(shaderProgram);
      GLint isValidated = 0;
      glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, (int *)&isValidated);
      if(isValidated == GL_FALSE)
      {
          GLint maxLength = 1024;
          glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
          char infoLog[1024]{};
          glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
          CHECK_GL_ERROR();
          assert(0);
      }

      glUseProgram(shaderProgram);
      GLint loc = glGetUniformLocation(shaderProgram, "tex");
      glUniform1i(loc, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex);
      CHECK_GL_ERROR();

      glBindVertexArray(VAO);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      CHECK_GL_ERROR();
      // Present the rendered image. This is an implicit glFlush.
      EGLBoolean swapResult = eglSwapBuffers(display_, surface_);
      assert(swapResult == EGL_TRUE);

      glBindVertexArray(0);
      glBindTexture(GL_TEXTURE_2D, ct);
      glBindFramebuffer(GL_FRAMEBUFFER, cf);
      glUseProgram(cp);
  }

  void AndroidDevice::ToolKitFrame()
  {
    static Timing timing{};
    timing.CurrentTime = GetElapsedMilliSeconds();
    timing.DeltaTime = timing.CurrentTime - timing.LastTime;

    m_game->Frame(timing.DeltaTime, gameViewport);
    Render(m_game->renderTarget->m_textureId);

    timing.FrameCount++;
    timing.TimeAccum += timing.DeltaTime;
    if (timing.TimeAccum >= 1000.0f)
    {
      timing.TimeAccum = timing.FrameCount = 0;
    }

    timing.LastTime = timing.CurrentTime;
  }

  inline bool FileHasExtension(const String& path, const String& extension)
  {
      String ext, name;
      DecomposePath(path, nullptr, nullptr, &ext);
      return ext == extension;
  }

  // copys all of the engine assets to internal data folder if already not coppied
  void AndroidDevice::CopyAllAssetsToDataPath()
  {
    String internalDataPath = g_android_app->activity->internalDataPath;
    using namespace std::filesystem;
    
    if (!exists(internalDataPath + "/Resources"))
    {
      if (create_directories(internalDataPath + "/Resources"))
      {
        ANDROID_LOG("created resources \n");
      }
    }

    String resourcesEngine = "/Resources/Engine";

    if (!exists(internalDataPath + resourcesEngine))
    {
      if (create_directories(internalDataPath + resourcesEngine))
      {
        ANDROID_LOG("created Resources/Engine\n");
      }
    }

    const int numDirectories = 6;
    String directories[numDirectories] { "/Fonts", "/Materials", "/Meshes", "/Scenes", "/Shaders", "/Textures" };
    AAssetManager* assetManager = g_android_app->activity->assetManager;
    std::vector<char> buffer;

    ANDROID_LOG(" traversing directories \n");
    for (int i = 0; i < numDirectories; i++)
    {
      if (!exists(internalDataPath + resourcesEngine + directories[i]))
      {
        create_directories(internalDataPath + resourcesEngine + directories[i]);
      }

      String assetDirectory = resourcesEngine + directories[i];
      assetDirectory.erase(assetDirectory.begin(), assetDirectory.begin() + 1); // delete first character '/'
      AAssetDir* dir = AAssetManager_openDir(assetManager, assetDirectory.c_str());

      ANDROID_LOG("asset directory: %s\n", assetDirectory.c_str());

      while (const char* file = AAssetDir_getNextFileName(dir))
      {
        String fileName = resourcesEngine + directories[i] + "/" + file;
        String filePath = internalDataPath + fileName;
        fileName.erase(fileName.begin(), fileName.begin() + 1); // delete first character '/'
        ANDROID_LOG("file: %s\n", filePath.c_str());

        if (exists(filePath))
        {
          if (FileHasExtension(filePath, ".shader"))
            std::filesystem::remove(filePath);
          else continue;
        }

        AAsset* asset = AAssetManager_open(assetManager, fileName.c_str(), 0);
        if (!asset)
        {
            ANDROID_LOG("cannot open asset! %s\n", fileName.c_str());
            continue;
        }
        FILE* fileHandle = fopen(filePath.c_str(), "wb");
        off_t size = AAsset_getLength(asset);
        if (buffer.size() < size) buffer.resize(size + 1, '\0');
        AAsset_read(asset, buffer.data(), size);
        fwrite(buffer.data(), 1, size + 1, fileHandle);
        memset(buffer.data(), 0, buffer.capacity());
        fclose(fileHandle);
        AAsset_close(asset);
      }
    }
  }

  void AndroidDevice::InitToolkit()
  {
      InitEGL();

      EGLint width, height;
      eglQuerySurface(display_, surface_, EGL_WIDTH, &width);
      eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);

      InitRender();
      // Init ToolKit
      Main* proxy = new Main();

      String internalDataPath = g_android_app->activity->internalDataPath;  
      // There should be a config folder for each export
      proxy->m_cfgPath = internalDataPath;
      proxy->m_resourceRoot = internalDataPath;
      Main::SetProxy(proxy);
      CopyAllAssetsToDataPath();

      proxy->PreInit();
      GetLogger()->SetWriteConsoleFn([](LogType lt, String ms) -> void { ANDROID_LOG("%s", ms.c_str()); });
      proxy->Init();

      proxy->m_renderSys->InitGl(nullptr, [](const std::string& msg) -> void
                                 {
                                     GetLogger()->WriteConsole(LogType::Memo, "tk opengl error: %s \n", msg.c_str());
                                 });

      gameViewport = new GameViewport((float)width, (float)height);
      m_game = new Game();
      m_game->Init(proxy);
  }
  
  void AndroidDevice::ToolKitMainWindowResize()
  {
    EGLint width, height;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);
  
    if (width != width_ || height != height_) 
    {
      width_ = width;
      height_ = height;
      glViewport(0, 0, width, height);
      gameViewport->ContentResize((float)width, (float)height);
    }
  }
  
  void AndroidDevice::DestroyToolKit(void* mainHandle)
  {
      ToolKit::Main* pToolKit = Main::GetInstance();
      DestroyRenderer();
      delete m_game;
      pToolKit->Uninit();
      pToolKit->PostUninit();
      delete pToolKit;
  }
}