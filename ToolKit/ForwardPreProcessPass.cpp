/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ForwardPreProcessPass.h"

#include "Shader.h"
#include "stdafx.h"

namespace ToolKit
{

  ForwardPreProcess::ForwardPreProcess()
  {
    ShaderPtr vertexShader   = GetShaderManager()->Create<Shader>(ShaderPath("forwardPreProcessVert.shader", true));
    ShaderPtr fragmentShader = GetShaderManager()->Create<Shader>(ShaderPath("forwardPreProcess.shader", true));

    m_framebuffer            = std::make_shared<Framebuffer>();
    m_linearMaterial         = MakeNewPtr<Material>();
    m_linearMaterial->m_vertexShader   = vertexShader;
    m_linearMaterial->m_fragmentShader = fragmentShader;
    m_linearMaterial->Init();
  }

  ForwardPreProcess::~ForwardPreProcess() {}

  void ForwardPreProcess::Render()
  {
    Renderer* renderer                      = GetRenderer();

    const auto renderLinearDepthAndNormalFn = [this, renderer](RenderJobArray& renderJobArray)
    {
      for (RenderJob& job : renderJobArray)
      {
        MaterialPtr activeMaterial    = job.Material;
        RenderState* renderstate      = activeMaterial->GetRenderState();
        BlendFunction beforeBlendFunc = renderstate->blendFunction;
        renderstate->blendFunction    = BlendFunction::NONE;
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

  void ForwardPreProcess::PreRender()
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

    using FAttachment                 = Framebuffer::Attachment;

    m_framebuffer->SetAttachment(FAttachment::ColorAttachment0, m_params.gLinearRt);
    m_framebuffer->SetAttachment(FAttachment::ColorAttachment1, m_params.gNormalRt);
    m_framebuffer->AttachDepthTexture(m_params.gFrameBuffer->GetDepthTexture());

    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_framebuffer, false);
    renderer->SetCameraLens(m_params.Cam);
  }

  void ForwardPreProcess::PostRender() { RenderPass::PostRender(); }

} // namespace ToolKit
