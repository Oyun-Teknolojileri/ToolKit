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

  typedef std::shared_ptr<class MaterialComponent> MaterialComponentPtr;
  typedef std::vector<MaterialComponentPtr> MaterialComponentPtrArray;

  static VariantCategory MaterialComponentCategory {"Material Component", 90};

  class TK_API MaterialComponent : public Component
  {
   public:
    TKDeclareClass(MaterialComponent, Component);

    MaterialComponent();
    virtual ~MaterialComponent();

    /**
     * Creates a copy of the MaterialComponent.
     * @param ntt Parent Entity of the component.
     * @return Copy of the MaterialComponent.
     */
    ComponentPtr Copy(Entity* ntt) override;

    void Init(bool flushClientSideArray);
    void AddMaterial(MaterialPtr mat);
    void RemoveMaterial(uint index);

    /**
     * Access to material list.
     * @returns Immutable material list.
     */
    const MaterialPtrArray& GetMaterialList() const;

    /**
     * Access to material list.
     * @returns Mutable material list.
     */
    MaterialPtrArray& GetMaterialList();

    /**
     * Re fetches all the materials from the parent entity.
     */
    void UpdateMaterialList();

    /**
     * This function is aimed to provide easy access to the first material in
     * the list, the assumption is that, the owner entity is a simple mesh with
     * a single material.
     * @returns The first material in the list. If the list is empty, the
     * material on the first mesh, if its null to the default material.
     */
    MaterialPtr GetFirstMaterial();

    /**
     * Sets the first element of the material list as the given material.
     * @params material is the material to set as the first.
     */
    void SetFirstMaterial(const MaterialPtr& material);

   protected:
    void ParameterConstructor() override;
    void DeSerializeImp(XmlDocument* doc, XmlNode* parent) override;
    void DeSerializeImpV045(XmlDocument* doc, XmlNode* parent);
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   private:
    /**
     * Array of materials in the entity's meshes. The index of the material
     * corresponds to index of the mesh / submesh.
     */
    MaterialPtrArray m_materialList;

    // Deprecated. Keeping it for Backward compatibility with 0.4.1
    TKDeclareParam(MaterialPtr, Material);
  };

} // namespace ToolKit