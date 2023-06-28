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

#include "Drawable.h"

#include "Component.h"
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "ResourceComponent.h"
#include "Skeleton.h"
#include "ToolKit.h"
#include "Util.h"

#include <memory>

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(Drawable, Entity);

  Drawable::Drawable() { AddComponent(new MeshComponent()); }

  Drawable::~Drawable() {}

  EntityType Drawable::GetType() const { return EntityType::Entity_Drawable; }

  void Drawable::SetPose(const AnimationPtr& anim, float time, BlendTarget* blendTarget)
  {
    Entity::SetPose(anim, time, blendTarget);
  }

  Entity* Drawable::CopyTo(Entity* copyTo) const { return Entity::CopyTo(copyTo); }

  void Drawable::DeSerializeImp(XmlDocument* doc, XmlNode* parent)
  {
    ClearComponents();
    Entity::DeSerializeImp(doc, parent);
  }

  XmlNode* Drawable::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  void Drawable::RemoveResources() { GetMeshManager()->Remove(GetMesh()->GetFile()); }

  MeshPtr Drawable::GetMesh() const
  {
    MeshComponentPtr meshComp = GetComponent<MeshComponent>();
    return meshComp->GetMeshVal();
  }

  void Drawable::SetMesh(const MeshPtr& mesh)
  {
    MeshComponentPtr meshComp = GetComponent<MeshComponent>();
    meshComp->SetMeshVal(mesh);
  }

} // namespace ToolKit
