#include "AnimationControllerComponent.h"

#include "Animation.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

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

} // namespace ToolKit