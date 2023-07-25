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

#include "TKObject.h"

namespace ToolKit
{

  /**
   * Enums for component types.
   * DEPRECATED here for backward compatibility.
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

  typedef std::shared_ptr<class Component> ComponentPtr;
  typedef std::vector<ComponentPtr> ComponentPtrArray;

  /**
   * Base component class that represent data which can be added and queried
   * by entities. Components are responsible bringing in related functionality
   * to the attached Entity classes.
   */
  class TK_API Component : public TKObject
  {
   public:
    TKDeclareClass(Component, TKObject);

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
    virtual ComponentPtr Copy(Entity* ntt) = 0;

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

   public:
    Entity* m_entity = nullptr; //!< Parent Entity of the component.
  };

  /**
   * DEPRECATED use TKObjectFactory
   * Utility class to construct Components.
   */
  class TK_API ComponentFactory final
  {
   public:
    Component* Create(ComponentType cls); //!< Deprecated. Just serving here for backward compatibility.
  };

} // namespace ToolKit
