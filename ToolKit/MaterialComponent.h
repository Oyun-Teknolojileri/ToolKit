/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
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
    ComponentPtr Copy(EntityPtr ntt) override;

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

    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent);

   private:
    /**
     * Array of materials in the entity's meshes. The index of the material
     * corresponds to index of the mesh / submesh.
     */
    MaterialPtrArray m_materialList;
  };

} // namespace ToolKit