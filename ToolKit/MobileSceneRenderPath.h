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
#include "FxaaPass.h"
#include "Pass.h"
#include "RenderSystem.h"
#include "SceneRenderPath.h"
#include "ShadowPass.h"
#include "SsaoPass.h"
#include "ToneMapPass.h"

namespace ToolKit
{
  /**
   * Mobile scene render path.
   */
  class TK_API MobileSceneRenderPath : public SceneRenderPath
  {
   public:
    MobileSceneRenderPath();
    explicit MobileSceneRenderPath(const SceneRenderPathParams& params);
    virtual ~MobileSceneRenderPath();

    void Render(Renderer* renderer) override;
    void PreRender(Renderer* renderer) override;
    void PostRender() override;

   protected:
    void SetPassParams() override;

   protected:
    RenderJobArray m_jobs;
    RenderJobArray m_opaqueJobs;
    RenderJobArray m_translucentJobs;
  };

  typedef std::shared_ptr<MobileSceneRenderPath> MobileSceneRenderPathPtr;
} // namespace ToolKit
