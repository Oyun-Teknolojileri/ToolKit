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
    Drawable();
    virtual ~Drawable();
    bool IsDrawable() const override;
    EntityType GetType() const override;
    void SetPose(const AnimationPtr& anim, float time) override;
    struct BoundingBox GetAABB(bool inWorld = false) const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    void RemoveResources() override;

    MeshPtr GetMesh() const;
    void SetMesh(const MeshPtr& mesh);

   protected:
    using Entity::ParameterConstructor;

    Entity* CopyTo(Entity* copyTo) const override;
    Entity* InstantiateTo(Entity* copyTo) const override;
  };

}  // namespace ToolKit
