//
// Created by Administrator on 8/30/2023.
//

#ifndef TOOLKITGAME_TOOLKITMAIN_H
#define TOOLKITGAME_TOOLKITMAIN_H

#endif //TOOLKITGAME_TOOLKITMAIN_H

typedef void* EGLSurface;
typedef void* EGLContext;
typedef void* EGLDisplay;
typedef int EGLint;
typedef unsigned int GLuint;

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
  };
}

