#include "../Codes/Game.h"

#include <stdio.h>
#include <chrono>
#include "Types.h"
#include "ToolKit.h"
#include "Common/SDLEventPool.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "emscripten.h"
#include "GLES3/gl3.h"

#include <iostream>

namespace ToolKit
{
    bool g_running = true;
    SDL_Window* g_window = nullptr;
    SDL_GLContext g_context = nullptr;
    Game* g_game = nullptr;
    Main* g_proxy = nullptr;

    // Setup.
    const char* appName = "ToolKit";
    const int width = 1280;
    const int height = 720;
    const uint fps = 120;

    class GameViewport : public Viewport
    {
    public:
      GameViewport()
			{
				m_contentAreaLocation = Vec2(0.0f);
			}
      GameViewport(float width, float height) :Viewport(width, height){}
      void Update(float dt) override
      {
        m_mouseOverContentArea = true;

        for (Event* e : Main::GetInstance()->m_eventPool)
        {
          if (e->m_type == Event::EventType::Mouse)
          {
            MouseEvent* me = static_cast<MouseEvent*> (e);
            if (me->m_action == EventAction::Move)
            {
              m_lastMousePosRelContentArea.x = me->absolute[0];
              m_lastMousePosRelContentArea.y = me->absolute[1];
              break;
            }
          }
        }
      }
    };

    Viewport* g_viewport = nullptr;
		int lastWidth = 0;
		int lastHeight = 0;
		void Resize(int w, int h)
		{
			if (g_viewport != nullptr && (lastWidth != w || lastHeight != h))
			{
				// TODO renderer does not come this way: GetRenderer()->m_windowSize = Vec2(w, h);
				g_viewport->m_wndContentAreaSize = Vec2(w, h);
			}
		}

    GLuint VAO, VBO, EBO;
    GLuint shaderProgram;
    void InitRender()
    {
      float vertices[] = {
              // positions      // texture coords
               1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // top right
               1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom right
              -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom left
              -1.0f,  1.0f, 0.0f,  0.0f, 1.0f  // top left
      };
      unsigned int indices[] = {
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
      "#version 300 es\n"
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
"       }\n"
;
      const char* fragmentShader = 
      "#version 300 es\n"
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
"       }\n"
;

      GLuint vs = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vs, 1, &vertexShader, 0);
      glCompileShader(vs);
      GLint isCompiled = 0;
      glGetShaderiv(vs, GL_COMPILE_STATUS, &isCompiled);
      if(isCompiled == GL_FALSE)
      {
        std::cout << "No compile vs\n";
        GLint maxLength = 0;
        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);
        char* infoLog = new char[maxLength];
        glGetShaderInfoLog(vs, maxLength, &maxLength, &infoLog[0]);
        std::cout << infoLog << "\n";
        glDeleteShader(vs);
      }

      GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fs, 1, &fragmentShader, 0);
      glCompileShader(fs);
      glGetShaderiv(fs, GL_COMPILE_STATUS, &isCompiled);
      if (isCompiled == GL_FALSE)
      {
        std::cout << "No compile fs\n";
        GLint maxLength = 0;
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);
        char* infoLog = new char[maxLength];
        glGetShaderInfoLog(fs, maxLength, &maxLength, &infoLog[0]);
        std::cout << infoLog << "\n";
        glDeleteShader(fs);
      }

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
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
        glDeleteProgram(shaderProgram);
        glDeleteShader(vs);
        glDeleteShader(fs);
      }
    }

    void Render(GLuint tex)
    {
      GLint ct,cf;
      glGetIntegerv(GL_TEXTURE_BINDING_2D, &ct);
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &cf);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glViewport(0,0,1280,720);
      
      //glClearColor(0.2, 0.2, 0.2, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      glValidateProgram(shaderProgram);
      GLint isValidated = 0;
      glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, (int *)&isValidated);
      if(isValidated == GL_FALSE)
      {
        GLint maxLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
        char* infoLog = new char[maxLength];
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
      }

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

    void Init()
    {
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
      {
        g_running = false;
      }
      else
      {
        SDL_GL_SetAttribute
        (
          SDL_GL_CONTEXT_PROFILE_MASK,
          SDL_GL_CONTEXT_PROFILE_ES
        );
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        g_window = SDL_CreateWindow
        (
          appName,
          SDL_WINDOWPOS_UNDEFINED,
          SDL_WINDOWPOS_UNDEFINED,
          width,
          height,
          SDL_WINDOW_OPENGL |
          SDL_WINDOW_RESIZABLE |
          SDL_WINDOW_SHOWN |
          SDL_WINDOW_ALLOW_HIGHDPI
        );

        const char* error =  SDL_GetError();
        std::cout << error << std::endl;

        if (g_window == nullptr)
        {
          g_running = false;
        }
        else
        {
          g_context = SDL_GL_CreateContext(g_window);

          const char* error =  SDL_GetError();
          std::cout << error << std::endl;

          if (g_context == nullptr)
          {
            g_running = false;
          }
          else
          {
            bool t = SDL_GL_ExtensionSupported("EXT_color_buffer_float");

            EM_ASM(

            );

            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);

            // Init ToolKit
            g_proxy = new Main();
            // There should be a config folder for each export
            g_proxy->m_cfgPath = ConcatPaths({".", "..", "Config"});
            g_proxy->m_resourceRoot = ConcatPaths({ ".", "..", "Resources" });
            Main::SetProxy(g_proxy);
            g_proxy->PreInit();

            GetLogger()->SetWriteConsoleFn([](LogType lt, String ms) -> void
            {
              GetLogger()->Log(ms);
            });

            g_proxy->Init();

            // Init game
            g_viewport = new GameViewport(1280.0f, 720.0f);
            g_game = new Game();
            g_game->Init(g_proxy);
						
						Resize(width, height);

            InitRender();
          }
        }
      }
    }

    float GetMilliSeconds()
    {
      static std::chrono::high_resolution_clock::time_point t1
      = std::chrono::high_resolution_clock::now();
      std::chrono::high_resolution_clock::time_point t2 =
      std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> time_span =
      std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

      return static_cast<float>(time_span.count() * 1000.0);
    }

    void Exit()
    {
      DeleteRender();
      
      SafeDel(g_game);

      g_proxy->Uninit();
      g_proxy->PostUninit();
      SafeDel(g_proxy);

      SDL_DestroyWindow(g_window);
      SDL_Quit();

      g_running = false;
    }

    void ProcessEvent(const SDL_Event& e)
    {
			if (e.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				Resize(e.window.data1, e.window.data2);
			}
    }

    void TK_Loop(void* args)
    {
      Timing* timer = static_cast<Timing*> (args);
      timer->Init(fps);

      {
        SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent))
        {
          PoolEvent(sdlEvent);
          ProcessEvent(sdlEvent);
        }

        timer->CurrentTime = GetMilliSeconds();
        if (timer->CurrentTime > timer->LastTime + timer->DeltaTime)
        {
          float ft = timer->CurrentTime - timer->LastTime;
					//std::cout << "FPS: " << 1000.0f / ft << std::endl;
					//std::cout << "MS: " << ft << std::endl;
          g_viewport->Update(ft);
          GetAnimationPlayer()->Update(MillisecToSec(ft));

          GLenum er = glGetError();
          if (er != 0)
          {
            int y = 0;
          }

          g_game->Frame(ft, g_viewport);

          // Render backbuffer
          Render(g_viewport->m_framebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0)->m_textureId);
	
          ClearPool();  // Clear after consumption.
          SDL_GL_SwapWindow(g_window);

          timer->FrameCount++;
          timer->TimeAccum += ft;
          if (timer->TimeAccum >= 1000.0f)
          {
            timer->TimeAccum = 0;
            timer->FrameCount = 0;
          }

          timer->LastTime = timer->CurrentTime;
        }
      }
    }

    int ToolKit_Main(int argc, char* argv[])
    {
      Init();

      static Timing timer;
      emscripten_set_main_loop_arg
      (
        TK_Loop,
        reinterpret_cast<void*> (&timer),
        0,
        1
      );

      Exit();
      return 0;
    }
}  // namespace ToolKit

// TODO call this later
extern "C" {
  void ResizeC(int width, int height)
  {
    ToolKit::Resize(width, height);
  }
}

int main(int argc, char* argv[])
{
  return ToolKit::ToolKit_Main(argc, argv);
}