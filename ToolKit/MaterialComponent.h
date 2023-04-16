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
    TKComponentType(MaterialComponent);

    MaterialComponent();
    virtual ~MaterialComponent();

    /**
     * Creates a copy of the MaterialComponent.
     * @param ntt Parent Entity of the component.
     * @return Copy of the MaterialComponent.
     */
    ComponentPtr Copy(Entity* ntt) override;

    void Init(bool flushClientSideArray);
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
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

   private:
    /**
     * Array of materials in the entity's meshes. The index of the material
     * corresponds to index of the mesh / submesh.
     */
    MaterialPtrArray m_materialList;
  };

} // namespace ToolKit