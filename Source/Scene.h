#pragma once

#include "ToolKit.h"
#include "Types.h"

namespace ToolKit
{

  class Scene
  {
  public:
    struct PickData
    {
      Vec3 pickPos;
      Entity* entity = nullptr;
    };

  public:
    Scene();
    virtual ~Scene();

    // Scene queries.
    PickData PickObject(Ray ray, const EntityIdArray& ignoreList = EntityIdArray()) const;
    void PickObject(const Frustum& frustum, std::vector<PickData>& pickedObjects, const EntityIdArray& ignoreList = EntityIdArray(), bool pickPartiallyInside = true) const;

    // Entity operations.
    Entity* GetEntity(EntityId id) const;
    void AddEntity(Entity* entity);
    const EntityRawPtrArray& GetEntities() const;
    EntityRawPtrArray GetByTag(const String& tag);

    virtual Entity* RemoveEntity(EntityId id);
    virtual void Destroy();

    // Serialization.
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);

  public:
    String m_name;

  protected:
    EntityRawPtrArray m_entitites;
  };

}