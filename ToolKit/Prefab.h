/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Entity.h"
#include "Types.h"

namespace ToolKit
{

  static VariantCategory PrefabCategory {"Prefab", 90};

  /**
   * Entity to use in scenes
   * Loads the given scene and instantiates root entities to current scene
   */
  class TK_API Prefab : public Entity
  {
   public:
    TKDeclareClass(Prefab, Entity);

    Prefab();
    virtual ~Prefab();

    bool IsDrawable() const override;

    /** Initiates prefab scene and link to the current scene. */
    void Init(Scene* currentScene);

    /** Destroys all prefab scene entities and unlink. */
    void UnInit();

    /** Remove the prefab entity and everything inside the prefab scene from the current scene. */
    void Unlink();

    /** Add all elements in the prefab scene to the current scene. */
    void Link();

    /** If the entity is child of a prefab, returns the prefab entity. */
    static PrefabPtr GetPrefabRoot(const EntityPtr ntt);

    /** Copy the prefab. New prefab is initialized but not linked by default. */
    Entity* CopyTo(Entity* other) const override;

    /**
     * This function will look for the first entity with given name in LINKED scene.
     * @return First entity with given name. Null pointer if the entity is not found or the prefab is not linked.
     */
    EntityPtr GetFirstByName(const String& name);

    /**
     * This function will look for the first entity with given tag in LINKED scene.
     * @return First entity with given tag. Null pointer if the entity is not found or the prefab is not linked.
     */
    EntityPtr GetFirstByTag(const String& tag);

    /** Returns entity list instantiated for this prefab. */
    const EntityPtrArray& GetInstancedEntities();

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent);
    void UpdateLocalBoundingBox() override;

   private:
    void ParameterConstructor() override;

   public:
    TKDeclareParam(String, PrefabPath);

   private:
    ScenePtr m_prefabScene;
    Scene* m_currentScene;
    bool m_initiated = false;
    bool m_linked    = false;

    // Used only in deserialization
    std::unordered_map<String, ParameterVariantArray> m_childCustomDatas;
    EntityPtrArray m_instanceEntities;
  };

} // namespace ToolKit
