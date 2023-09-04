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

#include "TKObject.h"

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

  bool TKClass::IsSublcassOf(TKClass* base)
  {
    if (base == Super)
    {
      return true;
    }

    if (this == base)
    {
      return true;
    }

    // This specific condition is only valid for TKObject, marking this point as the end.
    if (this == Super)
    {
      return false; // No match found.
    }

    return Super->IsSublcassOf(base);
  }

  TKDefineClass(TKObject, TKObject);

  TKObject::TKObject() {}

  TKObject::~TKObject() {}

  void TKObject::NativeConstruct()
  {
    ParameterConstructor();
    ParameterEventConstructor();
  }

  void TKObject::NativeDestruct() {}

  void TKObject::ParameterConstructor()
  {
    ULongID id = GetHandleManager()->GetNextHandle();
    Id_Define(id, EntityCategory.Name, EntityCategory.Priority, true, false);
  }

  void TKObject::ParameterEventConstructor() {}

  TKObjectPtr TKObject::Copy() const { return nullptr; }

  XmlNode* TKObject::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    assert(doc != nullptr && parent != nullptr);

    XmlNode* objNode = CreateXmlNode(doc, StaticClass()->Name, parent);
    WriteAttr(objNode, doc, XmlObjectClassAttr, Class()->Name);

    m_localData.Serialize(doc, objNode);
    return objNode;
  }

  XmlNode* TKObject::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    assert(parent != nullptr && "Root of the object can't be null.");
    m_localData.DeSerialize(info, parent);

    // Construction progress from bottom up.
    return parent;
  }

  void TKObject::PostDeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    ParameterEventConstructor(); // Set all the events after data deserialized.
  }

  TKObjectFactory::TKObjectFactory() { Init(); }

  TKObjectFactory::~TKObjectFactory() {}

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
