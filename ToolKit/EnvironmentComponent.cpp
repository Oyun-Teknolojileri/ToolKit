/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "EnvironmentComponent.h"

#include "Entity.h"
#include "MathUtil.h"
#include "Texture.h"

namespace ToolKit
{

  TKDefineClass(EnvironmentComponent, Component);

  EnvironmentComponent::EnvironmentComponent() {}

  EnvironmentComponent::~EnvironmentComponent() { UnInit(); }

  void EnvironmentComponent::Init(bool flushClientSideArray)
  {
    if (m_initialized)
    {
      return;
    }

    HdriPtr hdri = GetHdriVal();
    assert(hdri != nullptr && "Attempt to initialize hdri resource "
                              "that does not exist in environment component.");

    if (!hdri->IsDynamic())
    {
      hdri->Load();
    }

    hdri->m_specularIBLTextureSize = GetIBLTextureSizeVal().GetValue<int>();
    hdri->m_exposure               = GetExposureVal();
    hdri->Init(flushClientSideArray);

    UpdateBoundingBoxCache();
    m_initialized = true;
  }

  void EnvironmentComponent::UnInit() { m_initialized = false; }

  void EnvironmentComponent::ParameterConstructor()
  {
    Super::ParameterConstructor();

    Hdri_Define(nullptr, EnvironmentComponentCategory.Name, EnvironmentComponentCategory.Priority, true, true);

    PositionOffset_Define(Vec3(0.0f),
                          EnvironmentComponentCategory.Name,
                          EnvironmentComponentCategory.Priority,
                          true,
                          true,
                          {false, true, -FLT_MAX, FLT_MAX, 0.5f});

    Size_Define(Vec3(8.0f),
                EnvironmentComponentCategory.Name,
                EnvironmentComponentCategory.Priority,
                true,
                true,
                {false, true, 0.0f, 100000.0f, 0.5f});

    Illuminate_Define(true, EnvironmentComponentCategory.Name, EnvironmentComponentCategory.Priority, true, true);

    Intensity_Define(1.0f,
                     EnvironmentComponentCategory.Name,
                     EnvironmentComponentCategory.Priority,
                     true,
                     true,
                     {false, true, 0.0f, 100000.0f, 0.1f});

    Exposure_Define(1.0f,
                    EnvironmentComponentCategory.Name,
                    EnvironmentComponentCategory.Priority,
                    true,
                    true,
                    {false, true, 0.0f, 50.0f, 0.05f});

    auto createParameterVariant = [](const String& name, int val)
    {
      ParameterVariant param {val};
      param.m_name = name;
      return param;
    };

    MultiChoiceVariant mcv = {
        {createParameterVariant("256", 256),
         createParameterVariant("512", 512),
         createParameterVariant("1024", 1024),
         createParameterVariant("2048", 2048),
         createParameterVariant("4096", 4096)},
        1
    };

    IBLTextureSize_Define(mcv,
                          EnvironmentComponentCategory.Name,
                          EnvironmentComponentCategory.Priority,
                          true,
                          true,
                          {false, false});
  }

  void EnvironmentComponent::ParameterEventConstructor()
  {
    Super::ParameterEventConstructor();

    ParamIBLTextureSize().GetVar<MultiChoiceVariant>().CurrentVal.Callback = [&](Value& oldVal, Value& newVal)
    {
      HdriPtr hdri       = GetHdriVal();
      int iblTexSize     = std::get<uint>(newVal);
      int prevIblTexSize = std::get<uint>(oldVal);

      if (hdri != nullptr && iblTexSize != prevIblTexSize)
      {
        MultiChoiceVariant& self       = ParamIBLTextureSize().GetVar<MultiChoiceVariant>();
        hdri->m_specularIBLTextureSize = self.GetValue<int>();
        ReInitHdri(hdri, GetExposureVal());
      }
    };

    ParamExposure().m_onValueChangedFn.clear();
    ParamExposure().m_onValueChangedFn.push_back([this](Value& oldVal, Value& newVal) -> void
                                                 { ReInitHdri(GetHdriVal(), std::get<float>(newVal)); });

    ParamPositionOffset().m_onValueChangedFn.push_back([this](Value& oldVal, Value& newVal) -> void
                                                       { m_spatialCachesInvalidated = true; });

    ParamSize().m_onValueChangedFn.push_back([this](Value& oldVal, Value& newVal) -> void
                                             { m_spatialCachesInvalidated = true; });
  }

  ComponentPtr EnvironmentComponent::Copy(EntityPtr ntt)
  {
    EnvironmentComponentPtr ec = MakeNewPtr<EnvironmentComponent>();
    ec->m_localData            = m_localData;
    ec->m_entity               = ntt;

    return ec;
  }

  XmlNode* EnvironmentComponent::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* compNode = Super::DeSerializeImp(info, parent);
    ParameterEventConstructor();

    return compNode->first_node(StaticClass()->Name.c_str());
  }

  XmlNode* EnvironmentComponent::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    if (!m_serializableComponent)
    {
      return root;
    }

    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  const BoundingBox& EnvironmentComponent::GetBoundingBox()
  {
    if (m_spatialCachesInvalidated)
    {
      UpdateBoundingBoxCache();
    }

    return m_boundingBoxCache;
  }

  void EnvironmentComponent::ReInitHdri(HdriPtr hdri, float exposure)
  {
    hdri->UnInit();
    hdri->Load();
    hdri->m_exposure = exposure;
    hdri->Init(true);
  }

  void EnvironmentComponent::UpdateBoundingBoxCache()
  {
    Vec3 pos;
    if (EntityPtr owner = OwnerEntity())
    {
      pos += owner->m_node->GetTranslation(TransformationSpace::TS_WORLD);
    }

    m_boundingBoxCache.min     = GetPositionOffsetVal() + pos - GetSizeVal() * 0.5f;
    m_boundingBoxCache.max     = GetPositionOffsetVal() + pos + GetSizeVal() * 0.5f;
    m_spatialCachesInvalidated = false;
  };

} // namespace ToolKit