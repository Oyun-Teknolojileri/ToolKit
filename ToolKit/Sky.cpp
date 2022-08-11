#include "Sky.h"

#include <memory>

#include "ToolKit.h"
#include "DirectionComponent.h"

namespace ToolKit
{
  Sky::Sky()
  {
    EnvironmentComponent* envComp = new EnvironmentComponent;
    envComp->SetHdriVal
    (
      GetTextureManager()->Create<Hdri>(TexturePath("defaultHDRI.hdr", true))
    );
    AddComponent(envComp);

    ParameterConstructor();
    ParameterEventConstructor();
  }

  Sky::~Sky()
  {
  }

  EntityType Sky::GetType() const
  {
    return EntityType::Entity_Sky;
  }

  void Sky::Init()
  {
    if (m_initialized)
    {
      return;
    }

    // Environment Component
    EnvironmentComponentPtr envComp = GetComponent<EnvironmentComponent>();
    envComp->m_entity = this;
    envComp->GetHdriVal()->m_initiated = false;
    envComp->SetExposureVal(GetExposureVal());
    envComp->Init(true);

    // Do not expose environment component
    envComp->ParamMax().m_exposed = false;
    envComp->ParamMin().m_exposed = false;
    envComp->ParamHdri().m_exposed = false;
    envComp->ParamIlluminate().m_exposed = false;
    envComp->ParamIntensity().m_exposed = false;
    envComp->ParamExposure().m_exposed = false;

    // Skybox material
    ShaderPtr vert = GetShaderManager()->Create<Shader>
    (
      ShaderPath("skyboxVert.shader", true)
    );
    ShaderPtr frag = GetShaderManager()->Create<Shader>
    (
      ShaderPath("skyboxFrag.shader", true)
    );
    m_skyboxMaterial = std::make_shared<Material>();
    m_skyboxMaterial->m_cubeMap = envComp->GetHdriVal()->m_cubemap;
    m_skyboxMaterial->m_vertexShader = vert;
    m_skyboxMaterial->m_fragmetShader = frag;
    m_skyboxMaterial->GetRenderState()->cullMode = CullingType::TwoSided;
    m_skyboxMaterial->Init();

    m_initialized = true;
  }

  void Sky::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    ParameterEventConstructor();
  }

  MaterialPtr Sky::GetSkyboxMaterial()
  {
    HdriPtr hdri = GetComponent<EnvironmentComponent>()->GetHdriVal();
    m_skyboxMaterial->m_cubeMap = hdri->m_cubemap;
    return m_skyboxMaterial;
  }

  void Sky::ParameterConstructor()
  {
    DrawSky_Define
    (
      true,
      "Sky",
      90,
      true,
      true
    );
    Illuminate_Define
    (
      true,
      "Sky",
      90,
      true,
      true
    );
    Intensity_Define
    (
      GetComponent<EnvironmentComponent>()->GetIntensityVal(),
      "Sky",
      90,
      true,
      true
    );
    Exposure_Define
    (
      GetComponent<EnvironmentComponent>()->GetExposureVal(),
      "Sky",
      90,
      true,
      true
    );
    Hdri_Define
    (
      GetComponent<EnvironmentComponent>()->GetHdriVal(),
      "Sky",
      90,
      true,
      true
    );

    SetNameVal("Sky");

    ParamVisible().m_exposed = false;
  }

  void Sky::ParameterEventConstructor()
  {
    ParamIntensity().m_onValueChangedFn =
    [this](Value& oldVal, Value& newVal) -> void
    {
      GetComponent<EnvironmentComponent>()->SetIntensityVal
      (
        std::get<float>(newVal)
      );
    };

    ParamExposure().m_onValueChangedFn =
    [this](Value& oldVal, Value& newVal) -> void
    {
      GetComponent<EnvironmentComponent>()->SetExposureVal
      (
        std::get<float>(newVal)
      );
    };

    ParamHdri().m_onValueChangedFn =
    [this](Value& oldVal, Value& newVal) -> void
    {
      GetComponent<EnvironmentComponent>()->SetHdriVal
      (
        std::get<HdriPtr>(newVal)
      );
    };
  }

}  // namespace ToolKit
