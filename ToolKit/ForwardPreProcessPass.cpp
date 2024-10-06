/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

// Purpose of this pass is exporting forward depths and normals before SSAO pass

#include "ForwardPreProcessPass.h"

#include "Shader.h"
#include "stdafx.h"

namespace ToolKit
{

  ForwardPreProcess::ForwardPreProcess()
  {
    m_framebuffer            = MakeNewPtr<Framebuffer>();

    m_linearMaterial         = MakeNewPtr<Material>();
    ShaderPtr vertexShader   = GetShaderManager()->Create<Shader>(ShaderPath("forwardPreProcessVert.shader", true));
    ShaderPtr fragmentShader = GetShaderManager()->Create<Shader>(ShaderPath("forwardPreProcess.shader", true));
    m_linearMaterial->m_vertexShader   = vertexShader;
    m_linearMaterial->m_fragmentShader = fragmentShader;
    m_linearMaterial->Init();

    TextureSettings oneChannelSet = {};
    oneChannelSet.WarpS           = GraphicTypes::UVClampToEdge;
    oneChannelSet.WarpT           = GraphicTypes::UVClampToEdge;
    oneChannelSet.InternalFormat  = GraphicTypes::FormatRGBA16F;
    oneChannelSet.Format          = GraphicTypes::FormatRGBA;
    oneChannelSet.Type            = GraphicTypes::TypeFloat;
    oneChannelSet.GenerateMipMap  = false;
    m_normalRt                    = MakeNewPtr<RenderTarget>(128, 128, oneChannelSet);

    oneChannelSet.InternalFormat  = GraphicTypes::FormatRGBA32F;
    m_linearDepthRt               = MakeNewPtr<RenderTarget>(128, 128, oneChannelSet);
  }

  ForwardPreProcess::~ForwardPreProcess() {}

  void ForwardPreProcess::InitBuffers(int width, int height, int sampleCount)
  {
    const FramebufferSettings& fbs = m_framebuffer->GetSettings();
    bool requiresReconstruct = fbs.width != width || fbs.height != height || fbs.multiSampleFrameBuffer != sampleCount;

    m_framebuffer->ReconstructIfNeeded({width, height, false, false, sampleCount});
    m_normalRt->ReconstructIfNeeded(width, height);
    m_linearDepthRt->ReconstructIfNeeded(width, height);

    if (requiresReconstruct)
    {
      m_framebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_linearDepthRt);
      m_framebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment1, m_normalRt);
    }

    if (DepthTexturePtr depth = m_params.FrameBuffer->GetDepthTexture())
    {
      m_framebuffer->AttachDepthTexture(depth);
    }
  }

  void ForwardPreProcess::Render()
  {
    // Currently transparent objects are not rendered to export screen space normals or linear depth
    // we want SSAO and DOF to effect on opaque objects only renderLinearDepthAndNormalFn(m_params.TranslucentJobs);

    Renderer* renderer                      = GetRenderer();

    const auto renderLinearDepthAndNormalFn = [&](RenderJobItr begin, RenderJobItr end)
    {
      for (RenderJobItr job = begin; job != end; job++)
      {
        renderer->Render(*job);
      }
    };

    GpuProgramManager* gpuProgramManager = GetGpuProgramManager();

    RenderJobItr begin                   = m_params.renderData->GetForwardOpaqueBegin();
    RenderJobItr end                     = m_params.renderData->GetForwardAlphaMaskedBegin();

    m_linearMaterial->m_fragmentShader->SetDefine("EnableDiscardPixel", "0");
    m_program = gpuProgramManager->CreateProgram(m_linearMaterial->m_vertexShader, m_linearMaterial->m_fragmentShader);
    renderer->BindProgram(m_program);
    renderLinearDepthAndNormalFn(begin, end);

    begin = m_params.renderData->GetForwardAlphaMaskedBegin();
    end   = m_params.renderData->GetForwardTranslucentBegin();
    m_linearMaterial->m_fragmentShader->SetDefine("EnableDiscardPixel", "1");
    m_program = gpuProgramManager->CreateProgram(m_linearMaterial->m_vertexShader, m_linearMaterial->m_fragmentShader);
    renderer->BindProgram(m_program);
    renderLinearDepthAndNormalFn(begin, end);
  }

  void ForwardPreProcess::PreRender()
  {
    RenderPass::PreRender();

    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_framebuffer, GraphicBitFields::AllBits);
    renderer->SetCamera(m_params.Cam, true);
  }

  void ForwardPreProcess::InitDefaultDepthTexture(int width, int height)
  {
    if (m_depthTexture == nullptr)
    {
      m_depthTexture = MakeNewPtr<DepthTexture>();
      m_depthTexture->Init(width, height, false);
    }
  }

} // namespace ToolKit