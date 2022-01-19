#pragma once

#include "Resource.h"
#include "MathUtil.h"
#include <functional>

namespace ToolKit
{

  class TK_API Scene : public Resource
  {
  public:
    struct PickData
    {
      Vec3 pickPos;
      Entity* entity = nullptr;
    };

  public:
    TKResouceType(Scene)

    Scene();
    Scene(String file);
    virtual ~Scene();

    virtual void Load() override;
    virtual void Save(bool onlyIfDirty) override;
    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;
    virtual void Merge(ScenePtr other); // Merges entities from the other scene and wipeouts the other scene.

    // Scene queries.
    PickData PickObject(Ray ray, const EntityIdArray& ignoreList = EntityIdArray()) const;
    void PickObject(const Frustum& frustum, std::vector<PickData>& pickedObjects, const EntityIdArray& ignoreList = EntityIdArray(), bool pickPartiallyInside = true) const;

    // Entity operations.
    Entity* GetEntity(EntityId id) const;
    virtual void AddEntity(Entity* entity);
    const EntityRawPtrArray& GetEntities() const;
    Entity* GetFirstEntityByName(const String& name);
    EntityRawPtrArray GetByTag(const String& tag);
    EntityRawPtrArray Filter(std::function<bool(Entity*)> filter);

    virtual Entity* RemoveEntity(EntityId id);
    virtual void RemoveEntity(const EntityRawPtrArray& entities);
    virtual void RemoveAllEntities();
    virtual void Destroy(bool removeResources);
    virtual void SavePrefab(Entity* entity);

    // Serialization.
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  protected:
    virtual void CopyTo(Resource* other) override;

  protected:
    EntityRawPtrArray m_entities;
  };

  class TK_API SceneManager : public ResourceManager
  {
  public:
    SceneManager();
    virtual ~SceneManager();
    virtual void Init() override;
    virtual void Uninit() override;
    virtual bool CanStore(ResourceType t);
    virtual ResourcePtr CreateLocal(ResourceType type);

  public:
    ScenePtr m_currentScene;
  };

}