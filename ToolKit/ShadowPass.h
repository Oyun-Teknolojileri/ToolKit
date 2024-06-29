/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "BinPack2D.h"
#include "Pass.h"

namespace ToolKit
{

  struct ShadowPassParams
  {
    ScenePtr scene       = nullptr;
    CameraPtr viewCamera = nullptr;
    LightPtrArray lights;
  };

  /** Create shadow map buffers for all given lights. */
  class TK_API ShadowPass : public Pass
  {
   public:
    ShadowPass();
    ShadowPass(const ShadowPassParams& params);
    ~ShadowPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

    RenderTargetPtr GetShadowAtlas();

   private:
    /** Perform all renderings to generate all shadow maps for the given light. */
    void RenderShadowMaps(LightPtr light);

    /** Performs a single render that generates a single shadow map of a cascade, or a face of a cube etc...*/
    void RenderShadowMap(LightPtr light, CameraPtr shadowCamera);

    /**
     * Sets layer and coordinates of the shadow maps in shadow atlas.
     * @param lights Light array that have shadows.
     * @return number of layers needed.
     */
    int PlaceShadowMapsToShadowAtlas(const LightPtrArray& lights);

    /** Creates a shadow atlas for m_params.Lights */
    void InitShadowAtlas();

   public:
    ShadowPassParams m_params;

   private:
    MaterialPtr m_shadowMatOrtho       = nullptr;
    MaterialPtr m_shadowMatPersp       = nullptr;

    const Vec4 m_shadowClearColor      = Vec4(1.0f);
    FramebufferPtr m_shadowFramebuffer = nullptr;
    RenderTargetPtr m_shadowAtlas      = nullptr;
    int m_layerCount                   = 0; // Number of textures in array texture (shadow atlas)
    int m_activeCascadeCount           = 0;
    bool m_useEVSM4                    = false;
    IDArray m_previousShadowCasters;

    Quaternion m_cubeMapRotations[6];
    BinPack2D m_packer;

    LightPtrArray m_lights; // Shadow casters in scene.
  };

  typedef std::shared_ptr<ShadowPass> ShadowPassPtr;

} // namespace ToolKit
