#include "OutlinePass.h"

#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"

namespace ToolKit
{

  OutlinePass::OutlinePass()
  {
    m_stencilPass  = std::make_shared<StencilRenderPass>();
    m_stencilAsRt  = std::make_shared<RenderTarget>();

    m_outlinePass  = std::make_shared<FullQuadPass>();
    m_dilateShader = GetShaderManager()->Create<Shader>(
        ShaderPath("dilateFrag.shader", true));
  }

  OutlinePass::OutlinePass(const OutlinePassParams& params) : OutlinePass()
  {
    m_params = params;
  }

  OutlinePass::~OutlinePass()
  {
    m_stencilPass  = nullptr;
    m_outlinePass  = nullptr;
    m_dilateShader = nullptr;
    m_stencilAsRt  = nullptr;
  }

  void OutlinePass::Render()
  {
    // Generate stencil binary image.
    RenderSubPass(m_stencilPass);

    // Use stencil output as input to the dilation.
    GetRenderer()->SetTexture(0, m_stencilAsRt->m_textureId);
    m_dilateShader->SetShaderParameter("Color",
                                       ParameterVariant(m_params.OutlineColor));

    // Draw outline to the viewport.
    m_outlinePass->m_params.FragmentShader   = m_dilateShader;
    m_outlinePass->m_params.FrameBuffer      = m_params.FrameBuffer;
    m_outlinePass->m_params.ClearFrameBuffer = false;

    RenderSubPass(m_outlinePass);
  }

  void OutlinePass::PreRender()
  {
    Pass::PreRender();

    // Create stencil image.
    m_stencilPass->m_params.Camera   = m_params.Camera;
    m_stencilPass->m_params.RenderJobs = m_params.RenderJobs;

    // Construct output target.
    FramebufferSettings fbs          = m_params.FrameBuffer->GetSettings();
    m_stencilAsRt->ReconstructIfNeeded(fbs.width, fbs.height);
    m_stencilPass->m_params.OutputTarget = m_stencilAsRt;
  }

  void OutlinePass::PostRender() { Pass::PostRender(); }

} // namespace ToolKit