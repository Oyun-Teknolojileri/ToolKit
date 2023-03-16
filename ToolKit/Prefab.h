#pragma once

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

  static VariantCategory PrefabCategory {"Prefab", 90};

  /**
   * Entity to use in scenes
   * Loads the given scene and instantiates root entities to current scene
   */
  class TK_API Prefab : public Entity
  {
   public:
    Prefab();
    virtual ~Prefab();

    EntityType GetType() const override;

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
    * Add all elements in the prabscene to the current scene.
    */
    void Link();

    static Prefab* GetPrefabRoot(Entity* ntt);
    Entity* CopyTo(Entity* other) const override;

    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;

   private:
    void ParameterConstructor();
    void ParameterEventConstructor();

   public:
    // Should be in Prefab folder
    TKDeclareParam(String, PrefabPath);
    ScenePtr m_prefabScene;
    Scene* m_currentScene;
    bool m_initiated = false;

   private:
    bool m_linked = false;

    // Used only in deserialization
    std::unordered_map<String, ParameterVariantArray> m_childCustomDatas;
    EntityRawPtrArray m_instanceEntities;
  };
} // namespace ToolKit
