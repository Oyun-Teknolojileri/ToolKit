/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Sky.h"

#include "DirectionComponent.h"
#include "EnvironmentComponent.h"
#include "GradientSky.h"
#include "Material.h"
#include "Shader.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(SkyBase, Entity);

  SkyBase::SkyBase() {}

  void SkyBase::NativeConstruct()
  {
    Super::NativeConstruct();
  }

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
      envComp      = AddComponent<EnvironmentComponent>();
      HdriPtr hdri = nullptr;

      // Provide an empty hdri to construct gradient sky.
      if (IsA<GradientSky>())
      {
        hdri = MakeNewPtr<Hdri>();
      }
      else // Use default hdri image.
      {
        TextureManager* tman = GetTextureManager();
        hdri                 = tman->Create<Hdri>(tman->GetDefaultResource(Hdri::StaticClass()));
        hdri->Init();
      }

      envComp->SetHdriVal(hdri);
    }

    Vec3 mp = Vec3(TK_FLT_MAX);
    envComp->SetSizeVal(mp);
    envComp->m_entity = m_sharedEntity;
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
    Super::ParameterConstructor();

    DrawSky_Define(true, "Sky", 90, true, true);
    Illuminate_Define(true, "Sky", 90, true, true);
    Intensity_Define(1.0f, "Sky", 90, true, true, {false, true, 0.0f, 100000.0f, 0.1f});

    SetNameVal("SkyBase");
  }

  void SkyBase::ParameterEventConstructor()
  {
    Super::ParameterEventConstructor();

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
    m_skyboxMaterial                             = std::make_shared<Material>();
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

    HdriPtr hdri                = GetHdri();
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
