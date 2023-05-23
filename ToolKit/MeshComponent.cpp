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

#include "MeshComponent.h"

#include "Entity.h"
#include "Mesh.h"
#include "SkeletonComponent.h"

#include "DebugNew.h"

namespace ToolKit
{

  MeshComponent::MeshComponent()
  {
    Mesh_Define(std::make_shared<ToolKit::Mesh>(),
                MeshComponentCategory.Name,
                MeshComponentCategory.Priority,
                true,
                true);

    CastShadow_Define(true, MeshComponentCategory.Name, MeshComponentCategory.Priority, true, true);
  }

  MeshComponent::~MeshComponent() {}

  ComponentPtr MeshComponent::Copy(Entity* ntt)
  {
    MeshComponentPtr mc = std::make_shared<MeshComponent>();
    mc->m_localData     = m_localData;
    mc->m_entity        = ntt;
    return mc;
  }

  BoundingBox MeshComponent::GetAABB()
  {
    SkeletonComponentPtr skelComp = m_entity->GetComponent<SkeletonComponent>();
    if (skelComp && GetMeshVal()->IsSkinned())
    {
      SkinMesh* skinMesh = (SkinMesh*) GetMeshVal().get();
      if (skelComp->isDirty)
      {
        m_aabb            = skinMesh->CalculateAABB(skelComp->GetSkeletonResourceVal().get(), skelComp->m_map);
        skelComp->isDirty = false;
      }
      return m_aabb;
    }
    return GetMeshVal()->m_aabb;
  }

  void MeshComponent::Init(bool flushClientSideArray) { GetMeshVal()->Init(flushClientSideArray); }

} // namespace ToolKit