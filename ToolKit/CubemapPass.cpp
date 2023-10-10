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

#include "CubemapPass.h"

#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"

#define NOMINMAX
#include "nvtx3.hpp"
#undef WriteConsole

namespace ToolKit
{

  CubeMapPass::CubeMapPass() { m_cube = MakeNewPtr<Cube>(); }

  CubeMapPass::CubeMapPass(const CubeMapPassParams& params) : CubeMapPass() { m_params = params; }

  CubeMapPass::~CubeMapPass() { m_cube = nullptr; }

  void CubeMapPass::Render()
  {
    NVTX3_FUNC_RANGE();

    m_cube->m_node->SetTransform(m_params.Transform);

    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_params.FrameBuffer, false);

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJobs({m_cube}, jobs);
    renderer->Render(jobs, m_params.Cam);
  }

  void CubeMapPass::PreRender()
  {
    nvtx3::mark("Cubemap Pass");
    NVTX3_FUNC_RANGE();

    Pass::PreRender();
    MaterialComponentPtr matCom = m_cube->GetMaterialComponent();
    matCom->SetFirstMaterial(m_params.Material);
    GetRenderer()->SetDepthTestFunc(m_params.DepthFn);
  }

  void CubeMapPass::PostRender()
  {
    NVTX3_FUNC_RANGE();

    Pass::PostRender();
    GetRenderer()->SetDepthTestFunc(CompareFunctions::FuncLess);
  }

} // namespace ToolKit