/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "GradientSky.h"

#include "Camera.h"
#include "EnvironmentComponent.h"
#include "Material.h"
#include "MathUtil.h"
#include "RHIConstants.h"
#include "RenderSystem.h"
#include "Shader.h"
#include "TKStats.h"

namespace ToolKit
{

  TKDefineClass(GradientSky, SkyBase);

  GradientSky::GradientSky() {}

  GradientSky::~GradientSky() {}

  void GradientSky::Init()
  {
    if (m_initialized || m_waitingForInit)
    {
      return;
    }

    SkyBase::Init();

    // Skybox material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(ShaderPath("skyboxVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(ShaderPath("gradientSkyboxFrag.shader", true));

    ConstructSkyMaterial(vert, frag);

    m_frameBuffer = MakeNewPtr<Framebuffer>(FramebufferSettings {0, 0, false, false}, "SkyFB");
    m_frameBuffer->Init();

    GradientSkyPtr self = Self<GradientSky>(); // Make sure, the object will stay valid.
    RenderTask task {[self, this](Renderer* renderer) -> void
                     {
                       if (!self || m_initialized)
                       {
                         return;
                       }

                       // Render gradient to cubemap and store the output
                       GenerateGradientCubemap(renderer);

                       // Create irradiance map from cubemap and set
                       GenerateIrradianceCubemap(renderer);

                       renderer->SetFramebuffer(nullptr, GraphicBitFields::None);
                       m_frameBuffer    = nullptr;

                       m_initialized    = true;
                       m_waitingForInit = false;
                     }};

    GetRenderSystem()->AddRenderTask(task);
    m_waitingForInit = true;
  }

  MaterialPtr GradientSky::GetSkyboxMaterial()
  {
    Init();

    m_skyboxMaterial->UpdateProgramUniform("topColor", GetTopColorVal());
    m_skyboxMaterial->UpdateProgramUniform("middleColor", GetMiddleColorVal());
    m_skyboxMaterial->UpdateProgramUniform("bottomColor", GetBottomColorVal());
    m_skyboxMaterial->UpdateProgramUniform("exponent", GetGradientExponentVal());

    return m_skyboxMaterial;
  }

  bool GradientSky::ReadyToRender() { return m_skyboxMaterial != nullptr; }

  void GradientSky::ParameterConstructor()
  {
    Super::ParameterConstructor();

    TopColor_Define(Vec3(0.3f, 0.3f, 1.0f), "Sky", 90, true, true, {true});
    MiddleColor_Define(Vec3(1.0f, 1.0f, 0.8f), "Sky", 90, true, true, {true});
    BottomColor_Define(Vec3(0.5f, 0.3f, 0.1f), "Sky", 90, true, true, {true});
    GradientExponent_Define(0.3f, "Sky", 90, true, true, {false, true, 0.0f, 10.0f, 0.02f});

    SetNameVal("Gradient Sky");
  }

  void GradientSky::ParameterEventConstructor() { Super::ParameterEventConstructor(); }

  void GradientSky::GenerateGradientCubemap(Renderer* renderer)
  {
    const TextureSettings set = {GraphicTypes::TargetCubeMap,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::SampleNearest,
                                 GraphicTypes::SampleNearest,
                                 GraphicTypes::FormatRGBA16F,
                                 GraphicTypes::FormatRGBA,
                                 GraphicTypes::TypeFloat,
                                 1,
                                 false};

    uint size                 = (uint) GetIBLTextureSizeVal().GetValue<int>();
    RenderTargetPtr cubemap   = MakeNewPtr<RenderTarget>(size, size, set);
    cubemap->Init();

    // Create material
    m_skyboxMaterial->UpdateProgramUniform("topColor", GetTopColorVal());
    m_skyboxMaterial->UpdateProgramUniform("middleColor", GetMiddleColorVal());
    m_skyboxMaterial->UpdateProgramUniform("bottomColor", GetBottomColorVal());
    m_skyboxMaterial->UpdateProgramUniform("exponent", GetGradientExponentVal());

    // Views for 6 different angles
    CameraPtr cam = MakeNewPtr<Camera>();
    cam->SetLens(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

    Mat4 views[] = {glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    for (int i = 0; i < 6; ++i)
    {
      Vec3 pos;
      Quaternion rot;
      Vec3 sca;
      DecomposeMatrix(views[i], &pos, &rot, &sca);

      cam->m_node->SetTranslation(ZERO, TransformationSpace::TS_WORLD);
      cam->m_node->SetOrientation(rot, TransformationSpace::TS_WORLD);
      cam->m_node->SetScale(sca);

      m_frameBuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0,
                                        cubemap,
                                        0,
                                        -1,
                                        (Framebuffer::CubemapFace) i);
      if (i > 0)
      {
        Stats::AddHWRenderPass();
      }

      renderer->SetFramebuffer(m_frameBuffer, GraphicBitFields::None);
      renderer->DrawCube(cam, m_skyboxMaterial);
    }

    CubeMapPtr newCubemap = MakeNewPtr<CubeMap>();
    newCubemap->Consume(cubemap);
    GetHdri()->m_cubemap = newCubemap;
  }

  void GradientSky::GenerateIrradianceCubemap(Renderer* renderer)
  {
    HdriPtr hdr           = GetHdri();
    uint size             = (uint) GetIBLTextureSizeVal().GetValue<int>();

    // hdr->m_diffuseEnvMap = hdr->m_cubemap;
    hdr->m_diffuseEnvMap  = renderer->GenerateDiffuseEnvMap(hdr->m_cubemap, size);

    hdr->m_specularEnvMap = renderer->GenerateSpecularEnvMap(hdr->m_cubemap, size, RHIConstants::SpecularIBLLods);
  }

  XmlNode* GradientSky::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

} // namespace ToolKit