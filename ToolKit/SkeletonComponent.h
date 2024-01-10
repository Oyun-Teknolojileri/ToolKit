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

  static VariantCategory SkeletonComponentCategory {"Skeleton Component", 90};
  typedef std::shared_ptr<class SkeletonComponent> SkeletonComponentPtr;

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

    inline uint GetAnimFirstKeyFrame() const { return m_animFirstKeyFrame; }

    inline uint GetAnimSecondKeyFrame() const { return m_animSecondKeyFrame; }

    inline float GetAnimKeyFrameInterpolateTime() const { return m_animKeyFrameInterpolationTime; }

    inline uint GetAnimKeyFrameCount() const { return m_animKeyFrameCount; }

   protected:
    void ParameterConstructor() override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   public:
    TKDeclareParam(SkeletonPtr, SkeletonResource);

    DynamicBoneMapPtr m_map = nullptr;
    bool isDirty            = true;

   private:
    // These are per-frame variables for animation
    uint m_animFirstKeyFrame              = 0;
    uint m_animSecondKeyFrame             = 0;
    float m_animKeyFrameInterpolationTime = 0.0f;
    uint m_animKeyFrameCount              = 1;
  };

} // namespace ToolKit
