#include "FullQuadPass.h"

#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"

namespace ToolKit
{

  FullQuadPass::FullQuadPass()
  {
    m_camera                   = std::make_shared<Camera>(); // Unused.
    m_quad                     = std::make_shared<Quad>();

    m_material                 = std::make_shared<Material>();
    m_material->m_vertexShader = GetShaderManager()->Create<Shader>(
        ShaderPath("fullQuadVert.shader", true));
  }

  FullQuadPass::FullQuadPass(const FullQuadPassParams& params) : FullQuadPass()
  {
    m_params = params;
  }

  FullQuadPass::~FullQuadPass()
  {
    m_camera   = nullptr;
    m_quad     = nullptr;
    m_material = nullptr;
  }

  void FullQuadPass::Render()
  {
    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_params.FrameBuffer,
                             m_params.ClearFrameBuffer,
                             {0.0f, 0.0f, 0.0f, 1.0f});

    renderer->Render(m_quad.get(), m_camera.get(), m_params.lights);
  }

  void FullQuadPass::PreRender()
  {
    Pass::PreRender();
    Renderer* renderer      = GetRenderer();
    renderer->m_overrideMat = nullptr;
    renderer->EnableDepthTest(false);

    m_material->m_fragmentShader = m_params.FragmentShader;
    m_material->UnInit(); // Reinit in case, shader change.
    m_material->Init();
    m_material->GetRenderState()->blendFunction = m_params.BlendFunc;

    MeshComponentPtr mc                         = m_quad->GetMeshComponent();
    MeshPtr mesh                                = mc->GetMeshVal();
    mesh->m_material                            = m_material;
    mesh->Init();
  }

  void FullQuadPass::PostRender()
  {
    Pass::PostRender();
    GetRenderer()->EnableDepthTest(true);
  }

} // namespace ToolKit