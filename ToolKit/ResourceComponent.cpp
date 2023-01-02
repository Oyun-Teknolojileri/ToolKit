#include "ResourceComponent.h"

#include "Animation.h"
#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"

#include <utility>

#include "DebugNew.h"

namespace ToolKit
{

  MeshComponent::MeshComponent()
  {
    Mesh_Define(std::make_shared<ToolKit::Mesh>(),
                MeshComponentCategory.Name,
                MeshComponentCategory.Priority,
                true,
                true);

    CastShadow_Define(true,
                      MeshComponentCategory.Name,
                      MeshComponentCategory.Priority,
                      true,
                      true);
  }

  MeshComponent::~MeshComponent() {}

  ComponentPtr MeshComponent::Copy(Entity* ntt)
  {
    MeshComponentPtr mc = std::make_shared<MeshComponent>();
    mc->m_localData     = m_localData;
    mc->m_entity        = ntt;
    return mc;
  }

  BoundingBox MeshComponent::GetAABB()
  {
    SkeletonComponentPtr skelComp = m_entity->GetComponent<SkeletonComponent>();
    if (skelComp && GetMeshVal()->IsSkinned())
    {
      SkinMesh* skinMesh = (SkinMesh*) GetMeshVal().get();
      if (skelComp->isDirty)
      {
        m_aabb =
            skinMesh->CalculateAABB(skelComp->GetSkeletonResourceVal().get(),
                                    skelComp->m_map);
        skelComp->isDirty = false;
      }
      return m_aabb;
    }
    return GetMeshVal()->m_aabb;
  }

  void MeshComponent::Init(bool flushClientSideArray)
  {
    GetMeshVal()->Init(flushClientSideArray);
  }

  MaterialComponent::MaterialComponent()
  {
    Material_Define(nullptr,
                    MaterialComponentCategory.Name,
                    MaterialComponentCategory.Priority,
                    true,
                    true);
  }

  MaterialComponent::~MaterialComponent() {}

  void MaterialComponent::Init(bool flushClientSideArray)
  {
    GetMaterialVal()->Init(flushClientSideArray);
  }

  ComponentPtr MaterialComponent::Copy(Entity* ntt)
  {
    MaterialComponentPtr mc = std::make_shared<MaterialComponent>();
    mc->m_localData         = m_localData;
    mc->m_entity            = ntt;

    return mc;
  }

  EnvironmentComponent::EnvironmentComponent()
  {
    ParameterConstructor();
    ParameterEventConstructor();
  }

  EnvironmentComponent::~EnvironmentComponent() {}

  void EnvironmentComponent::Init(bool flushClientSideArray)
  {
    assert(GetHdriVal() != nullptr &&
           "Attempt to initialize hdri resource "
           "that does not exist in environment component.");

    GetHdriVal()->m_exposure = GetExposureVal();
    GetHdriVal()->Init(flushClientSideArray);
  }

  void EnvironmentComponent::ParameterConstructor()
  {
    Hdri_Define(nullptr,
                EnvironmentComponentCategory.Name,
                EnvironmentComponentCategory.Priority,
                true,
                true);

    PositionOffset_Define(Vec3(-4.0f),
                          EnvironmentComponentCategory.Name,
                          EnvironmentComponentCategory.Priority,
                          true,
                          true,
                          {false, true, 0.0f, 100000.0f, 0.5f});

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

    Intensity_Define(0.25f,
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

    ParamExposure().m_onValueChangedFn.push_back(
        [this, reInitHdriFn](Value& oldVal, Value& newVal) -> void
        { reInitHdriFn(GetHdriVal(), std::get<float>(newVal)); });

    ParamHdri().m_onValueChangedFn.push_back(
        [this, reInitHdriFn](Value& oldVal, Value& newVal) -> void
        { reInitHdriFn(std::get<HdriPtr>(newVal), GetExposureVal()); });
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
      static Vec3 pos;
      pos = m_entity->m_node->GetTranslation(TransformationSpace::TS_WORLD);
    }
    aabb.min = GetPositionOffsetVal() + pos;
    aabb.max = GetPositionOffsetVal() + GetSizeVal() + pos;
    return aabb;
  }

  AnimControllerComponent::AnimControllerComponent()
  {
    Records_Define({},
                   AnimRecordComponentCategory.Name,
                   AnimRecordComponentCategory.Priority,
                   true,
                   true);

    m_id = GetHandleManager()->GetNextHandle();
  }

  AnimControllerComponent::~AnimControllerComponent()
  {
    if (activeRecord != nullptr)
    {
      GetAnimationPlayer()->RemoveRecord(activeRecord->m_id);
    }
  }

  ComponentPtr AnimControllerComponent::Copy(Entity* ntt)
  {
    AnimControllerComponentPtr ec = std::make_shared<AnimControllerComponent>();
    ec->m_localData               = m_localData;
    ec->m_entity                  = ntt;
    for (auto& record : ec->ParamRecords().GetVar<AnimRecordPtrMap>())
    {
      AnimRecordPtr newRecord = std::make_shared<AnimRecord>();
      ULongID p_id            = newRecord->m_id;
      *newRecord              = *record.second;
      newRecord->m_id         = p_id;
      newRecord->m_entity     = ntt;
      record.second           = newRecord;
    }

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

  void AnimControllerComponent::AddSignal(const String& signalName,
                                          AnimRecordPtr record)
  {
    ParamRecords().GetVar<AnimRecordPtrMap>().insert(
        std::make_pair(signalName, record));
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
    AnimRecordPtr& rec     = list[signalName];
    if (rec == nullptr)
    {
      return;
    }

    if (activeRecord)
    {
      activeRecord->m_state = AnimRecord::State::Stop;
    }
    rec->m_state  = AnimRecord::State::Play;
    rec->m_loop   = true;
    rec->m_entity = m_entity;
    activeRecord  = rec;
    GetAnimationPlayer()->AddRecord(rec.get());
  }

  void AnimControllerComponent::Stop()
  {
    if (activeRecord)
    {
      activeRecord->m_state = AnimRecord::State::Stop;
      activeRecord          = nullptr;
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

  AnimRecordPtr AnimControllerComponent::GetAnimRecord(const String& signalName)
  {
    AnimRecordPtrMap& records = ParamRecords().GetVar<AnimRecordPtrMap>();
    const auto& recordIter    = records.find(signalName);
    if (recordIter != records.end())
    {
      return recordIter->second;
    }
    return nullptr;
  }

  SkeletonComponent::SkeletonComponent()
  {
    SkeletonResource_Define(nullptr,
                            SkeletonComponentCategory.Name,
                            SkeletonComponentCategory.Priority,
                            true,
                            true);

    m_map = nullptr;
  }

  SkeletonComponent::~SkeletonComponent()
  {
    if (m_map)
    {
      delete m_map;
    }
  }

  void SkeletonComponent::Init()
  {
    const SkeletonPtr& resource = GetSkeletonResourceVal();
    if (resource == nullptr)
    {
      return;
    }

    m_map = new DynamicBoneMap;
    m_map->Init(resource.get());
  }

  ComponentPtr SkeletonComponent::Copy(Entity* ntt)
  {
    SkeletonComponentPtr dst = std::make_shared<SkeletonComponent>();
    dst->m_entity            = ntt;

    dst->SetSkeletonResourceVal(GetSkeletonResourceVal());
    dst->Init();

    return dst;
  }

  void SkeletonComponent::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Component::DeSerialize(doc, parent);
    m_map = new DynamicBoneMap;
    m_map->Init(GetSkeletonResourceVal().get());
  }

  MultiMaterialComponent::MultiMaterialComponent() {}

  MultiMaterialComponent::~MultiMaterialComponent() {}

  ComponentPtr MultiMaterialComponent::Copy(Entity* ntt)
  {
    MultiMaterialPtr mc = std::make_shared<MultiMaterialComponent>();
    mc->m_localData     = m_localData;
    mc->m_entity        = ntt;
    mc->materials       = materials;

    return mc;
  }

  void MultiMaterialComponent::Init(bool flushClientSideArray) {}

  const char *XmlMatCountAttrib = "MaterialCount", *XmlMatIdAttrib = "ID";

  void MultiMaterialComponent::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Component::DeSerialize(doc, parent);
    uint matCount = 0;
    ReadAttr(parent, XmlMatCountAttrib, matCount);
    materials.resize(matCount);
    for (uint i = 0; i < materials.size(); i++)
    {
      XmlNode* resourceNode = parent->first_node(std::to_string(i).c_str());
      if (!resourceNode)
      {
        materials[i] = GetMaterialManager()->GetCopyOfDefaultMaterial();
        continue;
      }
      materials[i] = GetMaterialManager()->Create<Material>(
          MaterialPath(Resource::DeserializeRef(resourceNode)));
    }
  }

  void MultiMaterialComponent::Serialize(XmlDocument* doc,
                                         XmlNode* parent) const
  {
    Component::Serialize(doc, parent);
    XmlNode* compNode = parent->last_node(XmlComponent.c_str());
    WriteAttr(compNode,
              doc,
              XmlMatCountAttrib,
              std::to_string(materials.size()));
    for (uint i = 0; i < materials.size(); i++)
    {
      XmlNode* resourceRefNode =
          CreateXmlNode(doc, std::to_string(i), compNode);
      materials[i]->SerializeRef(doc, resourceRefNode);
    }
  }

  void MultiMaterialComponent::AddMaterial(MaterialPtr mat)
  {
    materials.push_back(mat);
  }

  void MultiMaterialComponent::RemoveMaterial(uint index)
  {
    assert(materials.size() >= index && "Material List overflow");
    materials.erase(materials.begin() + index);
  }

  const MaterialPtrArray& MultiMaterialComponent::GetMaterialList() const
  {
    return materials;
  }

  MaterialPtrArray& MultiMaterialComponent::GetMaterialList()
  {
    return materials;
  }

  void MultiMaterialComponent::UpdateMaterialList(MeshComponentPtr meshComp)
  {
    if (meshComp == nullptr || meshComp->GetMeshVal() == nullptr)
    {
      return;
    }
    MeshRawPtrArray meshCollector;
    meshComp->GetMeshVal()->GetAllMeshes(meshCollector);
    materials.resize(meshCollector.size());
    for (uint i = 0; i < meshCollector.size(); i++)
    {
      materials[i] = meshCollector[i]->m_material;
    }
  }

  AABBOverrideComponent::AABBOverrideComponent()
  {
    PositionOffset_Define(Vec3(0),
                          AABBOverrideCompCategory.Name,
                          AABBOverrideCompCategory.Priority,
                          true,
                          true);

    Size_Define(Vec3(1),
                AABBOverrideCompCategory.Name,
                AABBOverrideCompCategory.Priority,
                true,
                true);
  }

  AABBOverrideComponent::~AABBOverrideComponent() {}

  ComponentPtr AABBOverrideComponent::Copy(Entity* ntt)
  {
    AABBOverrideComponentPtr dst = std::make_shared<AABBOverrideComponent>();
    dst->m_entity                = ntt;
    dst->m_localData             = m_localData;

    return dst;
  }

  void AABBOverrideComponent::Init(bool flushClientSideArray) {}

  BoundingBox AABBOverrideComponent::GetAABB()
  {
    BoundingBox aabb = {};
    aabb.min         = GetPositionOffsetVal();
    aabb.max         = GetPositionOffsetVal() + GetSizeVal();
    return aabb;
  }

  void AABBOverrideComponent::SetAABB(BoundingBox aabb)
  {
    SetPositionOffsetVal(aabb.min);
    SetSizeVal(aabb.max - aabb.min);
  }

} //  namespace ToolKit
