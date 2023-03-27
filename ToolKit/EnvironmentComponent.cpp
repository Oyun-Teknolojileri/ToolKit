#include "EnvironmentComponent.h"

#include "Entity.h"
#include "MathUtil.h"
#include "Texture.h"

#include "DebugNew.h"

namespace ToolKit
{

  EnvironmentComponent::EnvironmentComponent()
  {
    ParameterConstructor();
    ParameterEventConstructor();
  }

  EnvironmentComponent::~EnvironmentComponent() {}

  void EnvironmentComponent::Init(bool flushClientSideArray)
  {
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
  }

  void EnvironmentComponent::ParameterConstructor()
  {
    Hdri_Define(nullptr,
                EnvironmentComponentCategory.Name,
                EnvironmentComponentCategory.Priority,
                true,
                true);

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

    Illuminate_Define(true,
                      EnvironmentComponentCategory.Name,
                      EnvironmentComponentCategory.Priority,
                      true,
                      true);

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
        {createParameterVariant("32", 32),
         createParameterVariant("64", 64),
         createParameterVariant("128", 128),
         createParameterVariant("256", 256),
         createParameterVariant("512", 512),
         createParameterVariant("1024", 1024)},
        1,
        [&](Value& oldVal, Value& newVal)
        {
          HdriPtr hdri           = GetHdriVal();
          MultiChoiceVariant mcv = GetIBLTextureSizeVal();

          if (hdri != nullptr)
          {
            hdri->m_specularIBLTextureSize = mcv.GetValue<int>();
            ReInitHdri(hdri, GetExposureVal());
          }
         }
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
    ParamExposure().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        { ReInitHdri(GetHdriVal(), std::get<float>(newVal)); });

    ParamHdri().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        { ReInitHdri(std::get<HdriPtr>(newVal), GetExposureVal()); });
  }

  ComponentPtr EnvironmentComponent::Copy(Entity* ntt)
  {
    EnvironmentComponentPtr ec = std::make_shared<EnvironmentComponent>();
    ec->m_localData            = m_localData;
    ec->m_entity               = ntt;

    return ec;
  }

  void EnvironmentComponent::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Component::Serialize(doc, parent);
  }

  void EnvironmentComponent::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Component::DeSerialize(doc, parent);
    ParameterEventConstructor();
  }

  BoundingBox EnvironmentComponent::GetBBox()
  {
    Vec3 pos;
    BoundingBox aabb;
    if (m_entity != nullptr)
    {
      pos += m_entity->m_node->GetTranslation(TransformationSpace::TS_WORLD);
    }
    aabb.min = GetPositionOffsetVal() + pos - GetSizeVal() * 0.5f;
    aabb.max = GetPositionOffsetVal() + pos + GetSizeVal() * 0.5f;
    return aabb;
  }

  void EnvironmentComponent::ReInitHdri(HdriPtr hdri, float exposure)
  {
    hdri->UnInit();
    hdri->Load();
    hdri->m_exposure = exposure;
    hdri->Init(true);
  };

} // namespace ToolKit