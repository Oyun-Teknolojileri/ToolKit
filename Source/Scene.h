#pragma once

#include "Resource.h"
#include "MathUtil.h"

namespace ToolKit
{

  class Scene : public Resource
  {
  public:
    struct PickData
    {
      Vec3 pickPos;
      Entity* entity = nullptr;
    };

  public:
    Scene();
    Scene(String file);
    virtual ~Scene();

    virtual void Load() override;
    virtual void Save(bool onlyIfDirty) override;
    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;

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
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  protected:
    EntityRawPtrArray m_entitites;
  };

  class SceneManager : public ResourceManager
  {
  public:
    SceneManager();
    virtual ~SceneManager();
  };

}