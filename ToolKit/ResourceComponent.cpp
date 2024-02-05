/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "ResourceComponent.h"

#include "Animation.h"
#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(AABBOverrideComponent, Component);

  AABBOverrideComponent::AABBOverrideComponent() {}

  AABBOverrideComponent::~AABBOverrideComponent() {}

  ComponentPtr AABBOverrideComponent::Copy(EntityPtr ntt)
  {
    AABBOverrideComponentPtr dst = MakeNewPtr<AABBOverrideComponent>();
    dst->m_entity                = ntt;
    dst->m_localData             = m_localData;

    return dst;
  }

  void AABBOverrideComponent::Init(bool flushClientSideArray) {}

  BoundingBox AABBOverrideComponent::GetBoundingBox()
  {
    BoundingBox aabb = {};
    aabb.min         = GetPositionOffsetVal();
    aabb.max         = GetPositionOffsetVal() + GetSizeVal();
    return aabb;
  }

  void AABBOverrideComponent::SetBoundingBox(BoundingBox aabb)
  {
    SetPositionOffsetVal(aabb.min);
    SetSizeVal(aabb.max - aabb.min);
  }

  void AABBOverrideComponent::ParameterConstructor()
  {
    Super::ParameterConstructor();

    PositionOffset_Define(Vec3(0.0f), AABBOverrideCompCategory.Name, AABBOverrideCompCategory.Priority, true, true);
    Size_Define(Vec3(1.0f), AABBOverrideCompCategory.Name, AABBOverrideCompCategory.Priority, true, true);
  }

  XmlNode* AABBOverrideComponent::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

} //  namespace ToolKit
