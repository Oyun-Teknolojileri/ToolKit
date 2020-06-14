#pragma once

#include "Entity.h"

namespace ToolKit
{

  class Drawable : public Entity
  {
  public:
    Drawable();
    virtual ~Drawable();
		virtual bool IsDrawable() const override;
    virtual EntityType GetType() const override;
    virtual void SetPose(Animation* anim) override;
		virtual struct BoundingBox GetAABB(bool inWorld = false) const override;
    virtual Drawable* GetCopy() const override;
    virtual void GetCopy(Entity* copyTo) const override;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  public:
    MeshPtr m_mesh;
  };

}