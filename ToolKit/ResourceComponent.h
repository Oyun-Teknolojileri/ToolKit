#pragma once

#include <vector>
#include <memory>

#include "Component.h"
#include "Types.h"
#include "MathUtil.h"

namespace ToolKit
{
  typedef std::shared_ptr<class MeshComponent> MeshComponentPtr;
  typedef std::vector<MeshComponentPtr> MeshComponentPtrArray;
  static VariantCategory MeshComponentCategory
  {
    "Mesh Component",
    90
  };

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
    TKDeclareParam(MeshPtr, Mesh);  //!< Component's Mesh resource.
  };

  typedef std::shared_ptr<class MaterialComponent> MaterialComponentPtr;
  typedef std::vector<MaterialComponentPtr> MaterialComponentPtrArray;

  static VariantCategory MaterialComponentCategory
  {
    "Material Component",
    90
  };

  class TK_API MaterialComponent : public Component
  {
   public:
    TKComponentType(MaterialComponent);

    MaterialComponent();
    virtual ~MaterialComponent();

    /**
    * Creates a copy of the MaterialComponent. Contained Material is copied 
    * and will be serialized to the scene if the containing Entity gets 
    * serialized.
    * @param ntt Parent Entity of the component.
    * @return Copy of the MaterialComponent.
    */
    ComponentPtr Copy(Entity* ntt) override;

    void Init(bool flushClientSideArray);

   public:
    /**
    * Component's material resource. In case this object is not null, Renderer
    * picks this material to render the mesh otherwise falls back to Material
    * within the Mesh.
    */
    TKDeclareParam(MaterialPtr, Material);
  };

  typedef std::shared_ptr<class EnvironmentComponent> EnvironmentComponentPtr;
  static VariantCategory EnvironmentComponentCategory
  {
    "Environment Component",
    90
  };

  class TK_API EnvironmentComponent : public Component
  {
   public:
    TKComponentType(EnvironmentComponent);

    EnvironmentComponent();
    virtual ~EnvironmentComponent();

    ComponentPtr Copy(Entity* ntt) override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    void UpdateBBox();

    BoundingBox* GetBBox();
    Vec3 GetBBoxMin();
    Vec3 GetBBoxMax();

    void Init(bool flushClientSideArray);

   private:
    void ParameterConstructor();
   public:
    TKDeclareParam(HdriPtr, Hdri);
    TKDeclareParam(Vec3, Max);
    TKDeclareParam(Vec3, Min);
    TKDeclareParam(bool, Illuminate);

   private:
    BoundingBox* m_bbox;
  };

}  //  namespace ToolKit
