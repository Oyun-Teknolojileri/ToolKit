/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "GBufferPass.h"

#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "TKOpenGL.h"
#include "TKProfiler.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  GBufferPass::GBufferPass()
  {
    RenderTargetSettigs gBufferRenderTargetSettings = {0,
                                                       GraphicTypes::Target2D,
                                                       GraphicTypes::UVClampToEdge,
                                                       GraphicTypes::UVClampToEdge,
                                                       GraphicTypes::UVClampToEdge,
                                                       GraphicTypes::SampleNearest,
                                                       GraphicTypes::SampleNearest,
                                                       GraphicTypes::FormatRGBA16F,
                                                       GraphicTypes::FormatRGBA,
                                                       GraphicTypes::TypeFloat,
                                                       1};

    m_gPosRt                                        = MakeNewPtr<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);
    m_gNormalRt                                     = MakeNewPtr<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);
    m_gColorRt                                      = MakeNewPtr<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);
    m_gEmissiveRt                                   = MakeNewPtr<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);
    m_gIblRt                                        = MakeNewPtr<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);

    gBufferRenderTargetSettings.InternalFormat      = GraphicTypes::FormatRGBA32F;
    gBufferRenderTargetSettings.Format              = GraphicTypes::FormatRGBA;
    // Note: A32 is not used, it didn't work on Android devices when we bind it to frame buffer
    m_gLinearDepthRt                                = MakeNewPtr<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);

    gBufferRenderTargetSettings.InternalFormat      = GraphicTypes::FormatRG16F;
    gBufferRenderTargetSettings.Format              = GraphicTypes::FormatRG;

    m_gMetallicRoughnessRt                          = MakeNewPtr<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);

    m_framebuffer                                   = MakeNewPtr<Framebuffer>();
    m_gBufferMaterial                               = MakeNewPtr<Material>();
  }

  GBufferPass::GBufferPass(const GBufferPassParams& params) : GBufferPass() { m_params = params; }

  GBufferPass::~GBufferPass()
  {
    m_framebuffer     = nullptr;
    m_gPosRt          = nullptr;
    m_gNormalRt       = nullptr;
    m_gColorRt        = nullptr;
    m_gEmissiveRt     = nullptr;
    m_gIblRt          = nullptr;
    m_gLinearDepthRt  = nullptr;
    m_gBufferMaterial = nullptr;
  }

  void GBufferPass::InitGBuffers(int width, int height)
  {
    PUSH_GPU_MARKER("GBufferPass::InitGBuffers");
    PUSH_CPU_MARKER("GBufferPass::InitGBuffers");

    bool reInitGBuffers = false;
    if (m_initialized)
    {
      if (width != m_width || height != m_height)
      {
        reInitGBuffers = true;
        UnInitGBuffers();
      }
    }

    if (!m_initialized || reInitGBuffers)
    {
      m_width  = width;
      m_height = height;

      // Gbuffers render targets
      m_framebuffer->Init({(uint) width, (uint) height, false, true});
      m_gPosRt->m_width                = width;
      m_gPosRt->m_height               = height;
      m_gNormalRt->m_width             = width;
      m_gNormalRt->m_height            = height;
      m_gColorRt->m_width              = width;
      m_gColorRt->m_height             = height;
      m_gEmissiveRt->m_width           = width;
      m_gIblRt->m_width                = width;
      m_gIblRt->m_height               = height;
      m_gEmissiveRt->m_height          = height;
      m_gLinearDepthRt->m_width        = width;
      m_gLinearDepthRt->m_height       = height;
      m_gMetallicRoughnessRt->m_width  = width;
      m_gMetallicRoughnessRt->m_height = height;
      m_gPosRt->Init();
      m_gNormalRt->Init();
      m_gColorRt->Init();
      m_gEmissiveRt->Init();
      m_gIblRt->Init();
      m_gLinearDepthRt->Init();
      m_gMetallicRoughnessRt->Init();
    }

    m_framebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_gPosRt);
    m_framebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment1, m_gNormalRt);
    m_framebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment2, m_gColorRt);
    m_framebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment3, m_gEmissiveRt);
    m_framebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment4, m_gLinearDepthRt);
    m_framebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment5, m_gMetallicRoughnessRt);
    m_framebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment6, m_gIblRt);

    // Gbuffer material
    ShaderPtr vertexShader              = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader", true));
    ShaderPtr fragmentShader            = GetShaderManager()->Create<Shader>(ShaderPath("gBufferFrag.shader", true));

    m_gBufferMaterial->m_vertexShader   = vertexShader;
    m_gBufferMaterial->m_fragmentShader = fragmentShader;

    m_initialized                       = true;

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void GBufferPass::UnInitGBuffers()
  {
    if (!m_initialized)
    {
      return;
    }

    m_framebuffer->UnInit();

    m_gPosRt->UnInit();
    m_gNormalRt->UnInit();
    m_gColorRt->UnInit();
    m_gEmissiveRt->UnInit();
    m_gIblRt->UnInit();
    m_gLinearDepthRt->UnInit();
    m_gMetallicRoughnessRt->UnInit();

    m_initialized = false;
  }

  void GBufferPass::PreRender()
  {
    PUSH_GPU_MARKER("GBufferPass::PreRender");
    PUSH_CPU_MARKER("GBufferPass::PreRender");

    Pass::PreRender();

    Renderer* renderer = GetRenderer();
    renderer->ResetTextureSlots();
    renderer->SetFramebuffer(m_framebuffer, true, Vec4(0.0f));
    renderer->SetCameraLens(m_params.Camera);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void GBufferPass::PostRender()
  {
    PUSH_GPU_MARKER("GBufferPass::PostRender");
    PUSH_CPU_MARKER("GBufferPass::PostRender");

    Pass::PostRender();

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void GBufferPass::Render()
  {
    PUSH_GPU_MARKER("GBufferPass::Render");
    PUSH_CPU_MARKER("GBufferPass::Render");

    Renderer* renderer = GetRenderer();
    for (RenderJob& job : m_params.RendeJobs)
    {
      MaterialPtr activeMaterial = job.Material;
      m_gBufferMaterial->SetRenderState(activeMaterial->GetRenderState());
      m_gBufferMaterial->UnInit();
      m_gBufferMaterial->m_diffuseTexture           = activeMaterial->m_diffuseTexture;
      m_gBufferMaterial->m_emissiveTexture          = activeMaterial->m_emissiveTexture;
      m_gBufferMaterial->m_emissiveColor            = activeMaterial->m_emissiveColor;
      m_gBufferMaterial->m_metallicRoughnessTexture = activeMaterial->m_metallicRoughnessTexture;
      m_gBufferMaterial->m_normalMap                = activeMaterial->m_normalMap;
      m_gBufferMaterial->m_cubeMap                  = activeMaterial->m_cubeMap;
      m_gBufferMaterial->m_color                    = activeMaterial->m_color;
      m_gBufferMaterial->m_metallic                 = activeMaterial->m_metallic;
      m_gBufferMaterial->m_roughness                = activeMaterial->m_roughness;
      m_gBufferMaterial->SetAlpha(activeMaterial->GetAlpha());
      m_gBufferMaterial->Init();

      renderer->m_overrideMat = m_gBufferMaterial;
      renderer->Render(job, m_params.Camera, {});
    }

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

} // namespace ToolKit
