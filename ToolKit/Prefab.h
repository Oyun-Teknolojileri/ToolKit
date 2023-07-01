/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
    TKDeclareClass(Prefab, Entity);

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
     * Add all elements in the prefab scene to the current scene.
     */
    void Link();

    static Prefab* GetPrefabRoot(Entity* ntt);
    Entity* CopyTo(Entity* other) const override;

   protected:
    void DeSerializeImp(XmlDocument* doc, XmlNode* parent) override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   private:
    void ParameterConstructor() override;

   public:
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
