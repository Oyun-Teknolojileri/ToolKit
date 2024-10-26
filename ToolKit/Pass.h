/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "RenderJob.h"
#include "Renderer.h"

namespace ToolKit
{

  typedef std::shared_ptr<class Pass> PassPtr;
  typedef std::vector<PassPtr> PassPtrArray;

  /** Base Pass class. */
  class TK_API Pass
  {
   public:
    Pass(StringView name);
    virtual ~Pass();

    virtual void Render() = 0;
    virtual void PreRender();
    virtual void PostRender();
    void RenderSubPass(const PassPtr& pass);

    Renderer* GetRenderer();
    void SetRenderer(Renderer* renderer);

    /** This function is used to pass custom uniforms to this pass. */
    void UpdateUniform(const ShaderUniform& shaderUniform);

   protected:
    GpuProgramPtr m_program = nullptr; //!< Program used to draw objects with in the pass.
    StringView m_name; //!< Label that appears in the gpu profile / debug applications (RenderDoc etc...).

   private:
    Renderer* m_renderer = nullptr;
  };

} // namespace ToolKit
