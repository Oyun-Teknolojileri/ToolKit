
#pragma once

typedef void* EGLSurface;
typedef void* EGLContext;
typedef void* EGLDisplay;
typedef int EGLint;
typedef unsigned int GLuint;

extern struct android_app* g_android_app;
extern struct AAssetManager* g_asset_manager;

namespace ToolKit
{
  class Main; 

  class AndroidDevice
  {
  public:
    void InitToolkit();
    void ToolKitFrame();
    void DestroyToolKit(void* mainHandle);
  private:
    void ToolKitMainWindowResize();
    float GetMiliSeconds();
    void CheckShaderError(unsigned int shader);
    void InitEGL();
    float GetMilliSeconds();
    void Render(GLuint tex);
    void InitRender();
    void DestroyRenderer();
    void CopyAllAssetsToDataPath();
  private:
    
    GLuint VAO, VBO, EBO;
    GLuint shaderProgram;
    
    EGLSurface surface_;
    EGLContext context_;
    EGLDisplay display_;
    EGLint width_  = -1;
    EGLint height_ = -1;
    class GameViewport* gameViewport;
    class Game* m_game;
  };
}
