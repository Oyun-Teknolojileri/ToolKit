/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Component.h"

namespace ToolKit
{

  static VariantCategory AnimRecordComponentCategory {"Animation Record Component", 90};
  typedef std::shared_ptr<class AnimControllerComponent> AnimControllerComponentPtr;

  /**
   * The component that stores animation records for the entity
   */
  class TK_API AnimControllerComponent : public Component
  {
   public:
    TKDeclareClass(AnimControllerComponent, Component);

    /**
     * Empty constructor.
     */
    AnimControllerComponent();
    virtual ~AnimControllerComponent();

    ComponentPtr Copy(EntityPtr ntt) override;

    void Play(const String& recordName);
    void Stop();
    void Pause();
    AnimRecordPtr GetActiveRecord();
    AnimRecordPtr GetAnimRecord(const String& signalName);
    void AddSignal(const String& signalName, AnimRecordPtr record);
    void RemoveSignal(const String& signalName);

    void AddAnimationToBlend(const String& animToBlendName, float blendDurationInSec);

   protected:
    void ParameterConstructor() override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   public:
    TKDeclareParam(AnimRecordPtrMap, Records);

   private:
    AnimRecordPtr activeRecord;
  };

} // namespace ToolKit