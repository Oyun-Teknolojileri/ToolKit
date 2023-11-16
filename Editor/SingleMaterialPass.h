/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "ForwardPass.h"

namespace ToolKit
{
  namespace Editor
  {

    struct SingleMatForwardRenderPassParams
    {
      ForwardRenderPassParams ForwardParams;
      ShaderPtr OverrideFragmentShader;
    };

    // Render whole scene in forward renderer with a single override material
    struct SingleMatForwardRenderPass : public ForwardRenderPass
    {
     public:
      SingleMatForwardRenderPass();
      explicit SingleMatForwardRenderPass(const SingleMatForwardRenderPassParams& params);

      void Render() override;
      void PreRender() override;

     public:
      SingleMatForwardRenderPassParams m_params;

     private:
      MaterialPtr m_overrideMat = nullptr;
    };

    typedef std::shared_ptr<SingleMatForwardRenderPass> SingleMatForwardRenderPassPtr;

  } // namespace Editor
} // namespace ToolKit