#pragma once

#include "Serialize.h"

namespace ToolKit
{

  enum class BlendFunction
  {
    NONE,
    SRC_ALPHA_ONE_MINUS_SRC_ALPHA,
    ALPHA_MASK,
    ONE_TO_ONE
  };

  enum class DrawType
  {
    Triangle  = static_cast<int>(GraphicTypes::DrawTypeTriangle),
    Line      = static_cast<int>(GraphicTypes::DrawTypeLines),
    LineStrip = static_cast<int>(GraphicTypes::DrawTypeLineStrip),
    LineLoop  = static_cast<int>(GraphicTypes::DrawTypeLineLoop),
    Point     = static_cast<int>(GraphicTypes::DrawTypePoints)
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
    CullingType cullMode        = CullingType::Back;
    bool depthTestEnabled       = true;
    GraphicTypes depthFunction  = GraphicTypes::FuncLess;
    BlendFunction blendFunction = BlendFunction::NONE;
    float alphaMaskTreshold     = 0.001f;
    DrawType drawType           = DrawType::Triangle;
    uint diffuseTexture         = 0;
    bool diffuseTextureInUse    = false;
    uint cubeMap                = 0;
    bool cubeMapInUse           = false;
    float lineWidth             = 1.0f;
    VertexLayout vertexLayout   = VertexLayout::None;

    /* ONLY Renderer class edits and uses this variable. This variable does not
     * give any functionality to disable or enable ibl for material.*/
    bool IBLInUse               = false;
    float iblIntensity          = 0.25f;
    uint irradianceMap          = 0;
    bool AOInUse                = true;
    int priority = 0; // The higher the priority, the earlier to draw.
    bool useForwardPath =
        false; // Force material to be drawn with forward pass.
    bool isColorMaterial      = true;
    bool emissiveTextureInUse = false;
    uint emissiveTexture      = 0;
  };

} // namespace ToolKit
