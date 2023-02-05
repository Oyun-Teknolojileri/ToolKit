#include "CubemapPass.h"

#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"

namespace ToolKit
{

  CubeMapPass::CubeMapPass()
  {
    m_cube = std::make_shared<Cube>();
    m_cube->AddComponent(new MaterialComponent());
  }

  CubeMapPass::CubeMapPass(const CubeMapPassParams& params) : CubeMapPass()
  {
    m_params = params;
  }

  CubeMapPass::~CubeMapPass() { m_cube = nullptr; }

  void CubeMapPass::Render()
  {
    m_cube->m_node->SetTransform(m_params.Transform);

    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_params.FrameBuffer, false);
    renderer->Render(m_cube.get(), m_params.Cam);
  }

  void CubeMapPass::PreRender()
  {
    Pass::PreRender();
    MaterialComponentPtr matCom = m_cube->GetMaterialComponent();
    matCom->SetMaterialVal(m_params.Material);
    GetRenderer()->SetDepthTestFunc(m_params.DepthFn);
  }

  void CubeMapPass::PostRender()
  {
    Pass::PostRender();
    GetRenderer()->SetDepthTestFunc(CompareFunctions::FuncLess);
  }

} // namespace ToolKit