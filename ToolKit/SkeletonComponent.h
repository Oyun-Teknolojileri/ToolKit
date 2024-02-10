/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Component.h"
#include "Skeleton.h"

namespace ToolKit
{
  struct AnimData
  {
    float firstKeyFrame                  = 0.0f; // normalized via (firstKeyFrame / keyFrameCount)
    float secondKeyFrame                 = 0.0f; // normalized via (firstKeyFrame / keyFrameCount)
    float keyFrameInterpolationTime      = 0.0f;
    float keyFrameCount                  = 1.0f; // default value is 1 to prevent division with 0

    AnimationPtr currentAnimation        = nullptr;
    AnimationPtr blendAnimation          = nullptr;
    float animationBlendFactor           = 0.0f; // between 0 - 1
    float blendFirstKeyFrame             = 0.0f; // normalized via (firstKeyFrame / keyFrameCount)
    float blendSecondKeyFrame            = 0.0f; // normalized via (firstKeyFrame / keyFrameCount)
    float blendKeyFrameInterpolationTime = 0.0f;
    float blendKeyFrameCount             = 1.0f; // default value is 1 to prevent division with 0
  };

  static VariantCategory SkeletonComponentCategory {"Skeleton Component", 90};

  /**
   * The component that stores skeleton resource reference and dynamic bone
      transformation info
   */
  class TK_API SkeletonComponent : public Component
  {
    friend class AnimationPlayer;
    friend class RenderJobProcessor;

   public:
    TKDeclareClass(SkeletonComponent, Component);

    /**
     * Empty constructor.
     */
    SkeletonComponent();
    virtual ~SkeletonComponent();

    void Init();
    ComponentPtr Copy(EntityPtr ntt) override;

    const AnimData& GetAnimData() const;

   protected:
    void ParameterConstructor() override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   public:
    TKDeclareParam(SkeletonPtr, SkeletonResource);

    DynamicBoneMapPtr m_map = nullptr;
    bool isDirty            = true;

   private:
    AnimData m_animData;
  };

} // namespace ToolKit
