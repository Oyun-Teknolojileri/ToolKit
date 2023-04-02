#pragma once

/**
 * @file Scene.h Header file for the Scene class.
 */

#include "Light.h"
#include "MathUtil.h"
#include "Resource.h"
#include "Sky.h"
#include "Types.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace ToolKit
{

  /**
   * The Scene class represents a collection of entities in a 3D environment. It
   * provides functionality for loading and saving scenes, updating and querying
   * entities, and serializing/deserializing to/from XML.
   */
  class TK_API Scene : public Resource
  {
   public:
    /**
     * A helper struct that holds the result of a ray-picking operation in the
     * scene.
     */
    struct PickData
    {
      /**
       * The position in world space where the ray intersects with the object.
       */
      Vec3 pickPos;

      /**
       * A pointer to the Entity object that was picked.
       */
      Entity* entity = nullptr;
    };

   public:
    TKResourceType(Scene)

    /**
     * The constructor for the Scene class.
     */
    Scene();

    /**
     * The constructor for the Scene class that loads a scene from a file.
     *
     * @param file The file path of the scene to load.
     */
    explicit Scene(String file);

    /**
     * The destructor for the Scene class.
     */
    virtual ~Scene();

    /**
     * Loads the scene from its file.
     */
    void Load() override;

    /**
     * Saves the scene to its file.
     *
     * @param onlyIfDirty If true, the scene will only be saved if it has been
     * modified since the last save.
     */
    void Save(bool onlyIfDirty) override;

    /**
     * Initializes the scene.
     *
     * @param flushClientSideArray It will pass down to resource components of
     * the entity for initialization.
     */
    void Init(bool flushClientSideArray = false) override;

    /**
     * Uninitializes the scene.
     */
    void UnInit() override;

    /**
     * Updates the scene by the specified amount of time.
     *
     * @param deltaTime The time elapsed since the last update.
     */
    virtual void Update(float deltaTime);

    /**
     * Merges the entities from another scene into this scene and clears the
     * other scene.
     *
     * @param other A pointer to the other scene to merge.
     */
    virtual void Merge(ScenePtr other);

    // Scene queries.

    /**
     * Performs a ray-picking operation on the scene to find the first object
     * that the ray intersects.
     *
     * @param ray The ray to use for picking.
     * @param ignoreList A list of entity IDs to ignore during the picking
     * operation.
     * @param extraList A list of extra entity pointers to include in the
     * picking operation.
     *
     * @return A PickData struct containing the result of the picking operation.
     */
    virtual PickData PickObject(
        Ray ray,
        const EntityIdArray& ignoreList    = EntityIdArray(),
        const EntityRawPtrArray& extraList = EntityRawPtrArray());

    /**
     * Performs a frustum culling operation on the scene to find all objects
     * that are partially or fully contained within the frustum.
     *
     * @param frustum The frustum to use for culling.
     * @param pickedObjects An output vector to store the results of the culling
     * operation.
     * @param ignoreList A list of entity IDs to ignore during the culling
     * operation.
     * @param extraList A list of extra entity pointers to include in the
     * culling operation.
     * @param pickPartiallyInside If true, objects that are partially contained
     * within the frustum will also be included.
     */
    virtual void PickObject(const Frustum& frustum,
                            std::vector<PickData>& pickedObjects,
                            const EntityIdArray& ignoreList    = {},
                            const EntityRawPtrArray& extraList = {},
                            bool pickPartiallyInside           = true);

    // Entity operations.

    /**
     * Gets the entity with the given ID from the scene.
     * @param id The ID of the entity to get.
     * @returns The entity with the given ID, or nullptr if no entity with that
     * ID exists in the scene.
     */
    Entity* GetEntity(ULongID id) const;

    /**
     * Adds an entity to the scene.
     * @param entity The entity to add.
     */
    virtual void AddEntity(Entity* entity);
    EntityRawPtrArray& AccessEntityArray(); //!< Mutable Entity array access.

    /**
     * Gets all the entities in the scene.
     * @returns An array containing pointers to all the entities in the scene.
     */
    const EntityRawPtrArray& GetEntities() const;

    /**
     * Gets an array of all the lights in the scene.
     * @returns An array containing pointers to all the lights in the scene.
     */
    LightRawPtrArray GetLights() const;

    /**
     * Gets the first entity in the scene with the given name.
     * @param name The name of the entity to get.
     * @returns The first entity in the scene with the given name, or nullptr if
     * no entity with that name exists in the scene.
     */
    Entity* GetFirstEntityByName(const String& name);

    /**
     * Gets an array of all the entities in the scene with the given tag.
     * @param tag The tag to search for.
     * @returns An array containing pointers to all the entities in the scene
     * with the given tag.
     */
    EntityRawPtrArray GetByTag(const String& tag);

    /**
     * Gets the first entity in the scene with the given tag.
     * @param tag The tag to search for.
     * @returns The first entity in the scene with the given tag, or nullptr if
     * no entity with that tag exists in the scene.
     */
    Entity* GetFirstByTag(const String& tag);

    /**
     * Filters the entities in the scene using the given filter function.
     * @param filter A function that takes an Entity pointer as its argument and
     * returns a boolean value indicating whether to include the entity in the
     * filtered result.
     * @returns An array containing pointers to all the entities in the scene
     * that passed the filter function.
     */
    EntityRawPtrArray Filter(std::function<bool(Entity*)> filter);

    /**
     * Gets the sky object associated with the scene.
     * @returns A pointer to the SkyBase object associated with the scene, or
     * nullptr if no sky object is associated with the scene.
     */
    SkyBase* GetSky();

    /**
     * Links a prefab to the scene.
     * @param fullPath The full path to the prefab file.
     */
    void LinkPrefab(const String& fullPath);

    /**
     * Returns an array of pointers to all environment volume components in the
     * scene.
     * @returns The array of pointers to environment volume components.
     */
    EnvironmentComponentPtrArray GetEnvironmentVolumes();

    /**
     * Removes the entity with the given id from the scene.
     * @param  The id of the entity that will be removed.
     * @param  States if the remove will be recursive to the all leafs.
     * @returns The removed entity.
     */
    virtual Entity* RemoveEntity(ULongID id, bool deep = true);

    /**
     * Removes an array of entities from the scene.
     * @param entities An array of pointers to the entities to be removed.
     */
    virtual void RemoveEntity(const EntityRawPtrArray& entities);

    /**
     * Removes all entities from the scene.
     */
    virtual void RemoveAllEntities();

    /**
     * Destroys the scene and removes all of its resources.
     * @param removeResources Whether to remove all associated resources or not.
     */
    virtual void Destroy(bool removeResources);

    /**
     * Saves a prefab for an entity in the scene.
     * @param entity The entity to create the prefab from.
     */
    virtual void SavePrefab(Entity* entity);

    /**
     * Removes all entities from the scene.
     */
    virtual void ClearEntities();

    // Serialization.

    /**
     * Serializes the scene to an XML document.
     * @param doc The XML document to serialize to.
     * @param parent The parent XML node to serialize under.
     */
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;

    /**
     * Deserializes the scene from an XML document.
     * @param doc The XML document to deserialize from.
     * @param parent The parent XML node to deserialize from.
     */
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    /**
     * Returns the biggest number generated during the current runtime. This
     * function is used to avoid ID collision during scene merges.
     *
     * @return The biggest entity ID generated during the current runtime.
     */
    ULongID GetBiggestEntityId();

   private:
    /**
     * Removes all children of the given entity.
     * @param removed The entity whose children will be removed.
     */
    void RemoveChildren(Entity* removed);

   protected:
    /**
     * Copies the scene to another resource.
     * @param other The resource to copy to.
     */
    void CopyTo(Resource* other) override;

    /**
     * Normalize the ID of an entity while serializing the scene.
     * @param doc The XML document being serialized.
     * @param parent The parent node of the entity being serialized.
     * @param indx The index of the entity in the entity list.
     */
    void NormalizeEntityID(XmlDocument* doc, XmlNode* prent, size_t indx) const;

   protected:
    EntityRawPtrArray m_entities; //!< The entities in the scene.
    String m_version;             //!< The version of the scene file.
    bool m_isPrefab;              //!< Whether or not the scene is a prefab.
  };

  /**
   * A class for managing game scenes as resources.
   */
  class TK_API SceneManager : public ResourceManager
  {
   public:
    /**
     * Constructor.
     */
    SceneManager();

    /**
     * Destructor.
     */
    virtual ~SceneManager();

    /**
     * Initializes the scene manager.
     */
    void Init() override;

    /**
     * Uninitializes the scene manager.
     */
    void Uninit() override;

    /**
     * Checks if a given resource type can be stored by the scene manager.
     * @param t The resource type to check.
     * @return True if the resource type can be stored, false otherwise.
     */
    bool CanStore(ResourceType t) override;

    /**
     * Creates a new local resource of the given type.
     * @param type The type of resource to create.
     * @return A pointer to the newly created resource.
     */
    ResourcePtr CreateLocal(ResourceType type) override;

    /**
     * Gets the default resource file path for the given resource type.
     * @param type The resource type.
     * @return The default resource file path.
     */
    String GetDefaultResource(ResourceType type) override;

    /**
     * Gets the currently active scene.
     * @return A pointer to the current scene.
     */
    ScenePtr GetCurrentScene();

    /**
     * Sets the currently active scene.
     * @param scene A pointer to the new current scene.
     */
    void SetCurrentScene(const ScenePtr& scene);

   private:
    ScenePtr m_currentScene; //!< A pointer to the currently active scene.
  };

} // namespace ToolKit
