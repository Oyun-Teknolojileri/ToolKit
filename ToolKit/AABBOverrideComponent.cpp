/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "AABBOverrideComponent.h"

#include "Animation.h"
#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "Scene.h"
#include "ToolKit.h"

namespace ToolKit
{

  TKDefineClass(AABBOverrideComponent, Component);

  AABBOverrideComponent::AABBOverrideComponent() {}

  AABBOverrideComponent::~AABBOverrideComponent() { InvalidateSpatialCaches(); }

  ComponentPtr AABBOverrideComponent::Copy(EntityPtr ntt)
  {
    AABBOverrideComponentPtr dst = MakeNewPtr<AABBOverrideComponent>();
    dst->m_entity                = ntt;
    dst->m_localData             = m_localData;

    return dst;
  }

  void AABBOverrideComponent::Init(bool flushClientSideArray) { InvalidateSpatialCaches(); }

  BoundingBox AABBOverrideComponent::GetBoundingBox()
  {
    BoundingBox aabb;
    aabb.min = GetPositionOffsetVal();
    aabb.max = GetPositionOffsetVal() + GetSizeVal();
    return aabb;
  }

  void AABBOverrideComponent::SetBoundingBox(const BoundingBox& aabb)
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

  void AABBOverrideComponent::ParameterEventConstructor()
  {
    Super::ParameterEventConstructor();

    auto invalidatefn = [this](Value& oldVal, Value& newVal) -> void { InvalidateSpatialCaches(); };
    ParamSize().m_onValueChangedFn.push_back(invalidatefn);
    ParamPositionOffset().m_onValueChangedFn.push_back(invalidatefn);
  }

  XmlNode* AABBOverrideComponent::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    if (!m_serializableComponent)
    {
      return root;
    }

    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

} //  namespace ToolKit
