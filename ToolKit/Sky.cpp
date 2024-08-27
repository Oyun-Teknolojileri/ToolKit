/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Sky.h"

#include "DirectionComponent.h"
#include "EnvironmentComponent.h"
#include "GradientSky.h"
#include "Material.h"
#include "Shader.h"
#include "ToolKit.h"

namespace ToolKit
{

  TKDefineClass(SkyBase, Entity);

  SkyBase::SkyBase() { m_partOfAABBTree = false; }

  void SkyBase::NativeConstruct() { Super::NativeConstruct(); }

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
      envComp = AddComponent<EnvironmentComponent>();
    }

    HdriPtr hdri = nullptr;
    // Provide an empty hdri to construct gradient sky.
    if (IsA<GradientSky>())
    {
      hdri = MakeNewPtr<Hdri>();
    }
    else
    {
      hdri = envComp->GetHdriVal();
      if (hdri != nullptr)
      {
        hdri->Init();
      }
      else
      {
        // Use default hdri image.
        TextureManager* tman = GetTextureManager();
        hdri                 = tman->Create<Hdri>(tman->GetDefaultResource(Hdri::StaticClass()));
        hdri->Init();
      }
    }

    envComp->SetHdriVal(hdri);

    envComp->SetSizeVal(Vec3(TK_FLT_MAX));
    envComp->OwnerEntity(Self<Entity>());
    envComp->Init(false);

    // Do not expose environment component
    envComp->m_localData.ExposeByCategory(false, EnvironmentComponentCategory);
  }

  void SkyBase::ReInit()
  {
    m_initialized = false;
    Init();
  }

  XmlNode* SkyBase::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* nttNode = Super::DeSerializeImp(info, parent);
    return nttNode->first_node(StaticClass()->Name.c_str());
  }

  bool SkyBase::IsInitialized() { return m_initialized; }

  MaterialPtr SkyBase::GetSkyboxMaterial() { return m_skyboxMaterial; }

  CubeMapPtr SkyBase::GetIrradianceMap()
  {
    HdriPtr hdri = GetHdri();
    return hdri->m_diffuseEnvMap;
  }

  HdriPtr SkyBase::GetHdri()
  {
    HdriPtr hdri = GetComponent<EnvironmentComponent>()->GetHdriVal();
    return hdri;
  }

  BoundingBox SkyBase::GetBoundingBox(bool inWorld) { return unitBox; }

  bool SkyBase::ReadyToRender()
  {
    if (EnvironmentComponentPtr envComp = GetComponent<EnvironmentComponent>())
    {
      if (HdriPtr hdri = envComp->GetHdriVal())
      {
        return hdri->m_initiated;
      }
    }

    return false;
  }

  void SkyBase::ParameterConstructor()
  {
    Super::ParameterConstructor();

    DrawSky_Define(true, "Sky", 90, true, true);
    Illuminate_Define(true, "Sky", 90, true, true);
    Intensity_Define(1.0f, "Sky", 90, true, true, {false, true, 0.0f, 100000.0f, 0.1f});

    auto createParameterVariantFn = [](const String& name, int val)
    {
      ParameterVariant param {val};
      param.m_name = name;
      return param;
    };

    MultiChoiceVariant mcv = {
        {createParameterVariantFn("256", 256),
         createParameterVariantFn("512", 512),
         createParameterVariantFn("1024", 1024),
         createParameterVariantFn("2048", 2048),
         createParameterVariantFn("4096", 4096)},
        1
    };

    IBLTextureSize_Define(mcv, "Sky", 90, true, true);

    SetNameVal("SkyBase");
  }

  void SkyBase::ParameterEventConstructor()
  {
    Super::ParameterEventConstructor();

    ParamIBLTextureSize().GetVar<MultiChoiceVariant>().CurrentVal.Callback = [&](Value& oldVal, Value& newVal)
    {
      // Pass parameter to environment component.
      if (EnvironmentComponentPtr ec = GetComponent<EnvironmentComponent>())
      {
        MultiChoiceVariant& self = ec->ParamIBLTextureSize().GetVar<MultiChoiceVariant>();
        self.CurrentVal          = std::get<uint>(newVal);
      }
    };

    ParamIlluminate().m_onValueChangedFn.clear();
    ParamIlluminate().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          if (IsInitialized())
          {
            GetComponent<EnvironmentComponent>()->SetIlluminateVal(std::get<bool>(newVal));
          }
        });

    ParamIntensity().m_onValueChangedFn.clear();
    ParamIntensity().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          if (IsInitialized())
          {
            GetComponent<EnvironmentComponent>()->SetIntensityVal(std::get<float>(newVal));
          }
        });
  }

  void SkyBase::ConstructSkyMaterial(ShaderPtr vertexPrg, ShaderPtr fragPrg)
  {
    m_skyboxMaterial                             = MakeNewPtr<Material>();
    m_skyboxMaterial->m_isShaderMaterial         = true;
    m_skyboxMaterial->m_cubeMap                  = GetHdri()->m_cubemap;
    m_skyboxMaterial->m_vertexShader             = vertexPrg;
    m_skyboxMaterial->m_fragmentShader           = fragPrg;
    m_skyboxMaterial->GetRenderState()->cullMode = CullingType::TwoSided;
    m_skyboxMaterial->Init();
  }

  XmlNode* SkyBase::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  TKDefineClass(Sky, SkyBase);

  Sky::Sky() {}

  Sky::~Sky() {}

  void Sky::Init()
  {
    if (m_initialized)
    {
      return;
    }

    SkyBase::Init();

    // Skybox material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(ShaderPath("skyboxVert.shader", true));

    ShaderPtr frag = GetShaderManager()->Create<Shader>(ShaderPath("skyboxFrag.shader", true));

    ConstructSkyMaterial(vert, frag);

    m_initialized = true;
  }

  MaterialPtr Sky::GetSkyboxMaterial()
  {
    Init();

    HdriPtr hdri = GetHdri();
    hdri->Init();
    m_skyboxMaterial->m_cubeMap = hdri->m_cubemap;
    return m_skyboxMaterial;
  }

  void Sky::ParameterConstructor()
  {
    Super::ParameterConstructor();

    Exposure_Define(1.0f, "Sky", 90, true, true, {false, true, 0.0f, 50.0f, 0.05f});

    Hdri_Define(nullptr, "Sky", 90, true, true);

    SetNameVal("Sky");

    ParamVisible().m_exposed = false;
  }

  void Sky::ParameterEventConstructor()
  {
    SkyBase::ParameterEventConstructor();

    ParamHdri().m_onValueChangedFn.clear();
    ParamHdri().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          EnvironmentComponentPtr environmentCom = GetComponent<EnvironmentComponent>();
          environmentCom->SetHdriVal(std::get<HdriPtr>(newVal));
        });

    ParamExposure().m_onValueChangedFn.clear();
    ParamExposure().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          EnvironmentComponentPtr environmentCom = GetComponent<EnvironmentComponent>();
          environmentCom->SetExposureVal(std::get<float>(newVal));
        });
  }

  XmlNode* Sky::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

} // namespace ToolKit
