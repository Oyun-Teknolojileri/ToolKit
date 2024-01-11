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
    {
      bool isDrawable = ntt->IsDrawable();
      bool isVisbile  = ntt->IsVisible();
      return isDrawable && (isVisbile || ignoreVisibility);
    };

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
      SkeletonComponentPtr skComp              = ntt->GetComponent<SkeletonComponent>();
      AnimationPtr currentAnimation            = nullptr;
      AnimationPtr blendAnimation              = nullptr;
      float blendFactor                        = 0.0f;
      const AnimRecordRawPtrArray& animRecords = GetAnimationPlayer()->m_records;
      for (AnimRecordRawPtr animRecord : animRecords)
      {
        if (EntityPtr animNtt = animRecord->m_entity.lock())
        {
          if (animNtt->GetIdVal() == ntt->GetIdVal())
          {
            currentAnimation = animRecord->m_animation;
            blendAnimation   = animRecord->m_blendAnimation;
            blendFactor      = animRecord->m_blendFactor;
            break;
          }
        }
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
            float frameKeyCount                    = (float) skComp->GetAnimKeyFrameCount();
            job.animData.firstKeyFrame             = (float) skComp->GetAnimFirstKeyFrame() / frameKeyCount;
            job.animData.secondKeyFrame            = (float) skComp->GetAnimSecondKeyFrame() / frameKeyCount;
            job.animData.keyFrameCount             = frameKeyCount;
            job.animData.keyFrameInterpolationTime = skComp->GetAnimKeyFrameInterpolateTime();
            job.animData.currentAnimation          = currentAnimation;
            job.animData.blendAnimation            = blendAnimation;
            job.animData.animationBlendFactor      = blendFactor;
          }

          jobArray.push_back(job);
        }
      };

      MeshRawPtrArray allMeshes;
      parentMesh->GetAllMeshes(allMeshes);
      for (Mesh* mesh : allMeshes)
      {
        addRenderJobForMeshFn(mesh);
      }

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

  struct LightSortStruct
  {
    LightPtr light      = nullptr;
    uint intersectCount = 0;
  };

  bool CompareLightIntersects(const LightSortStruct& i1, const LightSortStruct& i2)
  {
    return (i1.intersectCount > i2.intersectCount);
  }

  int RenderJobProcessor::SortLights(const RenderJob& job, LightPtrArray& lights, int startFromIndex)
  {
    std::vector<LightSortStruct> intersectCounts;
    intersectCounts.resize(lights.size() - startFromIndex);
    const BoundingBox& aabb = job.BoundingBox;

    // CAVIATE
    // This loop will move all light pointers to intersectCounts. Do not access lights afterwards.
    for (uint lightIndx = startFromIndex; lightIndx < lights.size(); lightIndx++)
    {
      LightPtr light = std::move(lights[lightIndx]);
      assert(light->IsA<SpotLight>() || light->IsA<PointLight>());

      uint& curIntersectCount = intersectCounts[lightIndx - startFromIndex].intersectCount;

      if (SpotLight* spot = light->As<SpotLight>())
      {
        // The shadow camera of light should be updated before accessing the frustum.
        // RenderPath PreRender functions should do that.
        if (FrustumBoxIntersection(spot->m_frustumCache, aabb) != IntersectResult::Outside)
        {
          curIntersectCount++;
        }
      }
      else if (PointLight* point = light->As<PointLight>())
      {
        if (SphereBoxIntersection(point->m_boundingSphereCache, aabb))
        {
          curIntersectCount++;
        }
      }

      intersectCounts[lightIndx - startFromIndex].light = std::move(light);
    }

    // Sort point & spot lights
    std::sort(intersectCounts.begin(), intersectCounts.end(), CompareLightIntersects);

    // CAVIATE
    // This loop will move all lights back to ligts array in a sorted way based on importance.
    int effectingLights = 0;
    for (size_t i = 0; i < intersectCounts.size(); i++)
    {
      LightSortStruct& ls        = intersectCounts[i];
      lights[i + startFromIndex] = std::move(ls.light);
      if (ls.intersectCount > 0)
      {
        effectingLights++;
      }
    }

    // All directional lights plus lights that intersect with job.
    return effectingLights + startFromIndex;
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

  LightPtrArray RenderJobProcessor::SortLights(EntityPtr entity, LightPtrArray& lights)
  {
    CPU_FUNC_RANGE();

    for (LightPtr light : lights)
    {
      light->UpdateShadowCamera();
    }

    RenderJobArray jobs;
    EntityPtrArray oneEntity = {entity};
    CreateRenderJobs(oneEntity, jobs);

    int startIndex = PreSortLights(lights);

    LightPtrArray allLights;
    for (RenderJob& rj : jobs)
    {
      int effectiveLights = SortLights(rj, lights, startIndex);
      allLights.insert(allLights.end(), lights.begin(), lights.begin() + effectiveLights);
    }

    return allLights;
  }

  void RenderJobProcessor::SortByDistanceToCamera(RenderJobArray& jobArray, const CameraPtr cam)
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
