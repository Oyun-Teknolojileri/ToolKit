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

  typedef std::shared_ptr<class MultiMaterialComponent> MultiMaterialPtr;
  typedef std::vector<MultiMaterialPtr> MultiMaterialPtrArray;

  static VariantCategory MultiMaterialCompCategory {"Multi-Material Component",
                                                    90};

  class TK_API MultiMaterialComponent : public Component
  {
   public:
    TKComponentType(MultiMaterialComponent);

    MultiMaterialComponent();
    virtual ~MultiMaterialComponent();

    /**
     * Creates a copy of the MultiMaterialComponent.
     * @param ntt Parent Entity of the component.
     * @return Copy of the MultiMaterialComponent.
     */
    ComponentPtr Copy(Entity* ntt) override;

    void Init(bool flushClientSideArray);
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void AddMaterial(MaterialPtr mat);
    void RemoveMaterial(uint index);
    const MaterialPtrArray& GetMaterialList() const;
    MaterialPtrArray& GetMaterialList();
    void UpdateMaterialList();

   private:
    MaterialPtrArray materials;
  };

} // namespace ToolKit