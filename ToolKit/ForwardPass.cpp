/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "ForwardPass.h"

#include "EngineSettings.h"
#include "Material.h"
#include "Mesh.h"
#include "Pass.h"
#include "Shader.h"
#include "ToolKit.h"

namespace ToolKit
{

  ForwardRenderPass::ForwardRenderPass() {}

  ForwardRenderPass::ForwardRenderPass(const ForwardRenderPassParams& params) : m_params(params)
  {
    // Create a default frame buffer.
    if (m_params.FrameBuffer == nullptr)
    {
      m_params.FrameBuffer = MakeNewPtr<Framebuffer>();
      m_params.FrameBuffer->Init({1024, 768, false, true, GetEngineSettings().Graphics.msaa});
    }
  }

  void ForwardRenderPass::Render()
  {
    RenderOpaque(m_params.renderData);
    RenderTranslucent(m_params.renderData);
  }

  ForwardRenderPass::~ForwardRenderPass() {}

  void ForwardRenderPass::PreRender()
  {
    Pass::PreRender();

    // Set self data.
    Renderer* renderer = GetRenderer();

    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.clearBuffer);
    renderer->SetCamera(m_params.Cam, true);

    if (m_params.hasForwardPrePass)
    {
      renderer->SetDepthTestFunc(CompareFunctions::FuncLequal);
    }
  }

  void ForwardRenderPass::PostRender()
  {
    Pass::PostRender();
    Renderer* renderer = GetRenderer();

    if (m_params.hasForwardPrePass)
    {
      renderer->SetDepthTestFunc(CompareFunctions::FuncLess);
    }
  }

  void ForwardRenderPass::RenderOpaque(RenderData* renderData)
  {
    const MaterialPtr mat = GetMaterialManager()->GetDefaultMaterial();
    mat->m_fragmentShader->SetDefine("EnableDiscardPixel", "0");

    EngineSettings::GraphicSettings& graphicsSettings = GetEngineSettings().Graphics;
    mat->m_fragmentShader->SetDefine("EVSM4", graphicsSettings.useEVSM4 ? "1" : "0");
    mat->m_fragmentShader->SetDefine("SMFormat16Bit", graphicsSettings.use32BitShadowMap ? "0" : "1");

    GpuProgramPtr gpuProgram = GetGpuProgramManager()->CreateProgram(mat->m_vertexShader, mat->m_fragmentShader);

    RenderJobItr begin       = renderData->GetForwardOpaqueBegin();
    RenderJobItr end         = renderData->GetForwardAlphaMaskedBegin();
    RenderOpaqueHelper(renderData, begin, end, gpuProgram);

    mat->m_fragmentShader->SetDefine("EnableDiscardPixel", "1");
    gpuProgram = GetGpuProgramManager()->CreateProgram(mat->m_vertexShader, mat->m_fragmentShader);

    begin      = renderData->GetForwardAlphaMaskedBegin();
    end        = renderData->GetForwardTranslucentBegin();
    RenderOpaqueHelper(renderData, begin, end, gpuProgram);

    mat->m_fragmentShader->SetDefine("EnableDiscardPixel", "0");
  }

  void ForwardRenderPass::RenderTranslucent(RenderData* renderData)
  {
    RenderJobItr begin = renderData->GetForwardTranslucentBegin();
    RenderJobItr end   = renderData->jobs.end();

    RenderJobProcessor::SortByDistanceToCamera(begin, end, m_params.Cam);

    Renderer* renderer = GetRenderer();
    auto renderFnc     = [&](RenderJob& job)
    {
      Material* mat = job.Material;
      if (mat->GetRenderState()->cullMode == CullingType::TwoSided)
      {
        mat->GetRenderState()->cullMode = CullingType::Front;
        renderer->Render(job);

        mat->GetRenderState()->cullMode = CullingType::Back;
        renderer->Render(job);

        mat->GetRenderState()->cullMode = CullingType::TwoSided;
      }
      else
      {
        renderer->Render(job);
      }
    };

    const MaterialPtr mat                             = GetMaterialManager()->GetDefaultMaterial();

    EngineSettings::GraphicSettings& graphicsSettings = GetEngineSettings().Graphics;
    mat->m_fragmentShader->SetDefine("EVSM4", graphicsSettings.useEVSM4 ? "1" : "0");

    GpuProgramPtr program = GetGpuProgramManager()->CreateProgram(mat->m_vertexShader, mat->m_fragmentShader);

    if (begin != end)
    {
      renderer->EnableDepthWrite(false);
      for (RenderJobArray::iterator job = begin; job != end; job++)
      {
        if (job->Material->m_isShaderMaterial)
        {
          renderer->BindProgramOfMaterial(job->Material);
        }
        else
        {
          renderer->BindProgram(program);
        }
        renderFnc(*job);
      }
      renderer->EnableDepthWrite(true);
    }
  }

  void ForwardRenderPass::RenderOpaqueHelper(RenderData* renderData,
                                             RenderJobItr begin,
                                             RenderJobItr end,
                                             GpuProgramPtr defaultGpuProgram)
  {
    Renderer* renderer = GetRenderer();

    renderer->SetAmbientOcclusionTexture(m_params.SsaoTexture);

    for (RenderJobItr job = begin; job != end; job++)
    {
      if (job->Material->m_isShaderMaterial)
      {
        renderer->RenderWithProgramFromMaterial(*job);
      }
      else
      {
        renderer->BindProgram(defaultGpuProgram);
        renderer->Render(*job);
      }
    }
  }

} // namespace ToolKit