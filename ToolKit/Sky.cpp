#include "Sky.h"

#include "DirectionComponent.h"
#include "ToolKit.h"

#include <memory>

namespace ToolKit
{
  SkyBase::SkyBase()
  {
    ParameterConstructor();
    ParameterEventConstructor();
  }

  EntityType SkyBase::GetType() const { return EntityType::Entity_SkyBase; }

  void SkyBase::Init() {}

  void SkyBase::ReInit()
  {
    m_initialized = false;
    Init();
  }

  void SkyBase::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    ParameterEventConstructor();
  }

  bool SkyBase::IsInitialized() { return m_initialized; }

  MaterialPtr SkyBase::GetSkyboxMaterial() { return m_skyboxMaterial; }

  CubeMapPtr SkyBase::GetIrradianceMap()
  {
    assert(false);
    return CubeMapPtr();
  }

  BoundingBox SkyBase::GetAABB(bool inWorld) const
  {
    // Return a unit boundary.
    return {
        {-0.5f, -0.5f, -0.5f},
        {0.5f,  0.5f,  0.5f }
    };
  }

  void SkyBase::ParameterConstructor()
  {
    DrawSky_Define(true, "Sky", 90, true, true);
    Illuminate_Define(true, "Sky", 90, true, true);
    Intensity_Define(0.25f,
                     "Sky",
                     90,
                     true,
                     true,
                     {false, true, 0.0f, 100000.0f, 0.1f});

    SetNameVal("SkyBase");
  }

  void SkyBase::ParameterEventConstructor() {}

  Sky::Sky()
  {
    EnvironmentComponent* envComp = new EnvironmentComponent;
    envComp->SetHdriVal(GetTextureManager()->Create<Hdri>(
        TexturePath("defaultHDRI.hdr", true)));
    AddComponent(envComp);

    ParameterConstructor();
    ParameterEventConstructor();
  }

  Sky::~Sky() {}

  EntityType Sky::GetType() const { return EntityType::Entity_Sky; }

  void Sky::Init()
  {
    if (m_initialized)
    {
      return;
    }

    // Environment Component
    EnvironmentComponentPtr envComp = GetComponent<EnvironmentComponent>();
    envComp->m_entity               = this;
    envComp->GetHdriVal()->UnInit();
    envComp->SetExposureVal(GetExposureVal());
    envComp->Init(true);

    // Do not expose environment component
    envComp->ParamPositionOffset().m_exposed = false;
    envComp->ParamSize().m_exposed           = false;
    envComp->ParamHdri().m_exposed           = false;
    envComp->ParamIlluminate().m_exposed     = false;
    envComp->ParamIntensity().m_exposed      = false;
    envComp->ParamExposure().m_exposed       = false;

    // Skybox material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("skyboxVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("skyboxFrag.shader", true));
    m_skyboxMaterial                   = std::make_shared<Material>();
    m_skyboxMaterial->m_cubeMap        = envComp->GetHdriVal()->m_cubemap;
    m_skyboxMaterial->m_vertexShader   = vert;
    m_skyboxMaterial->m_fragmentShader = frag;
    m_skyboxMaterial->GetRenderState()->cullMode = CullingType::TwoSided;
    m_skyboxMaterial->Init();

    m_initialized = true;
  }

  MaterialPtr Sky::GetSkyboxMaterial()
  {
    Init();

    HdriPtr hdri = GetComponent<EnvironmentComponent>()->GetHdriVal();
    m_skyboxMaterial->m_cubeMap = hdri->m_cubemap;
    return m_skyboxMaterial;
  }

  CubeMapPtr Sky::GetIrradianceMap() { return CubeMapPtr(); }

  void Sky::ParameterConstructor()
  {
    Exposure_Define(GetComponent<EnvironmentComponent>()->GetExposureVal(),
                    "Sky",
                    90,
                    true,
                    true,
                    {false, true, 0.0f, 50.0f, 0.05f});
    Hdri_Define(GetComponent<EnvironmentComponent>()->GetHdriVal(),
                "Sky",
                90,
                true,
                true);

    SetNameVal("Sky");

    ParamVisible().m_exposed = false;
  }

  void Sky::ParameterEventConstructor()
  {
    ParamIntensity().m_onValueChangedFn.clear();
    ParamIntensity().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          GetComponent<EnvironmentComponent>()->SetIntensityVal(
              std::get<float>(newVal));
        });

    ParamExposure().m_onValueChangedFn.clear();
    ParamExposure().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          GetComponent<EnvironmentComponent>()->SetExposureVal(
              std::get<float>(newVal));
        });

    ParamHdri().m_onValueChangedFn.clear();
    ParamHdri().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          GetComponent<EnvironmentComponent>()->SetHdriVal(
              std::get<HdriPtr>(newVal));
        });
  }

  GradientSky::GradientSky()
  {
    ParameterConstructor();
    ParameterEventConstructor();
  }

  GradientSky::~GradientSky() {}

  EntityType GradientSky::GetType() const
  {
    return EntityType::Entity_GradientSky;
  }

  void GradientSky::Init()
  {
    if (m_initialized)
    {
      return;
    }

    // Skybox material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("skyboxVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("gradientSkyboxFrag.shader", true));
    m_skyboxMaterial                             = std::make_shared<Material>();
    m_skyboxMaterial->m_vertexShader             = vert;
    m_skyboxMaterial->m_fragmentShader           = frag;
    m_skyboxMaterial->GetRenderState()->cullMode = CullingType::TwoSided;
    m_skyboxMaterial->Init();

    // Render gradient to cubemap and store the output
    GenerateGradientCubemap();

    // Create irradiance map from cubemap
    GenerateIrradianceCubemap();

    m_initialized = true;
  }

  MaterialPtr GradientSky::GetSkyboxMaterial()
  {
    Init();

    m_skyboxMaterial->m_fragmentShader->SetShaderParameter(
        "topColor",
        ParameterVariant(GetTopColorVal()));
    m_skyboxMaterial->m_fragmentShader->SetShaderParameter(
        "middleColor",
        ParameterVariant(GetMiddleColorVal()));
    m_skyboxMaterial->m_fragmentShader->SetShaderParameter(
        "bottomColor",
        ParameterVariant(GetBottomColorVal()));
    m_skyboxMaterial->m_fragmentShader->SetShaderParameter(
        "exponent",
        ParameterVariant(GetGradientExponentVal()));
    return m_skyboxMaterial;
  }

  CubeMapPtr GradientSky::GetIrradianceMap() { return m_irradianceMap; }

  void GradientSky::ParameterConstructor()
  {
    TopColor_Define(Vec3(0.3f, 0.3f, 1.0f), "Sky", 90, true, true, {true});
    MiddleColor_Define(Vec3(1.0f, 1.0f, 0.8f), "Sky", 90, true, true, {true});
    BottomColor_Define(Vec3(0.5f, 0.3f, 0.1f), "Sky", 90, true, true, {true});
    GradientExponent_Define(0.3f,
                            "Sky",
                            90,
                            true,
                            true,
                            {false, true, 0.0f, 10.0f, 0.02f});
    IrradianceResolution_Define(64.0f,
                                "Sky",
                                90,
                                true,
                                true,
                                {false, true, 32.0f, 2048.0f, 2.0f});

    SetNameVal("Gradient Sky");
  }

  void GradientSky::ParameterEventConstructor() {}

  void GradientSky::GenerateGradientCubemap()
  {
    FramebufferPtr fb = std::make_shared<Framebuffer>();
    fb->Init({m_size, m_size, false, true});

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
    RenderTargetPtr cubemap =
        std::make_shared<RenderTarget>(m_size, m_size, set);
    cubemap->Init();

    // Create material
    m_skyboxMaterial->m_fragmentShader->SetShaderParameter(
        "topColor",
        ParameterVariant(GetTopColorVal()));
    m_skyboxMaterial->m_fragmentShader->SetShaderParameter(
        "middleColor",
        ParameterVariant(GetMiddleColorVal()));
    m_skyboxMaterial->m_fragmentShader->SetShaderParameter(
        "bottomColor",
        ParameterVariant(GetBottomColorVal()));
    m_skyboxMaterial->m_fragmentShader->SetShaderParameter(
        "exponent",
        ParameterVariant(GetGradientExponentVal()));
    m_skyboxMaterial->GetRenderState()->depthTestEnabled = false;

    // Views for 6 different angles
    CameraPtr cam = std::make_shared<Camera>();
    cam->SetLens(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    Mat4 views[] = {
        glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
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

      fb->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                        cubemap,
                        -1,
                        (Framebuffer::CubemapFace) i);

      GetRenderer()->SetFramebuffer(fb, true, Vec4(0.0f));
      GetRenderer()->DrawCube(cam.get(), m_skyboxMaterial);
    }
    m_skyboxMaterial->GetRenderState()->depthTestEnabled = true;

    // Take the ownership of render target.
    m_skyboxMap          = std::make_shared<CubeMap>(cubemap->m_textureId);
    cubemap->m_textureId = 0;
    cubemap              = nullptr;
  }

  void GradientSky::GenerateIrradianceCubemap()
  {
    TexturePtr irradianceMap = GetRenderer()->GenerateIrradianceCubemap(
        m_skyboxMap,
        (uint) GetIrradianceResolutionVal(),
        (uint) GetIrradianceResolutionVal());

    // Take the ownership of render target.
    m_irradianceMap = std::make_shared<CubeMap>(irradianceMap->m_textureId);
    irradianceMap->m_textureId = 0;
    irradianceMap              = nullptr;
  }

} // namespace ToolKit
