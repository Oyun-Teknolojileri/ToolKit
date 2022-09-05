#pragma once

#include <vector>
#include <functional>
#include <unordered_map>

#include "Resource.h"
#include "MathUtil.h"
#include "Light.h"
#include "Types.h"
#include "Sky.h"

namespace ToolKit
{
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

    void Init(Scene* currentScene);
    static Prefab* GetPrefabRoot(Entity* ntt);

    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;

   private:
    void ParameterConstructor();
    void ParameterEventConstructor();

   public:
    TKDeclareParam(String, ScenePath);
    ScenePtr prefabScene;

   private:
    // Used only in deserialization
    std::unordered_map<String, ParameterVariantArray> childCustomDatas;
  };
} // namespace ToolKit
