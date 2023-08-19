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

#include "Component.h"

namespace ToolKit
{

  typedef std::shared_ptr<class MeshComponent> MeshComponentPtr;
  typedef std::vector<MeshComponentPtr> MeshComponentPtrArray;

  static VariantCategory MeshComponentCategory {"Mesh Component", 90};

  class TK_API MeshComponent : public Component
  {
   public:
    TKDeclareClass(MeshComponent, Component);

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
    ComponentPtr Copy(EntityPtr ntt) override;

    /**
     * Gets the bounding box of the contained Mesh.
     * @return BoundingBox of the contained Mesh.
     */
    BoundingBox GetAABB();

    /**
     * Initiates the MeshComponent and underlying Mesh and Material resources.
     */
    void Init(bool flushClientSideArray);

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    void ParameterConstructor() override;

   public:
    TKDeclareParam(MeshPtr, Mesh); //!< Component's Mesh resource.
    TKDeclareParam(bool, CastShadow);

   private:
    BoundingBox m_aabb = {};
  };

} // namespace ToolKit