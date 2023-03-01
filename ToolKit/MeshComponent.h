#pragma once

#include "Component.h"
#include "MathUtil.h"

namespace ToolKit
{
  typedef std::shared_ptr<class MeshComponent> MeshComponentPtr;
  typedef std::vector<MeshComponentPtr> MeshComponentPtrArray;
  static VariantCategory MeshComponentCategory {"Mesh Component", 90};

  class TK_API MeshComponent : public Component
  {
   public:
    /**
     * Auto generated code for type information.
     */
    TKComponentType(MeshComponent);

    /**
     * Empty constructor.
     */
    MeshComponent();

    /**
     * Empty destructor.
     */
    virtual ~MeshComponent();

    /**
     * Creates a copy of the MeshComponent. Contained Mesh does not get
     * copied but referenced. However Material is copied and will be serialized
     * to the scene if the containing Entity gets serialized.
     * @param ntt Parent Entity of the component.
     * @return Copy of the MeshComponent.
     */
    ComponentPtr Copy(Entity* ntt) override;

    /**
     * Gets the bounding box of the contained Mesh.
     * @return BoundingBox of the contained Mesh.
     */
    BoundingBox GetAABB();

    /**
     * Initiates the MeshComponent and underlying Mesh and Material resources.
     */
    void Init(bool flushClientSideArray);

   public:
    TKDeclareParam(MeshPtr, Mesh); //!< Component's Mesh resource.
    TKDeclareParam(bool, CastShadow);

   private:
    BoundingBox m_aabb = {};
  };
} // namespace ToolKit