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

#include "AnimationControllerComponent.h"

#include "Animation.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(AnimControllerComponent, Component);

  AnimControllerComponent::AnimControllerComponent()
  {
    Records_Define({}, AnimRecordComponentCategory.Name, AnimRecordComponentCategory.Priority, true, true);
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

  void AnimControllerComponent::DeSerializeImp(XmlDocument* doc, XmlNode* parent)
  {
    Component::DeSerializeImp(doc, parent);
    AnimRecordPtrMap& list = ParamRecords().GetVar<AnimRecordPtrMap>();
    for (auto iter = list.begin(); iter != list.end(); ++iter)
    {
      iter->second->m_entity = m_entity;
    }
  }

  XmlNode* AnimControllerComponent::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  void AnimControllerComponent::AddSignal(const String& signalName, AnimRecordPtr record)
  {
    ParamRecords().GetVar<AnimRecordPtrMap>().insert(std::make_pair(signalName, record));
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

  void AnimControllerComponent::Pause() { activeRecord->m_state = AnimRecord::State::Pause; }

  AnimRecordPtr AnimControllerComponent::GetActiveRecord() { return activeRecord; }

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