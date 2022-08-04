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
    Triangle = static_cast<int> (GraphicTypes::DrawTypeTriangle),
    Line = static_cast<int> (GraphicTypes::DrawTypeLines),
    LineStrip = static_cast<int> (GraphicTypes::DrawTypeLineStrip),
    LineLoop = static_cast<int> (GraphicTypes::DrawTypeLineLoop),
    Point = static_cast<int> (GraphicTypes::DrawTypePoints)
  };

  enum class CullingType
  {
    TwoSided,  // No culling
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
    bool IBLInUse = false;
    float iblIntensity = 0.25f;
    uint irradianceMap = 0;
    int priority = 0;  // The higher the priority, the earlier to draw.
  };

}  // namespace ToolKit
