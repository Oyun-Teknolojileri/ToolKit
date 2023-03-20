#include "Sky.h"

#include "DirectionComponent.h"
#include "EnvironmentComponent.h"
#include "ToolKit.h"

#include <memory>

#include "DebugNew.h"

namespace ToolKit
{

  SkyBase::SkyBase()
  {
    ParameterConstructor();
    ParameterEventConstructor();
  }

  EntityType SkyBase::GetType() const { return EntityType::Entity_SkyBase; }

  void SkyBase::Init()
  {
    if (m_initialized)
    {
      return;
    }

    EnvironmentComponentPtr envComp = GetComponent<EnvironmentComponent>();
    if (envComp == nullptr)
    {
      // Create a default environment component.
      envComp              = std::make_shared<EnvironmentComponent>();

      TextureManager* tman = GetTextureManager();
      HdriPtr defHdr =
          tman->Create<Hdri>(tman->GetDefaultResource(ResourceType::Hdri));
      defHdr->Init();

      envComp->SetHdriVal(defHdr);
      AddComponent(envComp);
    }

    Vec3 mp = Vec3(TK_FLT_MAX);
    envComp->SetSizeVal(mp);
    envComp->m_entity = this;
    envComp->Init(false);

    // Do not expose environment component
    envComp->m_localData.ExposeByCategory(false, EnvironmentComponentCategory);
  }

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
    HdriPtr hdri = GetHdri();
    return hdri->m_irradianceCubemap;
  }

  HdriPtr SkyBase::GetHdri() 
  {
    HdriPtr hdri = GetComponent<EnvironmentComponent>()->GetHdriVal();
    return hdri; 
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
    Intensity_Define(1.0f,
                     "Sky",
                     90,
                     true,
                     true,
                     {false, true, 0.0f, 100000.0f, 0.1f});

    SetNameVal("SkyBase");
  }

  void SkyBase::ParameterEventConstructor() {}

  void SkyBase::ConstructSkyMaterial(ShaderPtr vertexPrg, ShaderPtr fragPrg)
  {
    m_skyboxMaterial                   = std::make_shared<Material>();
    m_skyboxMaterial->m_cubeMap        = GetHdri()->m_cubemap;
    m_skyboxMaterial->m_vertexShader   = vertexPrg;
    m_skyboxMaterial->m_fragmentShader = fragPrg;
    m_skyboxMaterial->GetRenderState()->cullMode = CullingType::TwoSided;
    m_skyboxMaterial->Init();
  }

  Sky::Sky()
  {
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

    SkyBase::Init();

    // Skybox material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("skyboxVert.shader", true));

    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("skyboxFrag.shader", true));

    ConstructSkyMaterial(vert, frag);

    m_initialized = true;
  }

  MaterialPtr Sky::GetSkyboxMaterial()
  {
    Init();

    HdriPtr hdri                = GetHdri();
    m_skyboxMaterial->m_cubeMap = hdri->m_cubemap;
    return m_skyboxMaterial;
  }

  void Sky::ParameterConstructor()
  {
    Exposure_Define(1.0f,
                    "Sky",
                    90,
                    true,
                    true,
                    {false, true, 0.0f, 50.0f, 0.05f});

    Hdri_Define(nullptr, "Sky", 90, true, true);

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

} // namespace ToolKit
