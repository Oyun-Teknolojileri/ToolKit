#include "GradientSky.h"

#include "EnvironmentComponent.h"

namespace ToolKit
{

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

    SkyBase::Init();

    // Skybox material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("skyboxVert.shader", true));

    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("gradientSkyboxFrag.shader", true));

    ConstructSkyMaterial(vert, frag);

    RenderTask task {[this](Renderer* renderer) -> void
                     {
                       // Render gradient to cubemap and store the output
                       GenerateGradientCubemap();

                       m_onInit = true;

                       // Create irradiance map from cubemap and set
                       GenerateIrradianceCubemap();
                     }};

    GetRenderSystem()->AddRenderTask(task);
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

  void GradientSky::ParameterEventConstructor()
  {
    ParamIntensity().m_onValueChangedFn.clear();
    ParamIntensity().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          GetComponent<EnvironmentComponent>()->SetIntensityVal(
              std::get<float>(newVal));
        });
  }

  void GradientSky::GenerateGradientCubemap()
  {
    RenderTask task = {
        [this](Renderer* renderer) -> void
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

          renderer->EnableDepthTest(false);

          // Views for 6 different angles
          CameraPtr cam = std::make_shared<Camera>();
          cam->SetLens(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

          Mat4 views[] = {
              glm::lookAt(ZERO,
                          Vec3(1.0f, 0.0f, 0.0f),
                          Vec3(0.0f, -1.0f, 0.0f)),
              glm::lookAt(ZERO,
                          Vec3(-1.0f, 0.0f, 0.0f),
                          Vec3(0.0f, -1.0f, 0.0f)),
              glm::lookAt(ZERO,
                          Vec3(0.0f, -1.0f, 0.0f),
                          Vec3(0.0f, 0.0f, -1.0f)),
              glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
              glm::lookAt(ZERO,
                          Vec3(0.0f, 0.0f, 1.0f),
                          Vec3(0.0f, -1.0f, 0.0f)),
              glm::lookAt(ZERO,
                          Vec3(0.0f, 0.0f, -1.0f),
                          Vec3(0.0f, -1.0f, 0.0f))};

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
                              0,
                              -1,
                              (Framebuffer::CubemapFace) i);

            renderer->SetFramebuffer(fb, true, Vec4(0.0f));
            renderer->DrawCube(cam.get(), m_skyboxMaterial);
          }

          renderer->EnableDepthTest(true);

          // Take the ownership of render target.
          GetHdri()->m_cubemap =
              std::make_shared<CubeMap>(cubemap->m_textureId);

          cubemap->m_textureId = 0;
          cubemap              = nullptr;
        }};

    GetRenderSystem()->AddRenderTask(task);
  }

  void GradientSky::GenerateIrradianceCubemap()
  {
    RenderTask task = {[this](Renderer* renderer) -> void
                       {
                         HdriPtr hdr = GetHdri();
                         hdr->m_irradianceCubemap =
                             renderer->GenerateEnvIrradianceMap(
                                 hdr->m_cubemap,
                                 (uint) GetIrradianceResolutionVal(),
                                 (uint) GetIrradianceResolutionVal());

                         if (m_onInit)
                         {
                           m_initialized = true;
                           m_onInit      = false;
                         }
                       }};

    GetRenderSystem()->AddRenderTask(task);
  }

} // namespace ToolKit