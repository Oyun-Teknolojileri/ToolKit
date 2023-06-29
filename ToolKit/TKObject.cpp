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
#include "Camera.h"
#include "DirectionComponent.h"
#include "Entity.h"
#include "EnvironmentComponent.h"
#include "GradientSky.h"
#include "Light.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "Prefab.h"
#include "Primative.h"
#include "ResourceComponent.h"
#include "SkeletonComponent.h"
#include "Sky.h"
#include "SpriteSheet.h"
#include "Surface.h"

namespace ToolKit
{

  bool TKClass::IsSublcassOf(TKClass* base)
  {
    if (Super == TKObject::StaticClass())
    {
      // Bottom of the inheritance chain, meaning that no match found.
      return false;
    }

    if (base == Super)
    {
      return true;
    }

    if (this == base)
    {
      return true;
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

  TKObjectPtr TKObject::Copy() { return nullptr; }

  XmlNode* TKObject::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    assert(doc != nullptr && parent != nullptr);

    XmlNode* objNode = CreateXmlNode(doc, StaticClass()->Name, parent);
    WriteAttr(objNode, doc, XmlObjectClassAttr, Class()->Name);

    m_localData.Serialize(doc, objNode);
    return objNode;
  }

  void TKObject::DeSerializeImp(XmlDocument* doc, XmlNode* parent) {}

  void TKObjectFactory::Init()
  {
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
  }

} // namespace ToolKit
