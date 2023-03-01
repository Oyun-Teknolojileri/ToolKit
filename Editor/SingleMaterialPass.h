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
      explicit SingleMatForwardRenderPass(
          const SingleMatForwardRenderPassParams& params);

      void Render() override;
      void PreRender() override;

     public:
      SingleMatForwardRenderPassParams m_params;

     private:
      MaterialPtr m_overrideMat = nullptr;
    };

    typedef std::shared_ptr<SingleMatForwardRenderPass>
        SingleMatForwardRenderPassPtr;

  } // namespace Editor
} // namespace ToolKit