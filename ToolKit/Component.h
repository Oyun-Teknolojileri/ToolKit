#pragma once

/**
 * @file Component.h Header file for base Component classes
 * and related structures.
 */

#include <vector>
#include <memory>

#include "ParameterBlock.h"
#include "Serialize.h"
#include "Types.h"

namespace ToolKit
{

  /**
   * @def TKComponentType(type) The macro is responsible for
   * responsible for auto generating component type information.
   */
  #define TKComponentType(type) \
  static ComponentType GetTypeStatic() { return ComponentType::type; } \
  ComponentType GetType() const override { return ComponentType::type; }

  typedef std::shared_ptr<class Component> ComponentPtr;
  typedef std::vector<ComponentPtr> ComponentPtrArray;

  /**
   * Enums for component types.
   */
  enum class ComponentType
  {
    Base,
    MeshComponent,
    DirectionComponent,
    MaterialComponent,
    EnvironmentComponent,
    AnimControllerComponent
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
     * the component. It should not be an instance.
     * Base class take care of cloning ParameterBlock of the Component.
     * @param ntt Parent Entity of the component.
     * @return Copy of the component.
     */
    virtual ComponentPtr Copy(Entity* ntt) = 0;

    /**
     * Serializes the Component's ParameterBlock to the xml document.
     * If parent is not null appends this component to the parent node.
     */
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;

    /**
     * De serialize the Component's ParameterBlock from given xml node
     * and document.
     */
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    static Component* CreateByType(ComponentType t);

   public:
    ULongID m_id;  //!< Unique id of the component for the current runtime.
    ParameterBlock m_localData;  //!< Component local data.
    Entity* m_entity = nullptr;  //!< Parent Entity of the component.
  };

}  // namespace ToolKit
