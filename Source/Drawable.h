#pragma once

#include "Entity.h"

namespace ToolKit
{

  class TK_API Drawable : public Entity
  {
  public:
    using Entity::GetCopy;
    using Entity::GetInstance;

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

  protected:
    virtual Entity* GetCopy(Entity* copyTo) const override;
    virtual Entity* GetInstance(Entity* copyTo) const override;

  public:
    MeshPtr m_mesh;
  };

}