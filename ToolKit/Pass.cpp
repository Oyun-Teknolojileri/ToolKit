/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Pass.h"

#include "AABBOverrideComponent.h"
#include "BVH.h"
#include "Camera.h"
#include "DirectionComponent.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Pass.h"
#include "Renderer.h"
#include "TKProfiler.h"
#include "Threads.h"
#include "ToolKit.h"
#include "Viewport.h"

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

  void RenderJobProcessor::CreateRenderJobs(const EntityPtrArray& entities,
                                            RenderJobArray& jobArray,
                                            bool ignoreVisibility)
  {
    LightPtrArray nullLights;
    EnvironmentComponentPtrArray nullEnvironments;
    CreateRenderJobs(entities, jobArray, nullLights, nullptr, nullEnvironments, ignoreVisibility);
  }

  void RenderJobProcessor::CreateRenderJobs(RenderJobArray& jobArray,
                                            BVHPtr bvh,
                                            LightPtrArray& lights,
                                            CameraPtr camera,
                                            const EnvironmentComponentPtrArray& environments)
  {
    Mat4 project    = camera->GetProjectionMatrix();
    Mat4 view       = camera->GetViewMatrix();
    Frustum frustum = ExtractFrustum(project * view, false);

    EntityRawPtrArray nttiesInFrustum;
    bvh->FrustumTest(frustum, nttiesInFrustum);

    // Sort lights for disc.
    int directionalEndIndx = PreSortLights(lights);

    // Each entity can contain several meshes. This submeshIndexLookup array will be used
    // to find the index of the submehs for a given entity index.
    // Ex: Entity index is 4 and it has 3 submesh,
    // its submesh indexes would be = {4, 5, 6}
    // to look them up: {nttIndex + 0, nttIndex + 1, nttIndex + 3} formula is used.
    IntArray submeshIndexLookup;
    int size = 0;

    EntityRawPtrArray rawNtties;
    rawNtties.reserve(nttiesInFrustum.size());

    for (Entity* ntt : nttiesInFrustum)
    {
      if (ntt->IsVisible())
      {
        if (MeshComponent* meshComp = ntt->GetComponentFast<MeshComponent>())
        {
          rawNtties.push_back(ntt);
          meshComp->Init(false);
          submeshIndexLookup.push_back(size);
          size += meshComp->GetMeshVal()->GetMeshCount();
        }
      }
    }

    jobArray.clear();
    jobArray.resize(size); // at least.

    // Construct jobs.
    using poolstl::iota_iter;
    std::for_each(TKExecByConditional(rawNtties.size() > 100, WorkerManager::FramePool),
                  iota_iter<size_t>(0),
                  iota_iter<size_t>(rawNtties.size()),
                  [&](size_t nttIndex)
                  {
                    Entity* ntt                    = rawNtties[nttIndex];
                    MaterialPtrArray* materialList = nullptr;
                    if (MaterialComponent* matComp = ntt->GetComponentFast<MaterialComponent>())
                    {
                      materialList = &matComp->GetMaterialList();
                    }

                    MeshComponent* meshComp   = ntt->GetComponentFast<MeshComponent>();
                    const MeshPtr& parentMesh = meshComp->GetMeshVal();

                    MeshRawPtrArray allMeshes;
                    parentMesh->GetAllMeshes(allMeshes);

                    bool cullFlip  = ntt->m_node->RequresCullFlip();
                    Mat4 transform = ntt->m_node->GetTransform();
                    for (int subMeshIndx = 0; subMeshIndx < (int) allMeshes.size(); subMeshIndx++)
                    {
                      Mesh* mesh           = allMeshes[subMeshIndx];
                      MaterialPtr material = nullptr;

                      // Pick the material for submesh.
                      if (materialList != nullptr)
                      {
                        if (subMeshIndx < materialList->size())
                        {
                          material = (*materialList)[subMeshIndx];
                        }
                      }

                      // if material is still null, pick from mesh.
                      if (material == nullptr)
                      {
                        if (mesh->m_material)
                        {
                          material = mesh->m_material;
                        }
                      }

                      // Worst case, no material found pick a copy of default.
                      if (material == nullptr)
                      {
                        material = GetMaterialManager()->GetDefaultMaterial();
                        TK_WRN("Material component for entity: \"%s\" has less material than mesh count. Default "
                               "material used for meshes with missing material.",
                               ntt->GetNameVal().c_str());
                      }

                      // Translate nttIndex to corresponding job index.
                      int jobIndex       = submeshIndexLookup[nttIndex] + subMeshIndx;

                      RenderJob& job     = jobArray[jobIndex];
                      job.Entity         = ntt;
                      job.Mesh           = mesh;
                      job.Material       = material.get();
                      job.requireCullFlip = cullFlip;
                      job.ShadowCaster   = meshComp->GetCastShadowVal();
                      job.WorldTransform = ntt->m_node->GetTransform();
                      job.BoundingBox    = ntt->GetBoundingBox(true);

                      // Assign skeletal animations.
                      if (SkeletonComponent* skComp = ntt->GetComponentFast<SkeletonComponent>())
                      {
                        job.animData = skComp->GetAnimData(); // copy
                      }

                      AssignLight(job, lights, directionalEndIndx);
                      AssignEnvironment(job, environments);
                    }
                  });
  }

  void RenderJobProcessor::CreateRenderJobs(const EntityPtrArray& entities,
                                            RenderJobArray& jobArray,
                                            LightPtrArray& lights,
                                            const CameraPtr& camera,
                                            const EnvironmentComponentPtrArray& environments,
                                            bool ignoreVisibility)
  {
    CPU_FUNC_RANGE();

    // Create frustum for culling.
    Frustum frustum;
    if (camera != nullptr)
    {
      Mat4 pr = camera->GetProjectionMatrix();
      Mat4 v  = camera->GetViewMatrix();
      frustum = ExtractFrustum(pr * v, false);
    }

    // Sort lights for disc.
    int directionalEndIndx = PreSortLights(lights);

    // Each entity can contain several meshes. This submeshIndexLookup array will be used
    // to find the index of the submehs for a given entity index.
    // Ex: Entity index is 4 and it has 3 submesh,
    // its submesh indexes would be = {4, 5, 6}
    // to look them up: {nttIndex + 0, nttIndex + 1, nttIndex + 3} formula is used.
    IntArray submeshIndexLookup;
    submeshIndexLookup.reserve(entities.size());
    int size = 0;

    EntityRawPtrArray rawNtties;
    rawNtties.reserve(entities.size());

    for (const EntityPtr& ntt : entities)
    {
      if (ntt->IsVisible() || ignoreVisibility)
      {
        if (MeshComponent* meshComp = ntt->GetComponentFast<MeshComponent>())
        {
          meshComp->Init(false);
          submeshIndexLookup.push_back(size);
          size += meshComp->GetMeshVal()->GetMeshCount();
          rawNtties.push_back(ntt.get());
        }
      }
    }

    jobArray.clear();
    jobArray.resize(size); // at least.

    // Construct jobs.
    using poolstl::iota_iter;
    std::for_each(TKExecByConditional(rawNtties.size() > 100, WorkerManager::FramePool),
                  iota_iter<size_t>(0),
                  iota_iter<size_t>(rawNtties.size()),
                  [&](size_t nttIndex)
                  {
                    Entity* ntt                    = rawNtties[nttIndex];
                    MaterialPtrArray* materialList = nullptr;
                    if (MaterialComponent* matComp = ntt->GetComponentFast<MaterialComponent>())
                    {
                      materialList = &matComp->GetMaterialList();
                    }

                    MeshComponent* meshComp   = ntt->GetComponentFast<MeshComponent>();
                    const MeshPtr& parentMesh = meshComp->GetMeshVal();

                    MeshRawPtrArray allMeshes;
                    parentMesh->GetAllMeshes(allMeshes);

                    bool cullFlip  = ntt->m_node->RequresCullFlip();
                    Mat4 transform = ntt->m_node->GetTransform();
                    for (int subMeshIndx = 0; subMeshIndx < (int) allMeshes.size(); subMeshIndx++)
                    {
                      Mesh* mesh           = allMeshes[subMeshIndx];
                      MaterialPtr material = nullptr;

                      // Pick the material for submesh.
                      if (materialList != nullptr)
                      {
                        if (subMeshIndx < materialList->size())
                        {
                          material = (*materialList)[subMeshIndx];
                        }
                      }

                      // if material is still null, pick from mesh.
                      if (material == nullptr)
                      {
                        if (mesh->m_material)
                        {
                          material = mesh->m_material;
                        }
                      }

                      // Worst case, no material found pick a copy of default.
                      if (material == nullptr)
                      {
                        material = GetMaterialManager()->GetDefaultMaterial();
                        TK_WRN("Material component for entity: \"%s\" has less material than mesh count. Default "
                               "material used for meshes with missing material.",
                               ntt->GetNameVal().c_str());
                      }

                      // Translate nttIndex to corresponding job index.
                      int jobIndex        = submeshIndexLookup[nttIndex] + subMeshIndx;

                      RenderJob& job      = jobArray[jobIndex];
                      job.Entity          = ntt;
                      job.Mesh            = mesh;
                      job.Material        = material.get();
                      job.requireCullFlip = cullFlip;
                      job.ShadowCaster    = meshComp->GetCastShadowVal();
                      job.WorldTransform  = transform;
                      job.BoundingBox     = ntt->GetBoundingBox(true);

                      // Assign skeletal animations.
                      if (SkeletonComponent* skComp = ntt->GetComponentFast<SkeletonComponent>())
                      {
                        job.animData = skComp->GetAnimData(); // copy
                      }

                      if (camera)
                      {
                        // Cull.
                        job.frustumCulled = FrustumTest(frustum, job.BoundingBox);

                        // Light assignment.
                        if (!job.frustumCulled)
                        {
                          AssignEnvironment(job, environments);
                        }
                      }
                    }
                  });
  }

  void RenderJobProcessor::CullLights(LightPtrArray& lights, const CameraPtr& camera, float maxDistance)
  {
    Mat4 project          = camera->GetProjectionMatrix();
    Mat4 view             = camera->GetViewMatrix();
    Mat4 projectView      = project * view;
    Vec3 camPos           = camera->m_node->GetTranslation();
    Frustum frustum       = ExtractFrustum(projectView, false);
    Frustum normalFrustum = frustum;
    NormalizeFrustum(normalFrustum);

    erase_if(lights,
             [&](const LightPtr& light) -> bool
             {
               bool culled = false;
               switch (light->GetLightType())
               {
               case Light::Directional:
                 return false;
               case Light::Spot:
               {
                 SpotLight* spot = static_cast<SpotLight*>(light.get());
                 culled = FrustumBoxIntersection(frustum, spot->m_boundingBoxCache) == IntersectResult::Outside;
               }
               break;
               case Light::Point:
               {
                 PointLight* point = static_cast<PointLight*>(light.get());
                 culled            = !FrustumSphereIntersection(normalFrustum, point->m_boundingSphereCache);
               }
               break;
               default:
                 assert(false && "Unknown light type.");
                 return true;
               }

               if (culled)
               {
                 return true;
               }

               float dist = glm::distance(light->m_node->GetTranslation(), camPos);
               return dist > maxDistance;
             });
  }

  void RenderJobProcessor::SeperateRenderData(RenderData& renderData, bool forwardOnly)
  {
    // Group culled.
    RenderJobItr culledItr  = std::partition(renderData.jobs.begin(),
                                            renderData.jobs.end(),
                                            [](const RenderJob& job) { return job.frustumCulled; });

    RenderJobItr forwardItr = culledItr;
    RenderJobItr translucentItr;
    RenderJobItr deferredAlphaMaskedItr;
    RenderJobItr forwardAlphaMaskedItr;

    if (!forwardOnly)
    {
      // Group opaque deferred - forward.
      forwardItr = std::partition(culledItr,
                                  renderData.jobs.end(),
                                  [](const RenderJob& job)
                                  { return !job.Material->m_isShaderMaterial && !job.Material->IsTranslucent(); });

      deferredAlphaMaskedItr =
          std::partition(culledItr, forwardItr, [](const RenderJob& job) { return !job.Material->IsAlphaMasked(); });
    }

    // Group translucent.
    translucentItr = std::partition(forwardItr,
                                    renderData.jobs.end(),
                                    [](const RenderJob& job) { return !job.Material->IsTranslucent(); });

    forwardAlphaMaskedItr =
        std::partition(forwardItr, translucentItr, [](const RenderJob& job) { return !job.Material->IsAlphaMasked(); });

    if (forwardOnly)
    {
      renderData.deferredJobsStartIndex            = -1;
      renderData.deferredAlphaMaskedJobsStartIndex = -1;
    }
    else
    {
      renderData.deferredJobsStartIndex = (int) std::distance(renderData.jobs.begin(), culledItr);
      renderData.deferredAlphaMaskedJobsStartIndex =
          (int) std::distance(renderData.jobs.begin(), deferredAlphaMaskedItr);
    }

    renderData.forwardOpaqueStartIndex          = (int) std::distance(renderData.jobs.begin(), forwardItr);
    renderData.forwardAlphaMaskedJobsStartIndex = (int) std::distance(renderData.jobs.begin(), forwardAlphaMaskedItr);
    renderData.forwardTranslucentStartIndex     = (int) std::distance(renderData.jobs.begin(), translucentItr);
  }

  void RenderJobProcessor::AssignLight(RenderJob& job, LightPtrArray& lights, int startIndex)
  {
    for (int i = 0; i < startIndex; i++)
    {
      job.lights.push_back(lights[i].get());
      if (i >= RHIConstants::MaxLightsPerObject)
      {
        break;
      }
    }

    if (lights.size() == job.lights.size())
    {
      // No more lights to assign. Possibly editor lighting.
      return;
    }

    // Iterate lights that are inside of bvh nodes same with the job entity
    for (BVHNode* bvhNode : job.Entity->m_bvhNodes)
    {
      for (const EntityPtr& lightEntity : bvhNode->m_lights)
      {
        if (job.lights.size() >= RHIConstants::MaxLightsPerObject)
        {
          return;
        }

        Light* light = static_cast<Light*>(lightEntity.get());
        if (light->GetLightType() == Light::LightType::Spot)
        {
          SpotLight* spot = static_cast<SpotLight*>(light);
          if (FrustumBoxIntersection(spot->m_frustumCache, job.BoundingBox) != IntersectResult::Outside)
          {
            job.lights.push_back(light);
          }
        }
        else
        {
          assert(light->IsA<PointLight>());
          PointLight* point = static_cast<PointLight*>(light);
          if (SphereBoxIntersection(point->m_boundingSphereCache, job.BoundingBox))
          {
            job.lights.push_back(light);
          }
        }
      }
    }

    /*
    for (int i = startIndex; i < lights.size(); ++i)
    {
      if (job.lights.size() >= RHIConstants::MaxLightsPerObject)
      {
        return;
      }

      LightPtr& light = lights[i];
      if (light->GetLightType() == Light::LightType::Spot)
      {
        SpotLightPtr spot = Cast<SpotLight>(light);
        if (FrustumBoxIntersection(spot->m_frustumCache, job.BoundingBox) != IntersectResult::Outside)
        {
          job.lights.push_back(spot);
        }
      }
      else
      {
        PointLightLightPtr point = Cast<PointLight>(light);
        if (SphereBoxIntersection(point->m_boundingSphereCache, job.BoundingBox))
        {
          job.lights.push_back(point);
        }
      }
    }
    */
  }

  void RenderJobProcessor::AssignLight(RenderJobItr begin, RenderJobItr end, LightPtrArray& lights)
  {
    int directionalEndIndx = PreSortLights(lights);

    for (RenderJobItr job = begin; job != end; job++)
    {
      AssignLight(*job, lights, directionalEndIndx);
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
    for (size_t i = 0; i < lights.size(); i++)
    {
      if (lights[i]->GetLightType() == Light::Directional)
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

  void RenderJobProcessor::CullRenderJobs(const RenderJobArray& jobArray,
                                          const CameraPtr& camera,
                                          UIntArray& resultIndices)
  {
    FrustumCull(jobArray, camera, resultIndices);
  }

  void RenderJobProcessor::CullRenderJobs(const RenderJobArray& jobArray,
                                          const CameraPtr& camera,
                                          RenderJobArray& unCulledJobs)
  {
    FrustumCull(jobArray, camera, unCulledJobs);
  }

  void RenderJobProcessor::SortByMaterial(RenderData& renderData)
  {
    auto sortRangeFn = [](RenderJobItr begin, RenderJobItr end) -> void
    {
      std::sort(begin,
                end,
                [](const RenderJob& a, const RenderJob& b) -> bool
                { return a.Material->GetIdVal() < b.Material->GetIdVal(); });
    };

    RenderJobItr begin, end;

    if (renderData.deferredJobsStartIndex != -1)
    {
      begin = renderData.GetDefferedBegin();
      end   = renderData.GetDeferredAlphaMaskedBegin();
      sortRangeFn(begin, end);

      begin = renderData.GetDeferredAlphaMaskedBegin();
      end   = renderData.GetForwardOpaqueBegin();
      sortRangeFn(begin, end);
    }

    begin = renderData.GetForwardOpaqueBegin();
    end   = renderData.GetForwardAlphaMaskedBegin();
    sortRangeFn(begin, end);

    begin = renderData.GetForwardAlphaMaskedBegin();
    end   = renderData.GetForwardTranslucentBegin();
    sortRangeFn(begin, end);

    begin = renderData.GetForwardTranslucentBegin();
    end   = renderData.jobs.end();
    sortRangeFn(begin, end);
  }

  void RenderJobProcessor::AssignEnvironment(RenderJobItr begin,
                                             RenderJobItr end,
                                             const EnvironmentComponentPtrArray& environments)
  {
    CPU_FUNC_RANGE();

    if (environments.empty())
    {
      return;
    }

    for (RenderJobItr job = begin; job != end; job++)
    {
      AssignEnvironment(*job, environments);
    }
  }

  void RenderJobProcessor::AssignEnvironment(RenderJob& job, const EnvironmentComponentPtrArray& environments)
  {
    BoundingBox bestBox;
    job.EnvironmentVolume = nullptr;
    for (const EnvironmentComponentPtr& volume : environments)
    {
      if (volume->GetIlluminateVal() == false)
      {
        continue;
      }

      // Pick the smallest volume intersecting with job.
      BoundingBox vbb = std::move(volume->GetBoundingBox());
      if (BoxBoxIntersection(vbb, job.BoundingBox))
      {
        if (bestBox.Volume() > vbb.Volume() || job.EnvironmentVolume == nullptr)
        {
          bestBox               = vbb;
          job.EnvironmentVolume = volume.get();
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
