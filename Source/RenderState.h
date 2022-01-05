#pragma once

#include "GL\glew.h"
#include "Serialize.h"

namespace ToolKit
{

  enum class BlendFunction
  {
    NONE,
    SRC_ALPHA_ONE_MINUS_SRC_ALPHA
  };

  enum class DrawType
  {
    Triangle = GL_TRIANGLES,
    Line = GL_LINES,
    LineStrip = GL_LINE_STRIP,
    LineLoop = GL_LINE_LOOP,
    Point = GL_POINTS
  };

  enum class CullingType
  {
    TwoSided, // No culling
    Front,
    Back
  };

  enum class VertexLayout
  {
    None,
    Mesh,
    SkinMesh
  };

  class TK_API RenderState : public Serializable
  {
  public:
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);

  public:
    CullingType cullMode = CullingType::Back;
    bool depthTestEnabled = true;
    BlendFunction blendFunction = BlendFunction::NONE;
    DrawType drawType = DrawType::Triangle;
    GLuint diffuseTexture = 0;
    bool diffuseTextureInUse = false;
    GLuint cubeMap = 0;
    bool cubeMapInUse = false;
    float lineWidth = 1.0f;
    VertexLayout vertexLayout = VertexLayout::None;
  };

}
