#include "ForwardPass.h"

#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"

namespace ToolKit
{

  ForwardRenderPass::ForwardRenderPass() {}

  ForwardRenderPass::ForwardRenderPass(const ForwardRenderPassParams& params)
      : m_params(params)
  {
    // Create a default frame buffer.
    if (m_params.FrameBuffer == nullptr)
    {
      m_params.FrameBuffer = std::make_shared<Framebuffer>();
      m_params.FrameBuffer->Init({1024u, 768u, false, true});
    }
  }

  void ForwardRenderPass::Render()
  {
    EntityRawPtrArray opaqueDrawList;
    EntityRawPtrArray translucentDrawList;
    SeperateTranslucentEntities(m_drawList,
                                opaqueDrawList,
                                translucentDrawList);

    RenderOpaque(opaqueDrawList, m_camera, m_params.Lights);

    RenderTranslucent(translucentDrawList, m_camera, m_params.Lights);
  }

  ForwardRenderPass::~ForwardRenderPass() {}

  void ForwardRenderPass::PreRender()
  {
    Pass::PreRender();

    Renderer* renderer = GetRenderer();

    // Set self data.
    m_drawList         = m_params.Entities;
    m_camera           = m_params.Cam;

    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.ClearFrameBuffer);
    renderer->SetCameraLens(m_camera);
  }

  void ForwardRenderPass::PostRender()
  {
    Pass::PostRender();
    GetRenderer()->m_overrideMat = nullptr;
  }

  void ForwardRenderPass::RenderOpaque(EntityRawPtrArray entities,
                                       Camera* cam,
                                       const LightRawPtrArray& lights)
  {
    Renderer* renderer = GetRenderer();
    for (Entity* ntt : entities)
    {
      LightRawPtrArray lightList = GetBestLights(ntt, lights);
      uint activeMeshIndx        = 0;

      MultiMaterialPtr mmComp    = ntt->GetComponent<MultiMaterialComponent>();

      if (mmComp == nullptr)
      {
        renderer->Render(ntt, cam, lightList);
      }
      else
      {
        MeshComponentPtrArray meshComps;
        ntt->GetComponent<MeshComponent>(meshComps);
        for (MeshComponentPtr meshComp : meshComps)
        {
          MeshRawCPtrArray meshes;
          meshComp->GetMeshVal()->GetAllMeshes(meshes);
          for (uint meshIndx = 0; meshIndx < meshes.size();
               meshIndx++, activeMeshIndx++)
          {
            if (mmComp && mmComp->GetMaterialList().size() > activeMeshIndx)
            {
              MaterialPtr mat = mmComp->GetMaterialList()[activeMeshIndx];
              if (mat->GetRenderState()->useForwardPath &&
                  (mat->GetRenderState()->blendFunction ==
                       BlendFunction::NONE ||
                   mat->GetRenderState()->blendFunction ==
                       BlendFunction::ALPHA_MASK))
              {
                renderer->Render(ntt, cam, lightList, {activeMeshIndx});
              }
            }
          }
        }
      }
    }
  }

  void ForwardRenderPass::RenderTranslucent(EntityRawPtrArray entities,
                                            Camera* cam,
                                            const LightRawPtrArray& lights)
  {
    StableSortByDistanceToCamera(entities, cam);

    Renderer* renderer = GetRenderer();
    for (Entity* ntt : entities)
    {

      uint activeMeshIndx        = 0;
      MultiMaterialPtr mmComp    = ntt->GetComponent<MultiMaterialComponent>();
      MaterialPtr renderMaterial = ntt->GetRenderMaterial();
      auto renderFnc =
          [ntt, cam, lights, renderer](MaterialPtr renderMaterial,
                                       const UIntArray& meshIndices)
      {
        LightRawPtrArray culledLights = GetBestLights(ntt, lights);
        if (renderMaterial->GetRenderState()->cullMode == CullingType::TwoSided)
        {
          renderMaterial->GetRenderState()->cullMode = CullingType::Front;
          renderer->Render(ntt, cam, culledLights, meshIndices);

          renderMaterial->GetRenderState()->cullMode = CullingType::Back;
          renderer->Render(ntt, cam, culledLights, meshIndices);

          renderMaterial->GetRenderState()->cullMode = CullingType::TwoSided;
        }
        else
        {
          renderer->Render(ntt, cam, culledLights, meshIndices);
        }
      };

      if (mmComp == nullptr)
      {
        renderFnc(renderMaterial, {});
      }
      else
      {
        MeshComponentPtrArray meshComps;
        ntt->GetComponent<MeshComponent>(meshComps);
        for (MeshComponentPtr meshComp : meshComps)
        {
          MeshRawCPtrArray meshes;
          meshComp->GetMeshVal()->GetAllMeshes(meshes);
          for (uint meshIndx = 0; meshIndx < meshes.size();
               meshIndx++, activeMeshIndx++)
          {
            if (mmComp && mmComp->GetMaterialList().size() > activeMeshIndx)
            {
              renderMaterial = mmComp->GetMaterialList()[activeMeshIndx];
              if (!renderMaterial->GetRenderState()->useForwardPath ||
                  (renderMaterial->GetRenderState()->blendFunction ==
                       BlendFunction::NONE ||
                   renderMaterial->GetRenderState()->blendFunction ==
                       BlendFunction::ALPHA_MASK))
              {
                continue;
              }
            }
            renderFnc(renderMaterial, {activeMeshIndx});
          }
        }
      }
    }
  }

} // namespace ToolKit