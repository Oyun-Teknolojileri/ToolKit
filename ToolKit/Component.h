#pragma once

/**
* @file Component.h Header file for base Component classes
* and related structures.
*/

#include <memory>

#include "Types.h"
#include "Serialize.h"

namespace ToolKit
{

  /**
  * @def TKComponentType(type) The macro is responsible for
  * responsible for auto generating component type information.
  */
#define TKComponentType(type) \
  static ComponentType GetTypeStatic() { return ComponentType::type; } \
  virtual ComponentType GetType() const { return ComponentType::type; }

  /**
  * Enums for component types.
  */
  enum class ComponentType
  {
    Base,
    MeshComponent
  };

  /**
  * Base component class that represent data which can be added and queried
  * by entities. Components are responsible bringing in related functionality
  * to the attached Entity classes.
  */
  class TK_API Component : public Serializable
  {
   public:
    /**
    * Default constructor. It initializes a unique id that is not obtained
    * during the current runtime.
    */
    Component();

    /**
    * Empty destructor.
    */
    virtual ~Component();

    /**
    * Base function for type informations. Derived classes expected to use
    * TKComponentType macro to override this function.
    * @return ComponentType of the class.
    */
    virtual ComponentType GetType() const;

    /**
    * Creates a copy of the component. Derived classes should perform 
    * appropriate cloning functionality that creates actual memory copy of
    * the component. It should not be instance.
    * @return Copy of the component.
    */
    virtual ComponentPtr Copy() = 0;

   public:
    ULongID m_id;  //!< Unique id of the component for the current runtime.

   private:
    static ULongID m_handle;  //!< Base id. Each new component increments 1.
  };

  typedef std::shared_ptr<class MeshComponent> MeshComponentPtr;

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
    * Serializes the MeshComponent to the xml document. If parent is not null
    * appends this component to the parent node.
    */
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;

    /**
    * De serialize the MeshComponent from given xml node and document.
    */
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    /**
    * Creates a copy of the MeshComponent. Contained Mesh does not get
    * copied but referenced. However Material is copied and will be serialized
    * to the scene if the containing Entity gets serialized.
    * @return Copy of the MeshComponent.
    */
    virtual ComponentPtr Copy();

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
    MeshPtr m_mesh;  //!< Component's Mesh resource.

    /**
    * Component's material resource. In case this object is not null, Renderer
    * picks this material to render the mesh otherwise falls back to Material
    * within the Mesh.
    */
    MaterialPtr m_material;
  };

}  // namespace ToolKit
