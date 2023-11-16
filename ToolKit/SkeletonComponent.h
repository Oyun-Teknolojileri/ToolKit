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
   public:
    TKDeclareClass(SkeletonComponent, Component);

    /**
     * Empty constructor.
     */
    SkeletonComponent();
    virtual ~SkeletonComponent();

    void Init();
    ComponentPtr Copy(EntityPtr ntt) override;

   protected:
    void ParameterConstructor() override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   public:
    TKDeclareParam(SkeletonPtr, SkeletonResource);

    DynamicBoneMapPtr m_map = nullptr;
    bool isDirty            = true;
  };

} // namespace ToolKit