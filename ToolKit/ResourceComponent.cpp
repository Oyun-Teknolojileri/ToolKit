#include "ResourceComponent.h"

#include "Animation.h"
#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"

#include <utility>

#include "DebugNew.h"

namespace ToolKit
{

  AABBOverrideComponent::AABBOverrideComponent()
  {
    PositionOffset_Define(Vec3(0),
                          AABBOverrideCompCategory.Name,
                          AABBOverrideCompCategory.Priority,
                          true,
                          true);

    Size_Define(Vec3(1),
                AABBOverrideCompCategory.Name,
                AABBOverrideCompCategory.Priority,
                true,
                true);
  }

  AABBOverrideComponent::~AABBOverrideComponent() {}

  ComponentPtr AABBOverrideComponent::Copy(Entity* ntt)
  {
    AABBOverrideComponentPtr dst = std::make_shared<AABBOverrideComponent>();
    dst->m_entity                = ntt;
    dst->m_localData             = m_localData;

    return dst;
  }

  void AABBOverrideComponent::Init(bool flushClientSideArray) {}

  BoundingBox AABBOverrideComponent::GetAABB()
  {
    BoundingBox aabb = {};
    aabb.min         = GetPositionOffsetVal();
    aabb.max         = GetPositionOffsetVal() + GetSizeVal();
    return aabb;
  }

  void AABBOverrideComponent::SetAABB(BoundingBox aabb)
  {
    SetPositionOffsetVal(aabb.min);
    SetSizeVal(aabb.max - aabb.min);
  }

} //  namespace ToolKit
