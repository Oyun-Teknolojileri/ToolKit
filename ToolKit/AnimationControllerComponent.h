#pragma once

#include "Component.h"

namespace ToolKit
{

  static VariantCategory AnimRecordComponentCategory {
      "Animation Record Component",
      90};
  typedef std::shared_ptr<class AnimControllerComponent>
      AnimControllerComponentPtr;

  /**
   * The component that stores animation records for the entity
   */
  class TK_API AnimControllerComponent : public Component
  {
   public:
    TKComponentType(AnimControllerComponent);

    /**
     * Empty constructor.
     */
    AnimControllerComponent();
    virtual ~AnimControllerComponent();

    ComponentPtr Copy(Entity* ntt) override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    void Play(const String& recordName, const float blendFactor = 0.0f);
    void Stop();
    void Pause();
    AnimRecordPtr GetActiveRecord();
    AnimRecordPtr GetAnimRecord(const String& signalName);
    void AddSignal(const String& signalName, AnimRecordPtr record);
    void RemoveSignal(const String& signalName);

   public:
    TKDeclareParam(AnimRecordPtrMap, Records);

   private:
    AnimRecordPtr activeRecord;
  };

} // namespace ToolKit