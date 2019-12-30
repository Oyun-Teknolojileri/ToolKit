#pragma once

#include "GL\glew.h"

namespace ToolKit
{

  enum BlendFunction
  {
    NONE,
    SRC_ALPHA_ONE_MINUS_SRC_ALPHA
  };

  enum DrawType
  {
    //Quad = GL_QUADS,
    Triangle = GL_TRIANGLES,
    Line = GL_LINES,
    Point = GL_POINTS
  };

  struct RenderState
  {
    bool backCullingEnabled = true;
    bool depthTestEnabled = true;
    BlendFunction blendFunction = NONE;
    DrawType drawType = Triangle;
    GLuint diffuseTexture = 0;
    bool diffuseTextureInUse = false;
    GLuint cubeMap = 0;
    bool cubeMapInUse = false;
  };

}
