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
  class DynamicBoneMap;

  class TK_API SkeletonComponent : public Component
  {
   public:
    TKComponentType(SkeletonComponent);

    /**
     * Empty constructor.
     */
    SkeletonComponent();
    virtual ~SkeletonComponent();
    void Init();

    ComponentPtr Copy(Entity* ntt) override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

   public:
    TKDeclareParam(SkeletonPtr, SkeletonResource);
    DynamicBoneMap* m_map;
    bool isDirty = true;
  };

} // namespace ToolKit