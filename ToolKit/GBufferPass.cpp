#include "GBufferPass.h"
#include "ToolKit.h"
#include "Mesh.h"
#include "Material.h"

namespace ToolKit
{

  GBufferPass::GBufferPass()
  {
    RenderTargetSettigs gBufferRenderTargetSettings = {
        0,
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

    m_gPosRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);

    m_gNormalRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);

    m_gColorRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);

    m_gEmissiveRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);

    m_gIblRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);

    // gBufferRenderTargetSettings.InternalFormat = GraphicTypes::FormatR16F;
    // gBufferRenderTargetSettings.Format         = GraphicTypes::FormatRed;

    m_gLinearDepthRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);

    gBufferRenderTargetSettings.InternalFormat = GraphicTypes::FormatRG16F;
    gBufferRenderTargetSettings.Format         = GraphicTypes::FormatRG;

    m_gMetallicRoughnessRt =
        std::make_shared<RenderTarget>(1024, 1024, gBufferRenderTargetSettings);

    m_framebuffer     = std::make_shared<Framebuffer>();
    m_gBufferMaterial = std::make_shared<Material>();
  }

  GBufferPass::GBufferPass(const GBufferPassParams& params) : GBufferPass()
  {
    m_params = params;
  }

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

    m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                 m_gPosRt);
    m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment1,
                                 m_gNormalRt);
    m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment2,
                                 m_gColorRt);
    m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment3,
                                 m_gEmissiveRt);
    m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment4,
                                 m_gLinearDepthRt);
    m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment5,
                                 m_gMetallicRoughnessRt);
    m_framebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment6,
                                 m_gIblRt);

    // Gbuffer material
    ShaderPtr vertexShader = GetShaderManager()->Create<Shader>(
        ShaderPath("defaultVertex.shader", true));
    ShaderPtr fragmentShader = GetShaderManager()->Create<Shader>(
        ShaderPath("gBufferFrag.shader", true));
    m_gBufferMaterial->m_vertexShader   = vertexShader;
    m_gBufferMaterial->m_fragmentShader = fragmentShader;

    m_initialized                       = true;
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
    Pass::PreRender();

    GetRenderer()->ResetTextureSlots();

    GetRenderer()->SetFramebuffer(m_framebuffer, true, Vec4(0.0f));
    GetRenderer()->SetCameraLens(m_params.camera);

    CreateRenderJobs(m_params.entities);
  }

  void GBufferPass::PostRender() { Pass::PostRender(); }

  void GBufferPass::Render()
  {
    Renderer* renderer = GetRenderer();

    for (Entity* ntt : m_params.entities)
    {
      MultiMaterialPtr mmComp    = ntt->GetComponent<MultiMaterialComponent>();
      uint activeMeshIndex       = 0;
      MaterialPtr activeMaterial = nullptr;

      if (mmComp == nullptr)
      {
        MaterialComponentPtr matComp = ntt->GetMaterialComponent();
        if (matComp)
        {
          activeMaterial = ntt->GetRenderMaterial();
        }
      }

      MeshComponentPtrArray meshComps;
      ntt->GetComponent<MeshComponent>(meshComps);
      for (MeshComponentPtr meshComp : meshComps)
      {
        MeshRawCPtrArray meshes;
        meshComp->GetMeshVal()->GetAllMeshes(meshes);
        for (uint meshIndx = 0; meshIndx < meshes.size();
             meshIndx++, activeMeshIndex++)
        {
          const Mesh* mesh = meshes[meshIndx];
          if (mmComp && mmComp->GetMaterialList().size() > activeMeshIndex)
          {
            activeMaterial = mmComp->GetMaterialList()[activeMeshIndex];
          }
          if (activeMaterial == nullptr)
          {
            activeMaterial = mesh->m_material;
          }
          if (activeMaterial == nullptr)
          {
            continue;
          }
          if (activeMaterial->GetRenderState()->useForwardPath ||
              (activeMaterial->GetRenderState()->blendFunction !=
                   BlendFunction::NONE &&
               activeMaterial->GetRenderState()->blendFunction !=
                   BlendFunction::ALPHA_MASK))
          {
            continue;
          }

          m_gBufferMaterial->SetRenderState(activeMaterial->GetRenderState());
          m_gBufferMaterial->UnInit();
          m_gBufferMaterial->m_diffuseTexture =
              activeMaterial->m_diffuseTexture;
          m_gBufferMaterial->m_emissiveTexture =
              activeMaterial->m_emissiveTexture;
          m_gBufferMaterial->m_emissiveColor = activeMaterial->m_emissiveColor;
          m_gBufferMaterial->m_metallicRoughnessTexture =
              activeMaterial->m_metallicRoughnessTexture;
          m_gBufferMaterial->m_normalMap    = activeMaterial->m_normalMap;
          m_gBufferMaterial->m_cubeMap      = activeMaterial->m_cubeMap;
          m_gBufferMaterial->m_color        = activeMaterial->m_color;
          m_gBufferMaterial->m_alpha        = activeMaterial->m_alpha;
          m_gBufferMaterial->m_metallic     = activeMaterial->m_metallic;
          m_gBufferMaterial->m_roughness    = activeMaterial->m_roughness;
          m_gBufferMaterial->m_materialType = activeMaterial->m_materialType;
          m_gBufferMaterial->Init();
          renderer->m_overrideMat = m_gBufferMaterial;

          renderer->Render(ntt, m_params.camera, {}, {activeMeshIndex});
        }
      }
    }
  }

} // namespace ToolKit
