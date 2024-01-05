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
#include "RenderSystem.h"
#include "Shader.h"
#include "TKStats.h"

#include "DebugNew.h"

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

    RenderTask task {[this](Renderer* renderer) -> void
                     {
                       if (m_initialized)
                       {
                         return;
                       }

                       // Render gradient to cubemap and store the output
                       GenerateGradientCubemap(renderer);

                       // Create irradiance map from cubemap and set
                       GenerateIrradianceCubemap(renderer);

                       m_initialized    = true;
                       m_waitingForInit = false;
                     }};

    GetRenderSystem()->AddRenderTask(task);
    m_waitingForInit = true;
  }

  MaterialPtr GradientSky::GetSkyboxMaterial()
  {
    Init();

    m_skyboxMaterial->m_fragmentShader->UpdateShaderUniform("topColor", GetTopColorVal());
    m_skyboxMaterial->m_fragmentShader->UpdateShaderUniform("middleColor", GetMiddleColorVal());
    m_skyboxMaterial->m_fragmentShader->UpdateShaderUniform("bottomColor", GetBottomColorVal());
    m_skyboxMaterial->m_fragmentShader->UpdateShaderUniform("exponent", GetGradientExponentVal());

    return m_skyboxMaterial;
  }

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
    FramebufferPtr fb = MakeNewPtr<Framebuffer>();
    fb->Init({0, 0, false, false});

    const RenderTargetSettigs set = {0,
                                     GraphicTypes::TargetCubeMap,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::FormatRGB,
                                     GraphicTypes::FormatRGB,
                                     GraphicTypes::TypeUnsignedByte};

    uint size                     = (uint) GetIBLTextureSizeVal().GetValue<int>();
    RenderTargetPtr cubemap       = MakeNewPtr<RenderTarget>(size, size, set);
    cubemap->Init();

    // Create material
    m_skyboxMaterial->m_fragmentShader->UpdateShaderUniform("topColor", GetTopColorVal());
    m_skyboxMaterial->m_fragmentShader->UpdateShaderUniform("middleColor", GetMiddleColorVal());
    m_skyboxMaterial->m_fragmentShader->UpdateShaderUniform("bottomColor", GetBottomColorVal());
    m_skyboxMaterial->m_fragmentShader->UpdateShaderUniform("exponent", GetGradientExponentVal());

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

      fb->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, cubemap, 0, -1, (Framebuffer::CubemapFace) i);
      if (i > 0)
      {
        AddHWRenderPass();
      }

      renderer->SetFramebuffer(fb, GraphicBitFields::None);
      renderer->DrawCube(cam, m_skyboxMaterial);
    }

    // Take the ownership of render target.
    CubeMapPtr hdriCubeMap = MakeNewPtr<CubeMap>();
    GetHdri()->m_cubemap   = hdriCubeMap;

    TextureSettings textureSettings;
    textureSettings.GenerateMipMap  = false;
    textureSettings.InternalFormat  = cubemap->m_settings.InternalFormat;
    textureSettings.MinFilter       = cubemap->m_settings.MinFilter;
    textureSettings.MipMapMinFilter = GraphicTypes::SampleNearestMipmapNearest;
    textureSettings.Target          = GraphicTypes::TargetCubeMap;
    textureSettings.Type            = GraphicTypes::TypeFloat;

    hdriCubeMap->m_textureId        = cubemap->m_textureId;
    hdriCubeMap->m_width            = cubemap->m_width;
    hdriCubeMap->m_height           = cubemap->m_height;
    hdriCubeMap->m_initiated        = true;
    hdriCubeMap->SetTextureSettings(textureSettings);

    cubemap->m_initiated = false;
    cubemap->m_textureId = 0;
    cubemap              = nullptr;
  }

  void GradientSky::GenerateIrradianceCubemap(Renderer* renderer)
  {

    HdriPtr hdr          = GetHdri();
    uint irRes           = (uint) GetIBLTextureSizeVal().GetValue<int>();

    hdr->m_diffuseEnvMap = renderer->GenerateDiffuseEnvMap(hdr->m_cubemap, irRes, irRes);

    hdr->m_specularEnvMap =
        renderer->GenerateSpecularEnvMap(hdr->m_cubemap, irRes, irRes, Renderer::RHIConstants::SpecularIBLLods);
  }

  XmlNode* GradientSky::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

} // namespace ToolKit