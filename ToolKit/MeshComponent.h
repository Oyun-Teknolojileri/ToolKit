/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Component.h"
#include "GeometryTypes.h"

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

    using Component::NativeConstruct;

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
    BoundingBox GetBoundingBox();

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
    BoundingBox m_boundingBox = {};
  };

} // namespace ToolKit