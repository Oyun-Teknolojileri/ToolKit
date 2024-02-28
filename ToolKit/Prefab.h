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

    /**
     * Initiates prefab scene and link to the current scene.
     */
    void Init(Scene* currentScene);

    /**
     * Destroys all prefab scene entities and unlink.
     */
    void UnInit();

    /**
     * Remove the prefab entity and everything inside the prefab scene from
     * the current scene.
     */
    void Unlink();

    /**
     * Add all elements in the prefab scene to the current scene.
     */
    void Link();

    static PrefabPtr GetPrefabRoot(const EntityPtr ntt);
    Entity* CopyTo(Entity* other) const override;
    BoundingBox GetBoundingBox(bool inWorld = false) const override;

    /**
     * These functions will look for the entity in LINKED scene.
     * Returns null pointer if the entity is not found or the prefab is not linked.
     */
    EntityPtr GetFirstByName(const String& name);
    EntityPtr GetFirstByTag(const String& tag);

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent);

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
