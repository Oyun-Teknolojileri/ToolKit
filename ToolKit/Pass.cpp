/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Pass.h"

namespace ToolKit
{

  Pass::Pass(StringView name) : m_name(name) {}

  Pass::~Pass() {}

  void Pass::PreRender() { Stats::BeginGpuScope(m_name); }

  void Pass::PostRender() { Stats::EndGpuScope(); }

  void Pass::RenderSubPass(const PassPtr& pass)
  {
    Renderer* renderer = GetRenderer();
    pass->SetRenderer(renderer);
    pass->PreRender();
    pass->Render();
    pass->PostRender();
  }

  Renderer* Pass::GetRenderer() { return m_renderer; }

  void Pass::SetRenderer(Renderer* renderer) { m_renderer = renderer; }

  void Pass::UpdateUniform(const ShaderUniform& shaderUniform)
  {
    if (m_program != nullptr)
    {
      m_program->UpdateCustomUniform(shaderUniform);
    }
  }

} // namespace ToolKit
