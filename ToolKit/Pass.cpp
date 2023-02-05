#include "Pass.h"

#include "DataTexture.h"
#include "DirectionComponent.h"
#include "Renderer.h"
#include "ResourceComponent.h"
#include "ShaderReflectionCache.h"
#include "Toolkit.h"
#include "Viewport.h"

#include "DebugNew.h"

namespace ToolKit
{
  RenderPass::RenderPass() {}

  RenderPass::~RenderPass() {}

  void RenderPass::StableSortByDistanceToCamera(EntityRawPtrArray& entities,
                                                const Camera* cam)
  {
    std::function<bool(Entity*, Entity*)> sortFn = [cam](Entity* ntt1,
                                                         Entity* ntt2) -> bool
    {
      Vec3 camLoc = cam->m_node->GetTranslation(TransformationSpace::TS_WORLD);

      BoundingBox bb1 = ntt1->GetAABB(true);
      float first     = glm::length2(bb1.GetCenter() - camLoc);

      BoundingBox bb2 = ntt2->GetAABB(true);
      float second    = glm::length2(bb2.GetCenter() - camLoc);

      return second < first;
    };

    if (cam->IsOrtographic())
    {
      sortFn = [cam](Entity* ntt1, Entity* ntt2) -> bool
      {
        float first =
            ntt1->m_node->GetTranslation(TransformationSpace::TS_WORLD).z;

        float second =
            ntt2->m_node->GetTranslation(TransformationSpace::TS_WORLD).z;

        return first < second;
      };
    }

    std::stable_sort(entities.begin(), entities.end(), sortFn);
  }

  void RenderPass::SeperateTranslucentEntities(
      const EntityRawPtrArray& allEntities,
      EntityRawPtrArray& opaqueEntities,
      EntityRawPtrArray& translucentEntities)
  {
    for (Entity* ntt : allEntities)
    {
      auto checkMatTranslucency = [](MaterialPtr mat) -> bool
      {
        if (mat &&
            mat->GetRenderState()->blendFunction != BlendFunction::NONE &&
            mat->GetRenderState()->blendFunction != BlendFunction::ALPHA_MASK)
        {
          return true;
        }
        return false;
      };

      // Return true if multi material component is found
      auto checkMultiMaterialComp = [ntt,
                                     &checkMatTranslucency,
                                     &opaqueEntities,
                                     &translucentEntities]() -> bool
      {
        MultiMaterialPtr mmComp;
        mmComp = ntt->GetComponent<MultiMaterialComponent>();
        if (mmComp)
        {
          bool isThereOpaque = false, isThereTranslucent = false;
          for (MaterialPtr mat : mmComp->GetMaterialList())
          {
            if (checkMatTranslucency(mat))
            {
              isThereTranslucent = true;
            }
            else
            {
              isThereOpaque = true;
            }
          }
          if (isThereTranslucent)
          {
            translucentEntities.push_back(ntt);
          }
          if (isThereOpaque)
          {
            opaqueEntities.push_back(ntt);
          }
          return true;
        }
        return false;
      };
      if (checkMultiMaterialComp())
      {
        continue;
      }

      auto checkMaterialComps = [ntt,
                                 &checkMatTranslucency,
                                 &opaqueEntities,
                                 &translucentEntities]() -> bool
      {
        // Check too see if there are any material with blend state.
        MaterialComponentPtrArray materials;
        ntt->GetComponent<MaterialComponent>(materials);

        if (!materials.empty())
        {
          bool isThereOpaque = false, isThereTranslucent = false;
          for (MaterialComponentPtr& mt : materials)
          {
            if (checkMatTranslucency(mt->GetMaterialVal()))
            {
              isThereTranslucent = true;
            }
            else
            {
              isThereOpaque = true;
            }
          }
          if (isThereTranslucent)
          {
            translucentEntities.push_back(ntt);
          }
          if (isThereOpaque)
          {
            opaqueEntities.push_back(ntt);
          }
          return true;
        }
        return false;
      };
      if (checkMaterialComps())
      {
        continue;
      }

      auto checkSubmeshes = [ntt,
                             &checkMatTranslucency,
                             &opaqueEntities,
                             &translucentEntities]() -> bool
      {
        MeshComponentPtrArray meshes;
        ntt->GetComponent<MeshComponent>(meshes);

        if (meshes.empty())
        {
          return false;
        }

        bool isThereOpaque = false, isThereTranslucent = false;
        for (MeshComponentPtr& ms : meshes)
        {
          MeshRawCPtrArray all;
          ms->GetMeshVal()->GetAllMeshes(all);
          for (const Mesh* m : all)
          {
            if (checkMatTranslucency(m->m_material))
            {
              isThereTranslucent = true;
            }
            else
            {
              isThereOpaque = true;
            }
          }
          if (isThereTranslucent)
          {
            translucentEntities.push_back(ntt);
          }
          if (isThereOpaque)
          {
            opaqueEntities.push_back(ntt);
          }
        }
        return true;
      };
      checkSubmeshes();
    }
  }

  void RenderPass::SeperateTranslucentAndUnlitEntities(
      const EntityRawPtrArray& allEntities,
      EntityRawPtrArray& opaqueEntities,
      EntityRawPtrArray& translucentAndUnlitEntities)
  {
    for (Entity* ntt : allEntities)
    {
      auto checkMatTranslucentUnlit = [](MaterialPtr mat)
      {
        if (mat)
        {
          RenderState* rs = mat->GetRenderState();
          if ((rs->blendFunction != BlendFunction::NONE &&
               rs->blendFunction != BlendFunction::ALPHA_MASK) ||
              rs->useForwardPath)
          {
            return true;
          }
        }
        return false;
      };

      auto checkMatOpaque = [](MaterialPtr mat)
      {
        if (mat)
        {
          RenderState* rs = mat->GetRenderState();
          if ((rs->blendFunction == BlendFunction::NONE ||
               rs->blendFunction == BlendFunction::ALPHA_MASK) &&
              !rs->useForwardPath)
          {
            return true;
          }
        }
        return false;
      };

      auto checkMultiMaterialComp = [ntt,
                                     &checkMatTranslucentUnlit,
                                     &checkMatOpaque,
                                     &opaqueEntities,
                                     &translucentAndUnlitEntities]() -> bool
      {
        MultiMaterialPtr mmComp;
        mmComp = ntt->GetComponent<MultiMaterialComponent>();
        if (mmComp)
        {
          bool isThereOpaque = false, isThereTranslucentUnlit = false;
          for (MaterialPtr mat : mmComp->GetMaterialList())
          {
            if (!isThereTranslucentUnlit && checkMatTranslucentUnlit(mat))
            {
              isThereTranslucentUnlit = true;
            }
            if (!isThereOpaque && checkMatOpaque(mat))
            {
              isThereOpaque = true;
            }
          }
          if (isThereTranslucentUnlit)
          {
            translucentAndUnlitEntities.push_back(ntt);
          }
          if (isThereOpaque)
          {
            opaqueEntities.push_back(ntt);
          }
          return true;
        }
        return false;
      };
      if (checkMultiMaterialComp())
      {
        continue;
      }

      auto checkMatComps = [ntt,
                            &checkMatTranslucentUnlit,
                            &checkMatOpaque,
                            &opaqueEntities,
                            &translucentAndUnlitEntities]() -> bool
      {
        // Check too see if there are any material with blend state.
        MaterialComponentPtrArray materials;
        ntt->GetComponent<MaterialComponent>(materials);
        if (!materials.empty())
        {
          bool isThereOpaque = false, isThereTranslucentUnlit = false;
          for (MaterialComponentPtr& mtc : materials)
          {
            if (!isThereTranslucentUnlit &&
                checkMatTranslucentUnlit(mtc->GetMaterialVal()))
            {
              isThereTranslucentUnlit = true;
            }
            if (!isThereOpaque && checkMatOpaque(mtc->GetMaterialVal()))
            {
              isThereOpaque = true;
            }
          }
          if (isThereTranslucentUnlit)
          {
            translucentAndUnlitEntities.push_back(ntt);
          }
          if (isThereOpaque)
          {
            opaqueEntities.push_back(ntt);
          }
          return true;
        }
        return false;
      };
      if (checkMatComps())
      {
        continue;
      }

      auto checkSubmeshes = [ntt,
                             &checkMatTranslucentUnlit,
                             &checkMatOpaque,
                             &opaqueEntities,
                             &translucentAndUnlitEntities]() -> bool
      {
        MeshComponentPtrArray meshes;
        ntt->GetComponent<MeshComponent>(meshes);

        if (meshes.empty())
        {
          return false;
        }

        for (MeshComponentPtr& ms : meshes)
        {
          MeshRawCPtrArray all;
          ms->GetMeshVal()->GetAllMeshes(all);
          bool isThereOpaque = false, isThereTranslucentUnlit = false;
          for (const Mesh* m : all)
          {
            if (!isThereTranslucentUnlit &&
                checkMatTranslucentUnlit(m->m_material))
            {
              isThereTranslucentUnlit = true;
            }
            if (!isThereOpaque && checkMatOpaque(m->m_material))
            {
              isThereOpaque = true;
            }
          }
          if (isThereTranslucentUnlit)
          {
            translucentAndUnlitEntities.push_back(ntt);
          }
          if (isThereOpaque)
          {
            opaqueEntities.push_back(ntt);
          }
        }
        return true;
      };
      checkSubmeshes();
    }
  }

  void RenderPass::CreateRenderJobs(EntityRawPtrArray entities) {}

  Pass::Pass() {}

  Pass::~Pass() {}

  void Pass::PreRender()
  {
    Renderer* renderer     = GetRenderer();
    m_prevOverrideMaterial = renderer->m_overrideMat;
    m_prevFrameBuffer      = renderer->GetFrameBuffer();
  }

  void Pass::PostRender()
  {
    Renderer* renderer      = GetRenderer();
    renderer->m_overrideMat = m_prevOverrideMaterial;
    renderer->SetFramebuffer(m_prevFrameBuffer, false);
  }

  void Pass::RenderSubPass(const PassPtr& pass)
  {
    Renderer* renderer = GetRenderer();
    pass->SetRenderer(renderer);
    pass->PreRender();
    pass->Render();
    pass->PostRender();
  }

  Renderer* Pass::GetRenderer() { return m_renderer; }

  void Pass::SetRenderer(Renderer* renderer) { m_renderer = renderer; }

  void RenderJobProcessor::CreateRenderJobs(EntityRawPtrArray entities,
                                            RenderJobArray& jobArray)
  {
    entities.erase(std::remove_if(entities.begin(),
                                  entities.end(),
                                  [](Entity* ntt) -> bool {
                                    return !ntt->GetVisibleVal() ||
                                           !ntt->IsDrawable();
                                  }),
                   entities.end());

    MeshRawPtrArray allMeshes;
    MaterialPtrArray allMaterials;
    for (Entity* ntt : entities)
    {
      MeshComponentPtr mc = ntt->GetMeshComponent();
      mc->GetMeshVal()->GetAllMeshes(allMeshes);

      allMaterials.reserve(allMeshes.size());
      if (MaterialComponentPtr mc = ntt->GetMaterialComponent())
      {
        // Single material overrides all mesh materials.
        MaterialPtr mat = mc->GetMaterialVal();
        for (size_t i = 0; i < allMeshes.size(); i++)
        {
          allMaterials.push_back(mat);
        }
      }
      else if (MultiMaterialPtr mmc =
                   ntt->GetComponent<MultiMaterialComponent>())
      {
        // There are material assignments per sub mesh.
        MaterialPtrArray& mlist = mmc->GetMaterialList();
        for (size_t i = 0; i > mlist.size(); i++)
        {
          allMaterials.push_back(mlist[i]);
        }
      }

      // Fill remaining if any with default or mesh materials.
      size_t startIndx = allMaterials.empty() ? 0 : allMaterials.size() - 1;
      for (size_t i = startIndx; i < allMeshes.size(); i++)
      {
        if (MaterialPtr mp = allMeshes[i]->m_material)
        {
          allMaterials.push_back(mp);
        }
        else
        {
          allMaterials.push_back(
              GetMaterialManager()->GetCopyOfDefaultMaterial());
        }
      }

      // Here we have all mesh and corresponding materials.
      Mat4 transform = ntt->m_node->GetTransform();
      for (size_t i = 0; i < allMeshes.size(); i++)
      {
        jobArray.push_back({allMeshes[i], allMaterials[i], transform});
      }
    }
  }

  void RenderJobProcessor::SeperateDeferredForward(
      const RenderJobArray& jobArray,
      RenderJobArray& deferred,
      RenderJobArray& forward,
      RenderJobArray& translucent)
  {
    for (const RenderJob& job : jobArray) 
    {
      // Forward pipeline.
      if (!job.Material->IsDeferred())
      {
        if (job.Material->IsTranslucent()) 
        {
          translucent.push_back(job);
        }
        else 
        {
          forward.push_back(job);
        }
      }
      else 
      {
        // Deferred pipeline.
        deferred.push_back(job);
      }
    }
  }

} // namespace ToolKit
