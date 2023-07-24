#include "ForwardLinearDepthPass.h"

#include "stdafx.h"

namespace ToolKit
{

  ForwardLinearDepth::ForwardLinearDepth()
  {
    ShaderPtr vertexShader   = GetShaderManager()->Create<Shader>(ShaderPath("forwardLinearDepthVert.shader", true));
    ShaderPtr fragmentShader = GetShaderManager()->Create<Shader>(ShaderPath("forwardLinearDepth.shader", true));

    m_normalMergeRt  = std::make_shared<RenderTarget>();
    m_framebuffer    = std::make_shared<Framebuffer>();
    m_linearMaterial = std::make_shared<Material>();
    m_linearMaterial->m_vertexShader   = vertexShader;
    m_linearMaterial->m_fragmentShader = fragmentShader;
    m_linearMaterial->Init();
  }

  ForwardLinearDepth::~ForwardLinearDepth() {}

  void ForwardLinearDepth::Render()
  {
    Renderer* renderer = GetRenderer();
    renderer->CopyTexture(m_params.gNormalRt, m_normalMergeRt);

    const auto renderLinearDepthAndNormalFn = [this, renderer](RenderJobArray& renderJobArray) 
    {
      for (RenderJob& job : renderJobArray)
      {
        MaterialPtr activeMaterial = job.Material;
        RenderState* renderstate   = activeMaterial->GetRenderState();
        BlendFunction beforeBlendFunc = renderstate->blendFunction;
        renderstate->blendFunction = BlendFunction::NONE;
        m_linearMaterial->SetRenderState(renderstate);
        m_linearMaterial->UnInit();

        renderer->m_overrideMat = m_linearMaterial;
        renderer->Render(job, m_params.Cam, {});
        renderstate->blendFunction = beforeBlendFunc;
      }
    };
    
    renderLinearDepthAndNormalFn(m_params.OpaqueJobs); 
    renderLinearDepthAndNormalFn(m_params.TranslucentJobs); 
  }

  void ForwardLinearDepth::PreRender()
  {
    RenderPass::PreRender();

    uint width  = (uint) m_params.gLinearRt->m_width;
    uint height = (uint) m_params.gLinearRt->m_height;

    m_framebuffer->Init({width, height, false, false});
    
    RenderTargetSettigs oneChannelSet = {};
    oneChannelSet.WarpS               = GraphicTypes::UVClampToEdge;
    oneChannelSet.WarpT               = GraphicTypes::UVClampToEdge;
    oneChannelSet.InternalFormat      = GraphicTypes::FormatRGB32F;
    oneChannelSet.Format              = GraphicTypes::FormatRGB;
    oneChannelSet.Type                = GraphicTypes::TypeFloat;

    // Init ssao texture
    m_normalMergeRt->m_settings       = oneChannelSet;
    m_normalMergeRt->ReconstructIfNeeded((uint) width, (uint) height);

    m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0, m_params.gLinearRt);
    m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment1, m_normalMergeRt);
    
    m_framebuffer->SetDepthFromOther(m_params.gFrameBuffer);

    Renderer* renderer = GetRenderer();
    renderer->ResetTextureSlots();
    renderer->SetFramebuffer(m_framebuffer, false);
    renderer->SetCameraLens(m_params.Cam);
  }

  void ForwardLinearDepth::PostRender() { RenderPass::PostRender(); }

} // namespace ToolKit
