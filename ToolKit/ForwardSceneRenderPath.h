/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "BloomPass.h"
#include "CubemapPass.h"
#include "EngineSettings.h"
#include "ForwardPass.h"
#include "ForwardPreProcessPass.h"
#include "Pass.h"
#include "RenderSystem.h"
#include "DeferredSceneRenderPath.h"
#include "ShadowPass.h"
#include "SsaoPass.h"

namespace ToolKit
{
  /**
   * Forward scene render path. All objects are drawn in forward manner. Bandwidth optimized.
   */
  class TK_API ForwardSceneRenderPath : public DeferredSceneRenderPath
  {
   public:
    ForwardSceneRenderPath();
    explicit ForwardSceneRenderPath(const SceneRenderPathParams& params);
    virtual ~ForwardSceneRenderPath();

    void Render(Renderer* renderer) override;
    void PreRender(Renderer* renderer) override;
    void PostRender(Renderer* renderer) override;

   protected:
    void SetPassParams() override;
  };

  typedef std::shared_ptr<ForwardSceneRenderPath> MobileSceneRenderPathPtr;
} // namespace ToolKit
