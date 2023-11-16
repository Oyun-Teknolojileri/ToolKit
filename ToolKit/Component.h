/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

/**
 * @file Component.h Header file for base Component classes
 * and related structures.
 */

#include "Object.h"

namespace ToolKit
{

  typedef std::shared_ptr<class Component> ComponentPtr;
  typedef std::vector<ComponentPtr> ComponentPtrArray;

  /**
   * Base component class that represent data which can be added and queried
   * by entities. Components are responsible bringing in related functionality
   * to the attached Entity classes.
   */
  class TK_API Component : public Object
  {
   public:
    TKDeclareClass(Component, Object);

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
     * Creates a copy of the component. Derived classes should perform
     * appropriate cloning functionality that creates actual memory copy of
     * the component. It should not be an instance.
     * Base class take care of cloning ParameterBlock of the Component.
     * @param ntt Parent Entity of the component.
     * @return Copy of the component.
     */
    virtual ComponentPtr Copy(EntityPtr ntt) = 0;

    /**
     * Getter function for owner entity.
     * @return Owner EntityPtr.
     */
    EntityPtr OwnerEntity() const { return m_entity.lock(); }

    /**
     * Setter function for owner entity.
     * @param owner owning EntityPtr.
     */
    void OwnerEntity(EntityPtr owner) { m_entity = owner; }

   protected:
    void ParameterConstructor() override;

    /**
     * Serializes the Component's ParameterBlock to the xml document.
     * If parent is not null appends this component to the parent node.
     */
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

    /**
     * De serialize the Component's ParameterBlock from given xml node
     * and document.
     */
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

   protected:
    EntityWeakPtr m_entity; //!< Parent Entity of the component.
  };

  /**
   * DEPRECATED use ObjectFactory
   * Utility class to construct Components.
   */
  class ComponentFactory final
  {
   public:
    enum class ComponentType
    {
      // Order is important. Don't change for backward comparable scene files.
      Base,
      MeshComponent,
      DirectionComponent,
      MaterialComponent,
      EnvironmentComponent,
      AnimControllerComponent,
      SkeletonComponent,
      MultiMaterialComponent, // Deprecated.
      AABBOverrideComponent
    };

   public:
    static ComponentPtr Create(ComponentType Class); //!< Deprecated. Just serving here for backward compatibility.
  };

} // namespace ToolKit
