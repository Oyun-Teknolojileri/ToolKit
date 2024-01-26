/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Pass.h"

#include "Camera.h"
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
    Renderer* renderer = GetRenderer();
    m_prevFrameBuffer  = renderer->GetFrameBuffer();
  }

  void Pass::PostRender()
  {
    Renderer* renderer = GetRenderer();
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

  BoundingBox RenderJobProcessor::CreateRenderJobs(const EntityPtrArray& entities,
                                                   RenderJobArray& jobArray,
                                                   bool ignoreVisibility)
  {
    CPU_FUNC_RANGE();

    auto checkDrawableFn = [ignoreVisibility](EntityPtr ntt) -> bool
    {
      bool isDrawable = ntt->IsDrawable();
      bool isVisbile  = ntt->IsVisible();
      return isDrawable && (isVisbile || ignoreVisibility);
    };

    jobArray.reserve(entities.size()); // at least.
    BoundingBox boundingVolume;

    for (const EntityPtr& ntt : entities)
    {
      if (!checkDrawableFn(ntt))
      {
        continue;
      }

      bool materialMissing       = false;
      MaterialComponent* matComp = ntt->GetComponentFast<MaterialComponent>();
      uint matIndex              = 0;
      MeshComponent* mc          = ntt->GetComponentFast<MeshComponent>();
      bool castShadow            = mc->GetCastShadowVal();
      const MeshPtr& parentMesh  = mc->GetMeshVal();
      Mat4 nttTransform          = ntt->m_node->GetTransform();
      bool overrideBBoxExists    = false;

      BoundingBox overrideBBox;
      if (AABBOverrideComponent* bbOverride = ntt->GetComponentFast<AABBOverrideComponent>())
      {
        overrideBBoxExists = true;
        overrideBBox       = std::move(bbOverride->GetAABB());
      }

      SkeletonComponent* skComp                = ntt->GetComponentFast<SkeletonComponent>();
      const AnimRecordRawPtrArray& animRecords = GetAnimationPlayer()->m_records;
      bool foundAnim                           = false;
      for (const AnimRecordRawPtr& animRecord : animRecords)
      {
        if (const EntityPtr& animNtt = animRecord->m_entity.lock())
        {
          if (animNtt->GetIdVal() == ntt->GetIdVal())
          {
            skComp->m_animData.currentAnimation = animRecord->m_animation;
            skComp->m_animData.blendAnimation   = animRecord->m_blendAnimation;
            foundAnim                           = true;
            break;
          }
        }
      }

      if (!foundAnim && skComp != nullptr)
      {
        skComp->m_animData.currentAnimation = nullptr;
        skComp->m_animData.blendAnimation   = nullptr;
      }

      auto addRenderJobForMeshFn = [&](Mesh* mesh)
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

          if (job.BoundingBox.IsValid())
          {
            boundingVolume.UpdateBoundary(job.BoundingBox);
          }

          job.ShadowCaster = castShadow;
          job.Mesh         = mesh;

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

          if (skComp != nullptr)
          {
            const AnimData& animData = skComp->GetAnimData();
            job.animData             = animData; // copy
          }

          jobArray.push_back(job);
        }
      };

      MeshRawPtrArray allMeshes;
      parentMesh->GetAllMeshes(allMeshes);
      for (Mesh* mesh : allMeshes)
      {
        if (mesh->GetVertexCount() != 0)
        {
          addRenderJobForMeshFn(mesh);
        }
      }

      if (materialMissing)
      {
        TK_WRN("Entity \"%s\" have less material than mesh count! ToolKit uses default material for now.",
               ntt->GetNameVal().c_str());
      }
    }

    return boundingVolume;
  }

  void RenderJobProcessor::CullLights(LightPtrArray& lights, const CameraPtr& camera)
  {
    Mat4 pr               = camera->GetProjectionMatrix();
    Mat4 v                = camera->GetViewMatrix();
    Mat4 prv              = pr * v;

    Frustum frustum       = ExtractFrustum(prv, false);

    Frustum normalFrustum = frustum;
    NormalizeFrustum(normalFrustum);

    lights.erase(std::remove_if(lights.begin(),
                                lights.end(),
                                [&](const LightPtr& light) -> bool
                                {
                                  switch (light->GetLightType())
                                  {
                                  case Light::Directional:
                                    return false;
                                  case Light::Spot:
                                  {
                                    SpotLight* spot = static_cast<SpotLight*>(light.get());
                                    return FrustumBoxIntersection(frustum, spot->m_boundingBoxCache) ==
                                           IntersectResult::Outside;
                                  }
                                  break;
                                  case Light::Point:
                                  {
                                    PointLight* point = static_cast<PointLight*>(light.get());
                                    return !FrustumSphereIntersection(normalFrustum, point->m_boundingSphereCache);
                                  }
                                  break;
                                  default:
                                    assert(false && "Unknown light type.");
                                    return true;
                                  }
                                }),
                 lights.end());
  }

  void RenderJobProcessor::SeperateRenderData(RenderData& renderData)
  {
    // Group deferred to forward.
    auto forwardItr                          = std::partition(renderData.jobs.begin(),
                                     renderData.jobs.end(),
                                     [](const RenderJob& job) { return job.Material->IsDeferred(); });

    // Group opaque to translucent.
    auto translucentItr                      = std::partition(forwardItr,
                                         renderData.jobs.end(),
                                         [](const RenderJob& job) { return job.Material->IsTranslucent(); });

    renderData.forwardOpaqueStartIndex       = (int) std::distance(renderData.jobs.begin(), forwardItr);
    renderData.forwardTranslucentStartIndex  = (int) std::distance(translucentItr, renderData.jobs.end());
    renderData.forwardTranslucentStartIndex += renderData.forwardOpaqueStartIndex;
  }

  void RenderJobProcessor::AssignLight(RenderJobItr begin, RenderJobItr end, LightPtrArray& lights)
  {
    int directionalEndIndx = PreSortLights(lights);

    auto assignmentFn      = [](RenderJobItr job, Light* light, int i) -> void
    {
      job->lights[job->activeLightCount] = i;
      job->activeLightCount++;
    };

    auto checkBreakFn = [](int activeLightCount) -> bool
    { return activeLightCount >= Renderer::RHIConstants::MaxLightsPerObject; };

    for (RenderJobItr job = begin; job != end; job++)
    {
      job->activeLightCount = 0;
      for (int i = 0; i < directionalEndIndx; i++)
      {
        assignmentFn(job, lights[i].get(), i);

        if (checkBreakFn(job->activeLightCount))
        {
          break;
        }
      }

      const BoundingBox& jobBox = job->BoundingBox;
      for (int i = directionalEndIndx; i < (int) lights.size(); i++)
      {
        if (checkBreakFn(job->activeLightCount))
        {
          break;
        }

        LightPtr& light = lights[i];
        if (light->GetLightType() == Light::Spot)
        {
          SpotLight* spot = static_cast<SpotLight*>(light.get());
          if (FrustumBoxIntersection(spot->m_frustumCache, jobBox) != IntersectResult::Outside)
          {
            assignmentFn(job, lights[i].get(), i);
          }
        }
        else
        {
          assert(light->GetLightType() == Light::Point && "Unknown light type.");
          PointLight* point = static_cast<PointLight*>(light.get());
          if (SphereBoxIntersection(point->m_boundingSphereCache, jobBox))
          {
            assignmentFn(job, lights[i].get(), i);
          }
        }
      }
    }
  }

  struct LightSortStruct
  {
    LightPtr light      = nullptr;
    uint intersectCount = 0;
  };

  bool CompareLightIntersects(const LightSortStruct& i1, const LightSortStruct& i2)
  {
    return (i1.intersectCount > i2.intersectCount);
  }

  int RenderJobProcessor::PreSortLights(LightPtrArray& lights)
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

    return directionalLightEndIndex;
  }

  void RenderJobProcessor::SortByDistanceToCamera(RenderJobItr begin, RenderJobItr end, const CameraPtr& cam)
  {
    CPU_FUNC_RANGE();

    Vec3 camLoc = cam->m_node->GetTranslation(TransformationSpace::TS_WORLD);

    std::function<bool(const RenderJob&, const RenderJob&)> sortFn = [&camLoc](const RenderJob& j1,
                                                                               const RenderJob& j2) -> bool
    {
      const BoundingBox& bb1 = j1.BoundingBox;
      const BoundingBox& bb2 = j2.BoundingBox;

      float first            = glm::length2(bb1.GetCenter() - camLoc);
      float second           = glm::length2(bb2.GetCenter() - camLoc);

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

    std::sort(begin, end, sortFn);
  }

  void RenderJobProcessor::CullRenderJobs(RenderJobArray& jobArray, const CameraPtr& camera)
  {
    FrustumCull(jobArray, camera);
  }

  void RenderJobProcessor::CullRenderJobs(const RenderJobArray& jobArray, const CameraPtr& camera, BoolArray& results)
  {
    FrustumCull(jobArray, camera, results);
  }

  void RenderJobProcessor::StableSortByMeshThanMaterail(RenderData& renderData)
  {
    auto sortRangeFn = [](RenderJobItr begin, RenderJobItr end) -> void
    {
      std::stable_sort(begin,
                       end,
                       [](const RenderJob& a, const RenderJob& b) -> bool
                       { return a.Mesh->GetIdVal() < b.Mesh->GetIdVal(); });

      std::stable_sort(begin,
                       end,
                       [](const RenderJob& a, const RenderJob& b) -> bool
                       { return a.Material->GetIdVal() < b.Material->GetIdVal(); });
    };

    // Deferred partition.
    RenderJobItr begin = renderData.jobs.begin();
    RenderJobItr end   = renderData.GetForwardOpaqueBegin();
    sortRangeFn(begin, end);

    // Forward Opaque
    begin = renderData.GetForwardOpaqueBegin();
    end   = renderData.GetForwardTranslucentBegin();
    sortRangeFn(begin, end);

    // Forward Translucent
    begin = renderData.GetForwardTranslucentBegin();
    end   = renderData.jobs.end();
    sortRangeFn(begin, end);
  }

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
