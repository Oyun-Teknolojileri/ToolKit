#pragma once

#include "Pass.h"

namespace ToolKit
{

  struct ShadowPassParams
  {
    LightRawPtrArray Lights    = {};
    EntityRawPtrArray Entities = {};
  };

  /**
   * Create shadow map buffers for all given lights.
   */
  class TK_API ShadowPass : public Pass
  {
   public:
    ShadowPass();
    explicit ShadowPass(const ShadowPassParams& params);
    ~ShadowPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

    RenderTargetPtr GetShadowAtlas();

   private:
    void RenderShadowMaps(Light* light, const EntityRawPtrArray& entities);

    /**
     * Sets layer and coordinates of the shadow maps in shadow atlas.
     * @param lights Light array that have shadows.
     * @return number of layers needed.
     */
    int PlaceShadowMapsToShadowAtlas(const LightRawPtrArray& lights);

    /**
     * Creates a shadow atlas for m_params.Lights
     */
    void InitShadowAtlas();

   public:
    ShadowPassParams m_params;

   private:
    MaterialPtr m_prevOverrideMaterial = nullptr;
    FramebufferPtr m_prevFrameBuffer   = nullptr;
    MaterialPtr m_lastOverrideMat      = nullptr;
    const Vec4 m_shadowClearColor      = Vec4(1.0f);

    FramebufferPtr m_shadowFramebuffer = nullptr;
    RenderTargetPtr m_shadowAtlas      = nullptr;
    int m_layerCount = 0; // Number of textures in array texture (shadow atlas)
    EntityIdArray m_previousShadowCasters;
    std::vector<bool> m_clearedLayers;

    EntityRawPtrArray m_drawList;
    Quaternion m_cubeMapRotations[6];
    Vec3 m_cubeMapScales[6];

    BinPack2D m_packer;
  };

  typedef std::shared_ptr<ShadowPass> ShadowPassPtr;

} // namespace ToolKit
