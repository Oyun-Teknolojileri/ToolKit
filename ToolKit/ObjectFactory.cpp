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

#include "ObjectFactory.h"

#include "AnimationControllerComponent.h"
#include "Audio.h"
#include "Camera.h"
#include "Canvas.h"
#include "DataTexture.h"
#include "DirectionComponent.h"
#include "Drawable.h"
#include "Entity.h"
#include "EnvironmentComponent.h"
#include "GradientSky.h"
#include "Light.h"
#include "Material.h"
#include "MaterialComponent.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "Prefab.h"
#include "Primative.h"
#include "ResourceComponent.h"
#include "Scene.h"
#include "Shader.h"
#include "SkeletonComponent.h"
#include "Sky.h"
#include "SpriteSheet.h"
#include "SsaoPass.h"
#include "Surface.h"
#include "Texture.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKObjectFactory::TKObjectFactory() {}

  TKObjectFactory::~TKObjectFactory() {}

  TKObjectFactory::ObjectConstructorCallback& TKObjectFactory::GetConstructorFn(const StringView Class)
  {
    auto constructorFnIt = m_constructorFnMap.find(Class);
    if (constructorFnIt != m_constructorFnMap.end())
    {
      return constructorFnIt->second;
    }

    return m_nullFn;
  }

  /**
   * Constructs a new Object from class name.
   * @param cls - Class name of the object to be created.
   * @return A new instance of the object with the given class name.
   */
  Object* TKObjectFactory::MakeNew(const StringView Class)
  {
    if (auto constructorFn = GetConstructorFn(Class))
    {
      Object* object = constructorFn();
      object->NativeConstruct();
      return object;
    }

    assert(false && "Unknown object type.");
    return nullptr;
  }

  void TKObjectFactory::Init()
  {
    // Entities.
    Register<AABBOverrideComponent>();
    Register<AnimControllerComponent>();
    Register<DirectionComponent>();
    Register<EnvironmentComponent>();
    Register<MaterialComponent>();
    Register<MeshComponent>();
    Register<SkeletonComponent>();
    Register<Arrow2d>();
    Register<AudioSource>();
    Register<Billboard>();
    Register<Camera>();
    Register<Cone>();
    Register<Cube>();
    Register<Drawable>();
    Register<Entity>();
    Register<EntityNode>();
    Register<Light>();
    Register<DirectionalLight>();
    Register<PointLight>();
    Register<SpotLight>();
    Register<LineBatch>();
    Register<Prefab>();
    Register<Quad>();
    Register<SkyBase>();
    Register<GradientSky>();
    Register<Sky>();
    Register<Sphere>();
    Register<Quad>();
    Register<SpriteAnimation>();
    Register<Surface>();
    Register<Canvas>();
    Register<Button>();
    // Resources.
    Register<Animation>();
    Register<Audio>();
    Register<Material>();
    Register<Mesh>();
    Register<SkinMesh>();
    Register<Scene>();
    Register<Shader>();
    Register<Skeleton>();
    Register<SpriteSheet>();
    Register<Texture>();
    Register<CubeMap>();
    Register<DataTexture>();
    Register<LightDataTexture>();
    Register<SSAONoiseTexture>();
    Register<DepthTexture>();
    Register<Hdri>();
    Register<RenderTarget>();
  }

} // namespace ToolKit
