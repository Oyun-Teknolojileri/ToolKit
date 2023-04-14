#include "Pass.h"

#include "DataTexture.h"
#include "DirectionComponent.h"
#include "Pass.h"
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
    erase_if(entities,
             [](Entity* ntt) -> bool
             { return !ntt->IsDrawable() || !ntt->IsVisible(); });

    for (Entity* ntt : entities)
    {
      MeshRawPtrArray allMeshes;
      MaterialPtrArray allMaterials;

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
        for (size_t i = 0; i < mlist.size(); i++)
        {
          allMaterials.push_back(mlist[i]);
        }
      }

      // Fill remaining if any with default or mesh materials.
      size_t startIndx = allMaterials.empty() ? 0 : allMaterials.size();
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
      RenderJobArray newJobs;
      newJobs.resize(allMeshes.size());
      Mat4 transform = ntt->m_node->GetTransform();
      for (size_t i = 0; i < allMeshes.size(); i++)
      {
        RenderJob& rj     = newJobs[i];
        rj.Entity         = ntt;
        rj.WorldTransform = transform;

        if (AABBOverrideComponentPtr bbOverride =
                ntt->GetComponent<AABBOverrideComponent>())
        {
          rj.BoundingBox = std::move(bbOverride->GetAABB());
        }
        else
        {
          rj.BoundingBox = allMeshes[i]->m_aabb;
        }

        TransformAABB(rj.BoundingBox, transform);

        rj.ShadowCaster = mc->GetCastShadowVal();
        rj.Mesh         = allMeshes[i];
        rj.Material     = allMaterials[i];
        rj.SkeletonCmp  = rj.Mesh->IsSkinned()
                              ? ntt->GetComponent<SkeletonComponent>()
                              : nullptr;
      }

      jobArray.insert(jobArray.end(), newJobs.begin(), newJobs.end());
    }
  }

  void RenderJobProcessor::CreateRenderJob(Entity* entity,
                                           RenderJobArray& jobArray)
  {
    EntityRawPtrArray tmpEntityArray;
    tmpEntityArray.push_back(entity);
    CreateRenderJobs(tmpEntityArray, jobArray);
  }

  void RenderJobProcessor::SeperateDeferredForward(
      const RenderJobArray& jobArray,
      RenderJobArray& deferred,
      RenderJobArray& forward,
      RenderJobArray& translucent)
  {
    for (const RenderJob& job : jobArray)
    {
      if (job.Material->IsTranslucent())
      {
        translucent.push_back(job);
      }
      else if (job.Material->IsDeferred())
      {
        deferred.push_back(job);
      }
      else
      {
        forward.push_back(job);
      }

      // Sanitize shaders.
      job.Material->SetDefaultMaterialTypeShaders();
    }
  }

  void RenderJobProcessor::SeperateOpaqueTranslucent(
      const RenderJobArray& jobArray,
      RenderJobArray& opaque,
      RenderJobArray& translucent)
  {
    for (const RenderJob& job : jobArray)
    {
      if (job.Material->IsTranslucent())
      {
        translucent.push_back(job);
      }
      else
      {
        opaque.push_back(job);
      }
    }
  }

  // An interval has start time and end time
  struct LightSortStruct
  {
    Light* light        = nullptr;
    uint intersectCount = 0;
  };

  // Compares two intervals according to starting times.
  bool CompareLightIntersects(const LightSortStruct& i1,
                              const LightSortStruct& i2)
  {
    return (i1.intersectCount > i2.intersectCount);
  }

  LightRawPtrArray RenderJobProcessor::SortLights(
      const RenderJob& job,
      const LightRawPtrArray& lights)
  {
    LightRawPtrArray bestLights;
    bestLights.reserve(lights.size());

    // Find the end of directional lights
    for (int i = 0; i < lights.size(); i++)
    {
      if (lights[i]->GetType() == EntityType::Entity_DirectionalLight)
      {
        bestLights.push_back(lights[i]);
      }
    }

    // Add the lights inside of the radius first
    std::vector<LightSortStruct> intersectCounts(lights.size());
    BoundingBox aabb = job.Mesh->m_aabb;
    TransformAABB(aabb, job.WorldTransform);

    for (uint lightIndx = 0; lightIndx < lights.size(); lightIndx++)
    {
      float radius;
      Light* light = lights[lightIndx];
      if (light->GetType() == EntityType::Entity_PointLight)
      {
        radius = static_cast<PointLight*>(light)->GetRadiusVal();
      }
      else if (light->GetType() == EntityType::Entity_SpotLight)
      {
        radius = static_cast<SpotLight*>(light)->GetRadiusVal();
      }
      else
      {
        continue;
      }

      intersectCounts[lightIndx].light = light;
      uint& curIntersectCount = intersectCounts[lightIndx].intersectCount;

      /* This algorithms can be used for better sorting
      for (uint dimIndx = 0; dimIndx < 3; dimIndx++)
      {
        for (uint isMin = 0; isMin < 2; isMin++)
        {
          Vec3 p     = aabb.min;
          p[dimIndx] = (isMin == 0) ? aabb.min[dimIndx] : aabb.max[dimIndx];
          float dist = glm::length(
              p - light->m_node->GetTranslation(TransformationSpace::TS_WORLD));
          if (dist <= radius)
          {
            curIntersectCount++;
          }
        }
      }*/

      if (light->GetType() == EntityType::Entity_SpotLight)
      {
        light->UpdateShadowCamera();

        Frustum spotFrustum =
            ExtractFrustum(light->m_shadowMapCameraProjectionViewMatrix, false);

        if (FrustumBoxIntersection(spotFrustum, aabb) !=
            IntersectResult::Outside)
        {
          curIntersectCount++;
        }
      }
      if (light->GetType() == EntityType::Entity_PointLight)
      {
        BoundingSphere lightSphere = {light->m_node->GetTranslation(), radius};
        if (SphereBoxIntersection(lightSphere, aabb))
        {
          curIntersectCount++;
        }
      }
    }

    std::sort(intersectCounts.begin(),
              intersectCounts.end(),
              CompareLightIntersects);

    for (uint i = 0; i < intersectCounts.size(); i++)
    {
      if (intersectCounts[i].intersectCount == 0)
      {
        break;
      }
      bestLights.push_back(intersectCounts[i].light);
    }

    return bestLights;
  }

  LightRawPtrArray RenderJobProcessor::SortLights(
      Entity* entity,
      const LightRawPtrArray& lights)
  {
    RenderJobArray jobs;
    CreateRenderJob(entity, jobs);

    LightRawPtrArray allLights;
    for (RenderJob& rj : jobs)
    {
      LightRawPtrArray la = SortLights(rj, lights);
      allLights.insert(allLights.end(), la.begin(), la.end());
    }

    return allLights;
  }

  void RenderJobProcessor::StableSortByDistanceToCamera(
      RenderJobArray& jobArray,
      const Camera* cam)
  {
    std::function<bool(const RenderJob&, const RenderJob&)> sortFn =
        [cam](const RenderJob& j1, const RenderJob& j2) -> bool
    {
      Vec3 camLoc = cam->m_node->GetTranslation(TransformationSpace::TS_WORLD);

      // TODO: Cihan cache calculated world boxes.
      BoundingBox bb1 = j1.Mesh->m_aabb;
      TransformAABB(bb1, j1.WorldTransform);

      BoundingBox bb2 = j2.Mesh->m_aabb;
      TransformAABB(bb2, j2.WorldTransform);

      float first  = glm::length2(bb1.GetCenter() - camLoc);
      float second = glm::length2(bb2.GetCenter() - camLoc);

      return second < first;
    };

    if (cam->IsOrtographic())
    {
      sortFn = [cam](const RenderJob& j1, const RenderJob& j2) -> bool
      {
        float first  = glm::column(j1.WorldTransform, 3).z;
        float second = glm::column(j2.WorldTransform, 3).z;
        return first < second;
      };
    }

    std::stable_sort(jobArray.begin(), jobArray.end(), sortFn);
  }

  void RenderJobProcessor::CullRenderJobs(RenderJobArray& jobArray,
                                          Camera* camera)
  {
    FrustumCull(jobArray, camera);
  }

  void RenderJobProcessor::AssignEnvironment(
      RenderJobArray& jobArray,
      const EnvironmentComponentPtrArray& environments)
  {
    if (environments.empty())
    {
      return;
    }

    for (RenderJob& job : jobArray)
    {
      BoundingBox bestBox;
      for (const EnvironmentComponentPtr& volume : environments)
      {
        if (volume->GetIlluminateVal() == false)
        {
          continue;
        }

        // Pick the smallest volume intersecting with job.
        BoundingBox vbb = std::move(volume->GetBBox());
        if (BoxBoxIntersection(vbb, job.BoundingBox))
        {
          if (bestBox.Volume() > vbb.Volume() ||
              job.EnvironmentVolume == nullptr)
          {
            bestBox               = vbb;
            job.EnvironmentVolume = volume;
          }
        }
      }
    }
  }

  void RenderJobProcessor::CalculateStdev(const RenderJobArray& rjVec,
                                          float& stdev,
                                          Vec3& mean)
  {
    int n = (int) rjVec.size();

    // Calculate mean position
    Vec3 sum(0.0f);
    for (int i = 0; i < n; i++)
    {
      Vec3 pos = rjVec[i].WorldTransform[3].xyz;
      sum      += pos;
    }
    mean      = sum / (float) n;

    // Calculate standard deviation of position
    float ssd = 0.0f;
    for (int i = 0; i < n; i++)
    {
      Vec3 pos  = rjVec[i].WorldTransform[3].xyz;
      Vec3 diff = pos - mean;
      ssd       += glm::dot(diff, diff);
    }
    stdev = std::sqrt(ssd / (float) n);
  }

  bool RenderJobProcessor::IsOutlier(const RenderJob& rj,
                                     float sigma,
                                     const float stdev,
                                     const Vec3& mean)
  {
    Vec3 pos   = rj.WorldTransform[3].xyz;
    Vec3 diff  = pos - mean;
    float dist = glm::length(diff) / stdev;

    return (dist > sigma);
  }

} // namespace ToolKit
