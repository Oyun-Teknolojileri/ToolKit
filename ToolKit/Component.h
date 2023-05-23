/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

/**
 * @file Component.h Header file for base Component classes
 * and related structures.
 */

#include "ParameterBlock.h"
#include "Serialize.h"
#include "Types.h"

#include <memory>
#include <vector>

namespace ToolKit
{

/**
 * @def TKComponentType(type) The macro is responsible for
 * responsible for auto generating component type information.
 */
#define TKComponentType(type)                                                                                          \
  static ComponentType GetTypeStatic()                                                                                 \
  {                                                                                                                    \
    return ComponentType::type;                                                                                        \
  }                                                                                                                    \
  ComponentType GetType() const override                                                                               \
  {                                                                                                                    \
    return ComponentType::type;                                                                                        \
  }

  typedef std::shared_ptr<class Component> ComponentPtr;
  typedef std::vector<ComponentPtr> ComponentPtrArray;

  /**
   * Enums for component types.
   */
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
    ULongID m_id;               //!< Unique id of the component for the current runtime.
    ParameterBlock m_localData; //!< Component local data.
    Entity* m_entity = nullptr; //!< Parent Entity of the component.
  };

} // namespace ToolKit
