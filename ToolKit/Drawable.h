/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Entity.h"

namespace ToolKit
{

  // Deprecated.
  // Still exist for backward compatibility. If you need a drawable object
  // consider adding a MeshComponent to an entity.
  class TK_API Drawable final : public Entity
  {
   public:
    TKDeclareClass(Drawable, Entity);

    Drawable();
    virtual ~Drawable();

    void SetPose(const AnimationPtr& anim, float time, BlendTarget* blendTarget = nullptr) override;
    void RemoveResources() override;

    MeshPtr GetMesh() const;
    void SetMesh(const MeshPtr& mesh);

   protected:
    using Entity::ParameterConstructor;

    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    Entity* CopyTo(Entity* copyTo) const override;
  };

} // namespace ToolKit
