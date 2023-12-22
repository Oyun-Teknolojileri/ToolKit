/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Pass.h"

#include "Camera.h"
#include "DataTexture.h"
#include "DirectionComponent.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Pass.h"
#include "Renderer.h"
#include "ResourceComponent.h"
#include "TKProfiler.h"
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
    renderer->SetFramebuffer(m_prevFrameBuffer, GraphicBitFields::None);
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

  void RenderJobProcessor::CreateRenderJobs(const EntityPtrArray& entities,
                                            RenderJobArray& jobArray,
                                            bool ignoreVisibility)
  {
    CPU_FUNC_RANGE();

    auto checkDrawableFn = [ignoreVisibility](EntityPtr ntt) -> bool
    { return ntt->IsDrawable() && (ntt->IsVisible() || ignoreVisibility); };

    for (EntityPtr ntt : entities)
    {
      if (!checkDrawableFn(ntt))
      {
        continue;
      }

      bool materialMissing         = false;
      MaterialComponentPtr matComp = ntt->GetMaterialComponent();
      uint matIndex                = 0;
      MeshComponentPtr mc          = ntt->GetMeshComponent();
      bool castShadow              = mc->GetCastShadowVal();
      MeshPtr parentMesh           = mc->GetMeshVal();
      Mat4 nttTransform            = ntt->m_node->GetTransform();
      bool overrideBBoxExists      = false;
      BoundingBox overrideBBox;
      if (AABBOverrideComponentPtr bbOverride = ntt->GetComponent<AABBOverrideComponent>())
      {
        overrideBBoxExists = true;
        overrideBBox       = std::move(bbOverride->GetAABB());
      }

      auto addRenderJobForMeshFn = [&materialMissing,
                                    &matComp,
                                    &matIndex,
                                    &mc,
                                    &ntt,
                                    &jobArray,
                                    &nttTransform,
                                    overrideBBoxExists,
                                    &overrideBBox,
                                    castShadow](Mesh* mesh)
      {
        if (mesh)
        {
          RenderJob job;
          job.Entity         = ntt;
          job.WorldTransform = nttTransform;
          if (overrideBBoxExists)
          {
            job.BoundingBox = overrideBBox;
          }
          else
          {
            job.BoundingBox = mesh->m_aabb;
          }
          TransformAABB(job.BoundingBox, job.WorldTransform);

          job.ShadowCaster = castShadow;
          job.Mesh         = mesh;
          job.SkeletonCmp  = job.Mesh->IsSkinned() ? ntt->GetComponent<SkeletonComponent>() : nullptr;

          // Look material component first, if we can not find a corresponding material in there, look inside mesh. If
          // still there is no corresponding material, give a warning to the user and use default material
          if (matComp)
          {
            const MaterialPtrArray& mats = matComp->GetMaterialList();
            if (matIndex < mats.size())
            {
              job.Material = mats[matIndex++];
            }
          }
          if (job.Material == nullptr)
          {
            if (mesh->m_material)
            {
              job.Material = mesh->m_material;
            }
          }
          if (job.Material == nullptr)
          {
            // Warn user that we use the default material for the mesh
            materialMissing = true;
            job.Material    = GetMaterialManager()->GetCopyOfDefaultMaterial(false);
          }

          jobArray.push_back(job);
        }
      };

      static MeshRawPtrArray allMeshes;
      parentMesh->GetAllMeshes(allMeshes);
      for (Mesh* mesh : allMeshes)
      {
        addRenderJobForMeshFn(mesh);
      }
      allMeshes.clear();

      if (materialMissing)
      {
        TK_WRN("Entity \"%s\" have less material than mesh count! ToolKit uses default material for now.",
               ntt->GetNameVal().c_str());
      }
    }
  }

  void RenderJobProcessor::SeperateDeferredForward(const RenderJobArray& jobArray,
                                                   RenderJobArray& deferred,
                                                   RenderJobArray& forward,
                                                   RenderJobArray& translucent)
  {
    CPU_FUNC_RANGE();

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

  void RenderJobProcessor::SeperateOpaqueTranslucent(const RenderJobArray& jobArray,
                                                     RenderJobArray& opaque,
                                                     RenderJobArray& translucent)
  {
    CPU_FUNC_RANGE();

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
    LightPtr light      = nullptr;
    uint intersectCount = 0;
  };

  // Compares two intervals according to starting times.
  bool CompareLightIntersects(const LightSortStruct& i1, const LightSortStruct& i2)
  {
    return (i1.intersectCount > i2.intersectCount);
  }

  void RenderJobProcessor::SortLights(const RenderJob& job, LightPtrArray& lights)
  {
    // Get all directional lights to beginning first
    uint directionalLightEndIndex = 0;
    for (size_t i = 0; i < lights.size(); ++i)
    {
      if (lights[i]->IsA<DirectionalLight>())
      {
        if (i == directionalLightEndIndex)
        {
          directionalLightEndIndex++;
          continue;
        }

        std::iter_swap(lights.begin() + i, lights.begin() + directionalLightEndIndex);
        directionalLightEndIndex++;
      }
    }

    static std::vector<LightSortStruct> intersectCounts;
    intersectCounts.clear();
    intersectCounts.resize(lights.size() - directionalLightEndIndex);
    BoundingBox aabb = job.Mesh->m_aabb;
    TransformAABB(aabb, job.WorldTransform);

    for (uint lightIndx = directionalLightEndIndex; lightIndx < lights.size(); lightIndx++)
    {
      float radius;
      LightPtr light = lights[lightIndx];
      if (PointLight* pLight = light->As<PointLight>())
      {
        radius = pLight->GetRadiusVal();
      }
      else if (SpotLight* sLight = light->As<SpotLight>())
      {
        radius = sLight->GetRadiusVal();
      }
      else
      {
        continue;
      }

      intersectCounts[lightIndx - directionalLightEndIndex].light = light;
      uint& curIntersectCount = intersectCounts[lightIndx - directionalLightEndIndex].intersectCount;

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

      if (light->IsA<SpotLight>())
      {
        // The shadow camera of light should be updated before calling this function
        // RenderPath PreRender functions should do that

        Frustum spotFrustum = ExtractFrustum(light->m_shadowMapCameraProjectionViewMatrix, false);

        if (FrustumBoxIntersection(spotFrustum, aabb) != IntersectResult::Outside)
        {
          curIntersectCount++;
        }
      }
      if (light->IsA<PointLight>())
      {
        BoundingSphere lightSphere = {light->m_node->GetTranslation(), radius};
        if (SphereBoxIntersection(lightSphere, aabb))
        {
          curIntersectCount++;
        }
      }
    }

    // Sort point & spot lights
    std::sort(intersectCounts.begin(), intersectCounts.end(), CompareLightIntersects);

    for (int i = 0; i < intersectCounts.size(); ++i)
    {
      lights[i + directionalLightEndIndex] = intersectCounts[i].light;
    }
  }

  LightPtrArray RenderJobProcessor::SortLights(EntityPtr entity, LightPtrArray& lights)
  {
    CPU_FUNC_RANGE();

    RenderJobArray jobs;
    EntityPtrArray oneEntity = {entity};
    CreateRenderJobs(oneEntity, jobs);

    LightPtrArray allLights;
    for (RenderJob& rj : jobs)
    {
      SortLights(rj, lights);
      allLights.insert(allLights.end(), lights.begin(), lights.end());
    }

    return allLights;
  }

  void RenderJobProcessor::SortByDistanceToCamera(RenderJobArray& jobArray, const CameraPtr cam)
  {
    CPU_FUNC_RANGE();

    std::function<bool(const RenderJob&, const RenderJob&)> sortFn = [cam](const RenderJob& j1,
                                                                           const RenderJob& j2) -> bool
    {
      Vec3 camLoc     = cam->m_node->GetTranslation(TransformationSpace::TS_WORLD);

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

    std::sort(jobArray.begin(), jobArray.end(), sortFn);
  }

  void RenderJobProcessor::CullRenderJobs(RenderJobArray& jobArray, CameraPtr camera) { FrustumCull(jobArray, camera); }

  void RenderJobProcessor::AssignEnvironment(RenderJobArray& jobArray, const EnvironmentComponentPtrArray& environments)
  {
    CPU_FUNC_RANGE();

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
          if (bestBox.Volume() > vbb.Volume() || job.EnvironmentVolume == nullptr)
          {
            bestBox               = vbb;
            job.EnvironmentVolume = volume;
          }
        }
      }
    }
  }

  void RenderJobProcessor::CalculateStdev(const RenderJobArray& rjVec, float& stdev, Vec3& mean)
  {
    int n = (int) rjVec.size();

    // Calculate mean position
    Vec3 sum(0.0f);
    for (int i = 0; i < n; i++)
    {
      Vec3 pos  = rjVec[i].WorldTransform[3];
      sum      += pos;
    }
    mean      = sum / (float) n;

    // Calculate standard deviation of position
    float ssd = 0.0f;
    for (int i = 0; i < n; i++)
    {
      Vec3 pos   = rjVec[i].WorldTransform[3];
      Vec3 diff  = pos - mean;
      ssd       += glm::dot(diff, diff);
    }
    stdev = std::sqrt(ssd / (float) n);
  }

  bool RenderJobProcessor::IsOutlier(const RenderJob& rj, float sigma, const float stdev, const Vec3& mean)
  {
    Vec3 pos   = rj.WorldTransform[3];
    Vec3 diff  = pos - mean;
    float dist = glm::length(diff) / stdev;

    return (dist > sigma);
  }

} // namespace ToolKit
