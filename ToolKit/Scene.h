#pragma once

#include <vector>
#include <functional>

#include "Resource.h"
#include "MathUtil.h"
#include "Light.h"
#include "Types.h"
#include "Sky.h"

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
    TKResourceType(Scene)

    Scene();
    explicit Scene(String file);
    virtual ~Scene();

    void Load() override;
    void Save(bool onlyIfDirty) override;
    void Init(bool flushClientSideArray = true) override;
    void UnInit() override;
    // Merges entities from the other scene and wipeouts the other scene.
    virtual void Merge(ScenePtr other);

    // Scene queries.
    virtual PickData PickObject
    (
      Ray ray,
      const EntityIdArray& ignoreList = EntityIdArray(),
      const EntityRawPtrArray& extraList = EntityRawPtrArray()
    );
    virtual void PickObject
    (
      const Frustum& frustum,
      std::vector<PickData>& pickedObjects,
      const EntityIdArray& ignoreList = EntityIdArray(),
      const EntityRawPtrArray& extraList = EntityRawPtrArray(),
      bool pickPartiallyInside = true
    );

    // Entity operations.
    Entity* GetEntity(ULongID id) const;
    virtual void AddEntity(Entity* entity);
    const EntityRawPtrArray& GetEntities() const;
    const LightRawPtrArray GetLights() const;
    Entity* GetFirstEntityByName(const String& name);
    EntityRawPtrArray GetByTag(const String& tag);
    Entity* GetFirstByTag(const String& tag);
    EntityRawPtrArray Filter(std::function<bool(Entity*)> filter);
    Sky* GetSky();

    virtual Entity* RemoveEntity(ULongID id);
    virtual void RemoveEntity(const EntityRawPtrArray& entities);
    virtual void RemoveAllEntities();
    virtual void Destroy(bool removeResources);
    virtual void SavePrefab(Entity* entity);
    virtual void ClearEntities();

    // Serialization.
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    // Used to avoid Id collision during scene merges.
    ULongID GetBiggestEntityId();

   protected:
    void CopyTo(Resource* other) override;

   protected:
    EntityRawPtrArray m_entities;
    String m_version;
  };

  class TK_API SceneManager : public ResourceManager
  {
   public:
    SceneManager();
    virtual ~SceneManager();
    void Init() override;
    void Uninit() override;
    bool CanStore(ResourceType t) override;
    ResourcePtr CreateLocal(ResourceType type) override;
    String GetDefaultResource(ResourceType type) override;

    ScenePtr GetCurrentScene();
    void SetCurrentScene(const ScenePtr& scene);

   private:
    ScenePtr m_currentScene;
  };

}  // namespace ToolKit
