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
    CullingType cullMode                  = CullingType::Back;
    BlendFunction blendFunction           = BlendFunction::NONE;
    DrawType drawType                     = DrawType::Triangle;
    float alphaMaskTreshold               = 0.001f;
    float lineWidth                       = 1.0f;

    bool depthTestEnabled                 = true;
    GraphicCompareFunctions depthFunction = GraphicCompareFunctions::FuncLess;
    uint cubeMap                          = 0;
    bool cubeMapInUse                     = false;
    // The higher the priority, the earlier to draw.
    int priority                          = 0;

    /* ONLY Renderer class edits and uses this variable. This variable does not
     * give any functionality to disable or enable ibl for material.*/
    bool IBLInUse                         = false;

    float iblIntensity                    = 0.25f;
    uint irradianceMap                    = 0;
    uint preFilteredSpecularMap           = 0;
    uint brdfLut                          = 0;
    bool AOInUse                          = true;
    // Force material to be drawn with forward pass.
    bool useForwardPath                   = false;
  };

} // namespace ToolKit
