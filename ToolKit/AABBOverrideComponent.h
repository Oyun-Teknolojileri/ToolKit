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

  typedef std::shared_ptr<class AABBOverrideComponent> AABBOverrideComponentPtr;
  typedef std::vector<AABBOverrideComponentPtr> AABBOverrideComponentPtrArray;

  static VariantCategory AABBOverrideCompCategory {"AABB Override Component", 90};

  class TK_API AABBOverrideComponent : public Component
  {
   public:
    TKDeclareClass(AABBOverrideComponent, Component);

    AABBOverrideComponent();
    virtual ~AABBOverrideComponent();

    /**
     * Creates a copy of the AABB Override Component.
     * @param ntt Parent Entity of the component.
     * @return Copy of the AABBOverrideComponent.
     */
    ComponentPtr Copy(EntityPtr ntt) override;

    void Init(bool flushClientSideArray);
    BoundingBox GetBoundingBox();

    // AABB should be in entity space (not world space)
    void SetBoundingBox(BoundingBox aabb);

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

    private:
    void InvalidateSpatialCaches();

   private:
    TKDeclareParam(Vec3, PositionOffset);
    TKDeclareParam(Vec3, Size);
  };
} //  namespace ToolKit
