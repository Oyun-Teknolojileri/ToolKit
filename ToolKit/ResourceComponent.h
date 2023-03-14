#pragma once

#include "Component.h"
#include "MathUtil.h"
#include "Types.h"

#include <memory>
#include <vector>

namespace ToolKit
{

  typedef std::shared_ptr<class AABBOverrideComponent> AABBOverrideComponentPtr;
  typedef std::vector<AABBOverrideComponentPtr> AABBOverrideComponentPtrArray;

  static VariantCategory AABBOverrideCompCategory {"AABB Override Component",
                                                   90};

  class TK_API AABBOverrideComponent : public Component
  {
   public:
    TKComponentType(AABBOverrideComponent);

    AABBOverrideComponent();
    virtual ~AABBOverrideComponent();

    /**
     * Creates a copy of the AABB Override Component.
     * @param ntt Parent Entity of the component.
     * @return Copy of the AABBOverrideComponent.
     */
    ComponentPtr Copy(Entity* ntt) override;

    void Init(bool flushClientSideArray);
    BoundingBox GetAABB();
    // AABB should be in entity space (not world space)
    void SetAABB(BoundingBox aabb);

   private:
    TKDeclareParam(Vec3, PositionOffset);
    TKDeclareParam(Vec3, Size);
  };
} //  namespace ToolKit
