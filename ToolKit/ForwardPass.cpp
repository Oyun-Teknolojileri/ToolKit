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

  ForwardRenderPass::ForwardRenderPass() : Pass("ForwardRenderPass")
  {
    EngineSettings::GraphicSettings& graphicsSettings = GetEngineSettings().Graphics;
    m_EVSM4                                           = graphicsSettings.useEVSM4;
    m_SMFormat16Bit                                   = !graphicsSettings.use32BitShadowMap;

    m_programConfigMat                                = GetMaterialManager()->GetCopyOfDefaultMaterial();

    m_programConfigMat->m_fragmentShader->SetDefine("EVSM4", std::to_string(m_EVSM4));
    m_programConfigMat->m_fragmentShader->SetDefine("SMFormat16Bit", std::to_string(m_SMFormat16Bit));
  }

  void ForwardRenderPass::Render()
  {
    RenderOpaque(m_params.renderData);
    RenderTranslucent(m_params.renderData);
  }

  void ForwardRenderPass::PreRender()
  {
    Pass::PreRender();

    // Set self data.
    Renderer* renderer = GetRenderer();

    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.clearBuffer);
    renderer->SetCamera(m_params.Cam, true);

    // Adjust the depth test considering z-pre pass.
    if (m_params.hasForwardPrePass)
    {
      // This is the optimal flag if the depth buffer is filled.
      // Only the visible fragments will pass the test.
      renderer->SetDepthTestFunc(CompareFunctions::FuncEqual);
    }
    else
    {
      renderer->SetDepthTestFunc(CompareFunctions::FuncLess);
    }
  }

  void ForwardRenderPass::PostRender()
  {
    Pass::PostRender();

    // Set the default depth test.
    Renderer* renderer = GetRenderer();
    renderer->SetDepthTestFunc(CompareFunctions::FuncLess);
  }

  void ForwardRenderPass::RenderOpaque(RenderData* renderData)
  {
    // Adjust program configuration.
    ConfigureProgram();

    // Render opaque.
    m_programConfigMat->m_fragmentShader->SetDefine("DrawAlphaMasked", "0");
    GpuProgramPtr gpuProgram =
        GetGpuProgramManager()->CreateProgram(m_programConfigMat->m_vertexShader, m_programConfigMat->m_fragmentShader);

    RenderJobItr begin = renderData->GetForwardOpaqueBegin();
    RenderJobItr end   = renderData->GetForwardAlphaMaskedBegin();
    RenderOpaqueHelper(renderData, begin, end, gpuProgram);

    // Render alpha masked.
    m_programConfigMat->m_fragmentShader->SetDefine("DrawAlphaMasked", "1");
    gpuProgram =
        GetGpuProgramManager()->CreateProgram(m_programConfigMat->m_vertexShader, m_programConfigMat->m_fragmentShader);

    begin = renderData->GetForwardAlphaMaskedBegin();
    end   = renderData->GetForwardTranslucentBegin();
    RenderOpaqueHelper(renderData, begin, end, gpuProgram);
  }

  void ForwardRenderPass::RenderTranslucent(RenderData* renderData)
  {
    ConfigureProgram();

    m_programConfigMat->m_fragmentShader->SetDefine("DrawAlphaMasked", "0");

    GpuProgramPtr program =
        GetGpuProgramManager()->CreateProgram(m_programConfigMat->m_vertexShader, m_programConfigMat->m_fragmentShader);

    Renderer* renderer = GetRenderer();
    renderer->BindProgram(program);

    RenderJobItr begin = renderData->GetForwardTranslucentBegin();
    RenderJobItr end   = renderData->jobs.end();
    RenderJobProcessor::SortByDistanceToCamera(begin, end, m_params.Cam);

    if (begin != end)
    {
      renderer->SetDepthTestFunc(CompareFunctions::FuncLess);
      renderer->EnableDepthWrite(false);
      for (RenderJobArray::iterator job = begin; job != end; job++)
      {
        if (job->Material->IsShaderMaterial())
        {
          renderer->RenderWithProgramFromMaterial(*job);
        }
        else
        {
          renderer->BindProgram(program);

          Material* mat = job->Material;
          if (mat->GetRenderState()->cullMode == CullingType::TwoSided)
          {
            mat->GetRenderState()->cullMode = CullingType::Front;
            renderer->Render(*job);

            mat->GetRenderState()->cullMode = CullingType::Back;
            renderer->Render(*job);

            mat->GetRenderState()->cullMode = CullingType::TwoSided;
          }
          else
          {
            renderer->Render(*job);
          }
        }
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
      if (job->Material->IsShaderMaterial())
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

  void ForwardRenderPass::ConfigureProgram()
  {
    EngineSettings::GraphicSettings& graphicsSettings = GetEngineSettings().Graphics;
    if (graphicsSettings.useEVSM4 != m_EVSM4)
    {
      m_EVSM4 = graphicsSettings.useEVSM4;
      m_programConfigMat->m_fragmentShader->SetDefine("EVSM4", std::to_string(m_EVSM4));
    }

    bool is16Bit = !graphicsSettings.use32BitShadowMap;
    if (is16Bit != m_SMFormat16Bit)
    {
      m_SMFormat16Bit = is16Bit;
      m_programConfigMat->m_fragmentShader->SetDefine("SMFormat16Bit", std::to_string(is16Bit));
    }
  }

} // namespace ToolKit