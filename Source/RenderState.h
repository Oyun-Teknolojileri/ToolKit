#pragma once

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
    Triangle = (int)GraphicTypes::DrawTypeTriangle,
    Line = (int)GraphicTypes::DrawTypeLines,
    LineStrip = (int)GraphicTypes::DrawTypeLineStrip,
    LineLoop = (int)GraphicTypes::DrawTypeLineLoop,
    Point = (int)GraphicTypes::DrawTypePoints
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
    uint diffuseTexture = 0;
    bool diffuseTextureInUse = false;
    uint cubeMap = 0;
    bool cubeMapInUse = false;
    float lineWidth = 1.0f;
    VertexLayout vertexLayout = VertexLayout::None;
  };

}
