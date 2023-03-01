#pragma once

#include "Serialize.h"

namespace ToolKit
{

  enum class GraphicBitFields
  {
    ColorBits        = 0x00004000,
    DepthBits        = 0x00000100,
    StencilBits      = 0x00000400,
    ColorDepthBits   = ColorBits | DepthBits,
    ColorStencilBits = ColorBits | StencilBits,
    DepthStencilBits = DepthBits | StencilBits,
    AllBits          = ColorBits | DepthBits | StencilBits
  };

  enum class CompareFunctions
  {
    FuncNever   = 0x0200,
    FuncLess    = 0x0201,
    FuncEqual   = 0x0202,
    FuncLequal  = 0x0203,
    FuncGreater = 0x0204,
    FuncNEqual  = 0x0205,
    FuncGEqual  = 0x0206,
    FuncAlways  = 0x0207
  };

  enum class BlendFunction
  {
    NONE,
    SRC_ALPHA_ONE_MINUS_SRC_ALPHA,
    ALPHA_MASK,
    ONE_TO_ONE
  };

  enum class DrawType
  {
    Point     = 0x0000,
    Line      = 0x0001,
    LineLoop  = 0x0002,
    LineStrip = 0x0003,
    Triangle  = 0x0004
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
    // Active state values.
    // Changing these settings will modify the renderer's state.
    CullingType cullMode        = CullingType::Back;
    BlendFunction blendFunction = BlendFunction::NONE;
    DrawType drawType           = DrawType::Triangle;
    float alphaMaskTreshold     = 0.001f;
    float lineWidth             = 1.0f;

   private:
    friend class Renderer;

    // Passive state values.
    // Renderer changes or updates these values.
    bool depthTestEnabled                 = true;
    CompareFunctions depthFunction        = CompareFunctions::FuncLess;

    /* ONLY Renderer class edits and uses this variable. This variable does not
     * give any functionality to disable or enable ibl for material.*/
    bool IBLInUse                         = false;

    float iblIntensity                    = 0.25f;
    uint irradianceMap                    = 0;
    uint preFilteredSpecularMap           = 0;
    uint brdfLut                          = 0;
  };

} // namespace ToolKit
