#include "ResourceComponent.h"
#include <utility>
#include "Mesh.h"
#include "Material.h"
#include "Entity.h"
#include "ToolKit.h"
#include "Animation.h"

namespace ToolKit
{

  MeshComponent::MeshComponent()
  {
    Mesh_Define
    (
      std::make_shared<ToolKit::Mesh>(),
      MeshComponentCategory.Name,
      MeshComponentCategory.Priority,
      true,
      true
    );
  }

  MeshComponent::~MeshComponent()
  {
  }

  ComponentPtr MeshComponent::Copy(Entity* ntt)
  {
    MeshComponentPtr mc = std::make_shared<MeshComponent>();
    mc->m_localData = m_localData;
    mc->m_entity = ntt;

    return mc;
  }

  BoundingBox MeshComponent::GetAABB()
  {
    return GetMeshVal()->m_aabb;
  }

  void MeshComponent::Init(bool flushClientSideArray)
  {
    GetMeshVal()->Init(flushClientSideArray);
  }

  MaterialComponent::MaterialComponent()
  {
    Material_Define
    (
      nullptr,
      MaterialComponentCategory.Name,
      MaterialComponentCategory.Priority,
      true,
      true
    );
  }

  MaterialComponent::~MaterialComponent()
  {
  }

  void MaterialComponent::Init(bool flushClientSideArray)
  {
    GetMaterialVal()->Init(flushClientSideArray);
  }

  ComponentPtr MaterialComponent::Copy(Entity* ntt)
  {
    MaterialComponentPtr mc = std::make_shared<MaterialComponent>();
    mc->m_localData = m_localData;
    mc->m_entity = ntt;

    return mc;
  }

  EnvironmentComponent::EnvironmentComponent()
  {
    ParameterConstructor();
    ParameterEventConstructor();

    m_bbox = new BoundingBox();
    UpdateBBox();
  }

  EnvironmentComponent::~EnvironmentComponent()
  {
    SafeDel(m_bbox);
  }

  void EnvironmentComponent::Init(bool flushClientSideArray)
  {
    assert
    (
      GetHdriVal() != nullptr && "Attempt to initialize hdri resource "
      "that does not exist in environment component."
    );

    GetHdriVal()->m_exposure = GetExposureVal();
    GetHdriVal()->Init(flushClientSideArray);
  }

  void EnvironmentComponent::ParameterConstructor()
  {
    Hdri_Define
    (
      nullptr,
      EnvironmentComponentCategory.Name,
      EnvironmentComponentCategory.Priority,
      true,
      true
    );

    Max_Define
    (
      Vec3(4.0f),
      EnvironmentComponentCategory.Name,
      EnvironmentComponentCategory.Priority,
      true,
      true
    );

    Min_Define
    (
      Vec3(-4.0f),
      EnvironmentComponentCategory.Name,
      EnvironmentComponentCategory.Priority,
      true,
      true
    );

    Illuminate_Define
    (
      true,
      EnvironmentComponentCategory.Name,
      EnvironmentComponentCategory.Priority,
      true,
      true
    );

    Intensity_Define
    (
      0.25f,
      EnvironmentComponentCategory.Name,
      EnvironmentComponentCategory.Priority,
      true,
      true
    );
    Exposure_Define
    (
      1.0f,
      EnvironmentComponentCategory.Name,
      EnvironmentComponentCategory.Priority,
      true,
      true
    );
  }

  void EnvironmentComponent::ParameterEventConstructor()
  {
    auto reInitHdriFn = [](HdriPtr hdri, float exposure)
    {
      hdri->UnInit();
      hdri->Load();
      hdri->m_exposure = exposure;
      hdri->Init(true);
    };

    ParamExposure().m_onValueChangedFn =
    [this, reInitHdriFn](Value& oldVal, Value& newVal) -> void
    {
      reInitHdriFn(GetHdriVal(), std::get<float>(newVal));
    };

    ParamHdri().m_onValueChangedFn =
    [this, reInitHdriFn](Value& oldVal, Value& newVal) -> void
    {
      reInitHdriFn(std::get<HdriPtr>(newVal), GetExposureVal());
    };
  }

  ComponentPtr EnvironmentComponent::Copy(Entity* ntt)
  {
    EnvironmentComponentPtr ec = std::make_shared<EnvironmentComponent>();
    ec->m_localData = m_localData;
    ec->m_entity = ntt;
    ec->UpdateBBox();

    return ec;
  }

  void EnvironmentComponent::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Component::Serialize(doc, parent);
  }

  void EnvironmentComponent::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Component::DeSerialize(doc, parent);
    UpdateBBox();
    ParameterEventConstructor();
  }

  void EnvironmentComponent::UpdateBBox()
  {
    if (m_entity != nullptr)
    {
      static Vec3 pos;
      pos = m_entity->m_node->GetTranslation(TransformationSpace::TS_WORLD);
      m_bbox->max = GetMaxVal() + pos;
      m_bbox->min = GetMinVal() + pos;
    }
    else
    {
      m_bbox->max = GetMaxVal();
      m_bbox->min = GetMinVal();
    }
  }

  BoundingBox* EnvironmentComponent::GetBBox()
  {
    UpdateBBox();
    return m_bbox;
  }

  Vec3 EnvironmentComponent::GetBBoxMin()
  {
    UpdateBBox();
    return m_bbox->min;
  }

  Vec3 EnvironmentComponent::GetBBoxMax()
  {
    UpdateBBox();
    return m_bbox->max;
  }


  AnimControllerComponent::AnimControllerComponent()
  {
    Records_Define
    (
      {},
      AnimRecordComponentCategory.Name,
      AnimRecordComponentCategory.Priority,
      true,
      true
    );

    m_id = GetHandleManager()->GetNextHandle();
  }
  AnimControllerComponent::~AnimControllerComponent()
  {
    Stop();
  }

  ComponentPtr AnimControllerComponent::Copy(Entity* ntt)
  {
    AnimControllerComponentPtr ec =
      std::make_shared<AnimControllerComponent>();
    ec->m_localData = m_localData;
    ec->m_entity = ntt;

    return ec;
  }

  void AnimControllerComponent::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Component::DeSerialize(doc, parent);
    AnimRecordPtrMap& list = ParamRecords().GetVar<AnimRecordPtrMap>();
    for (auto iter = list.begin(); iter != list.end(); ++iter)
    {
      iter->second->m_entity = m_entity;
    }
  }

  void AnimControllerComponent::AddSignal
  (
    const String& signalName,
    AnimRecordPtr record
  )
  {
    ParamRecords().GetVar<AnimRecordPtrMap>().insert
    (
      std::make_pair(signalName, record)
    );
  }

  void AnimControllerComponent::RemoveSignal(const String& signalName)
  {
    const auto& signal = GetRecordsVal().find(signalName);
    if (signal == GetRecordsVal().end())
    {
      return;
    }

    GetAnimationPlayer()->RemoveRecord(signal->second->m_id);
    ParamRecords().GetVar<AnimRecordPtrMap>().erase(signalName);
  }

  void AnimControllerComponent::Play(const String& signalName)
  {
    AnimRecordPtrMap& list = ParamRecords().GetVar<AnimRecordPtrMap>();
    AnimRecordPtr& rec = list[signalName];
    if (rec == nullptr)
    {
      return;
    }

    if (activeRecord)
    {
      activeRecord->m_state = AnimRecord::State::Stop;
    }
    rec->m_state = AnimRecord::State::Play;
    rec->m_loop = true;
    rec->m_entity = m_entity;
    activeRecord = rec;
    GetAnimationPlayer()->AddRecord(rec.get());
  }

  void AnimControllerComponent::Stop()
  {
    if (activeRecord)
    {
      activeRecord->m_state = AnimRecord::State::Stop;
      activeRecord = nullptr;
    }
  }

  void AnimControllerComponent::Pause()
  {
    activeRecord->m_state = AnimRecord::State::Pause;
  }
  AnimRecordPtr AnimControllerComponent::GetActiveRecord()
  {
    return activeRecord;
  }
}  //  namespace ToolKit
