#pragma once

#include "Entity.h"

namespace ToolKit
{

  class TK_API Drawable : public Entity
  {
  public:
    Drawable();
    virtual ~Drawable();
    virtual bool IsDrawable() const override;
    virtual EntityType GetType() const override;
    virtual void SetPose(Animation* anim) override;
    virtual struct BoundingBox GetAABB(bool inWorld = false) const override;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    virtual void RemoveResources() override;
    
    MeshPtr& GetMesh() const;
    void SetMesh(const MeshPtr& mesh);

  protected:
    using Entity::ParameterConstructor;

    virtual Entity* CopyTo(Entity* copyTo) const override;
    virtual Entity* InstantiateTo(Entity* copyTo) const override;
  };

}