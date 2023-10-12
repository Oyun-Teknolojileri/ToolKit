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

#include "SingleMaterialPass.h"

#include "Material.h"

#include "DebugNew.h"

#define NOMINMAX
#include "nvtx3.hpp"
#undef WriteConsole

namespace ToolKit
{
  namespace Editor
  {

    SingleMatForwardRenderPass::SingleMatForwardRenderPass() : ForwardRenderPass()
    {
      m_overrideMat = MakeNewPtr<Material>();
    }

    SingleMatForwardRenderPass::SingleMatForwardRenderPass(const SingleMatForwardRenderPassParams& params)
        : SingleMatForwardRenderPass()
    {
      m_params = params;
    }

    void SingleMatForwardRenderPass::Render()
    {
      nvtxRangePushA("SingleMatForwardRenderPass Render");

      Renderer* renderer      = GetRenderer();
      renderer->m_overrideMat = MakeNewPtr<Material>();
      for (RenderJob& job : m_params.ForwardParams.OpaqueJobs)
      {
        LightPtrArray lightList = RenderJobProcessor::SortLights(job, m_params.ForwardParams.Lights);

        MaterialPtr mat         = job.Material;
        renderer->m_overrideMat->SetRenderState(mat->GetRenderState());
        renderer->m_overrideMat->m_vertexShader    = mat->m_vertexShader;
        renderer->m_overrideMat->m_fragmentShader  = m_params.OverrideFragmentShader;
        renderer->m_overrideMat->m_diffuseTexture  = mat->m_diffuseTexture;
        renderer->m_overrideMat->m_emissiveTexture = mat->m_emissiveTexture;
        renderer->m_overrideMat->m_emissiveColor   = mat->m_emissiveColor;
        renderer->m_overrideMat->m_cubeMap         = mat->m_cubeMap;
        renderer->m_overrideMat->m_color           = mat->m_color;
        renderer->m_overrideMat->SetAlpha(mat->GetAlpha());
        renderer->m_overrideMat->Init();

        renderer->Render(job, m_params.ForwardParams.Cam, lightList);
      }

      RenderTranslucent(m_params.ForwardParams.TranslucentJobs,
                        m_params.ForwardParams.Cam,
                        m_params.ForwardParams.Lights);

      nvtxRangePop();
    }

    void SingleMatForwardRenderPass::PreRender()
    {
      nvtxRangePushA("SingleMatForwardRenderPass PreRender");

      ForwardRenderPass::m_params = m_params.ForwardParams;
      ForwardRenderPass::PreRender();

      m_overrideMat->UnInit();
      m_overrideMat->m_fragmentShader = m_params.OverrideFragmentShader;
      m_overrideMat->Init();

      nvtxRangePop();
    };

  } // namespace Editor
} // namespace ToolKit