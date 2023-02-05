#include "PostProcessPass.h"

#include "Shader.h"
#include "ShaderReflectionCache.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  PostProcessPass::PostProcessPass()
  {
    m_copyTexture = std::make_shared<RenderTarget>();
    m_copyBuffer  = std::make_shared<Framebuffer>();
    m_copyBuffer->Init({0, 0, false, false});

    m_postProcessPass = std::make_shared<FullQuadPass>();
  }

  PostProcessPass::PostProcessPass(const PostProcessPassParams& params)
      : PostProcessPass()
  {
    m_params = params;
  }

  PostProcessPass::~PostProcessPass()
  {
    m_postProcessShader = nullptr;
    m_postProcessPass   = nullptr;
    m_copyBuffer        = nullptr;
    m_copyTexture       = nullptr;
  }

  void PostProcessPass::PreRender()
  {
    Pass::PreRender();

    Renderer* renderer = GetRenderer();

    // Initiate copy buffer.
    FramebufferSettings fbs;
    fbs.depthStencil    = false;
    fbs.useDefaultDepth = false;
    if (m_params.FrameBuffer == nullptr)
    {
      fbs.width  = renderer->m_windowSize.x;
      fbs.height = renderer->m_windowSize.y;
    }
    else
    {
      FramebufferSettings targetFbs = m_params.FrameBuffer->GetSettings();
      fbs.width                     = targetFbs.width;
      fbs.height                    = targetFbs.height;
    }

    m_copyTexture->ReconstructIfNeeded(fbs.width, fbs.height);
    m_copyBuffer->ReconstructIfNeeded(fbs.width, fbs.height);
    m_copyBuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                m_copyTexture);

    // Copy given buffer.
    renderer->CopyFrameBuffer(m_params.FrameBuffer,
                              m_copyBuffer,
                              GraphicBitFields::ColorBits);

    // Set given buffer as a texture to be read in gamma pass.
    renderer->SetTexture(0, m_copyTexture->m_textureId);

    m_postProcessPass->m_params.FragmentShader   = m_postProcessShader;
    m_postProcessPass->m_params.FrameBuffer      = m_params.FrameBuffer;
    m_postProcessPass->m_params.ClearFrameBuffer = false;
  }

  void PostProcessPass::Render() { RenderSubPass(m_postProcessPass); }

  void PostProcessPass::PostRender() { Pass::PostRender(); }

} // namespace ToolKit