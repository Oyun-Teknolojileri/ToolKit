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

#include "ShadowPass.h"

#include "Camera.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "ToolKit.h"

namespace ToolKit
{

  ShadowPass::ShadowPass()
  {
    Mat4 views[6] = {glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                     glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
                     glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    for (int i = 0; i < 6; ++i)
    {
      DecomposeMatrix(views[i], nullptr, &m_cubeMapRotations[i], &m_cubeMapScales[i]);
    }

    m_shadowAtlas       = std::make_shared<RenderTarget>();
    m_shadowFramebuffer = std::make_shared<Framebuffer>();
  }

  ShadowPass::ShadowPass(const ShadowPassParams& params) : ShadowPass() { m_params = params; }

  ShadowPass::~ShadowPass() {}

  void ShadowPass::Render()
  {
    const Vec4 lastClearColor = GetRenderer()->m_clearColor;

    // Update shadow maps.
    for (LightPtr light : m_params.Lights)
    {
      light->InitShadowMapDepthMaterial();
      if (DirectionalLight* dLight = light->As<DirectionalLight>())
      {
        dLight->UpdateShadowFrustum(m_params.RendeJobs);
      }
      else
      {
        light->UpdateShadowCamera();
      }

      RenderShadowMaps(light, m_params.RendeJobs);
    }

    GetRenderer()->m_clearColor = lastClearColor;
  }

  void ShadowPass::PreRender()
  {
    Pass::PreRender();

    m_lastOverrideMat = GetRenderer()->m_overrideMat;

    // Dropout non shadow casters.
    erase_if(m_params.RendeJobs, [](RenderJob& job) -> bool { return !job.ShadowCaster; });

    // Dropout non shadow casting lights.
    erase_if(m_params.Lights, [](Light* light) -> bool { return !light->GetCastShadowVal(); });

    InitShadowAtlas();

    // Set all shadow atlas layers uncleared
    if (m_layerCount != m_clearedLayers.size())
    {
      m_clearedLayers.resize(m_layerCount);
    }

    for (int i = 0; i < m_layerCount; i++)
    {
      m_clearedLayers[i] = false;
    }
  }

  void ShadowPass::PostRender()
  {
    GetRenderer()->m_overrideMat = m_lastOverrideMat;
    Pass::PostRender();
  }

  RenderTargetPtr ShadowPass::GetShadowAtlas() { return m_shadowAtlas; }

  void ShadowPass::RenderShadowMaps(LightPtr light, const RenderJobArray& jobs)
  {
    Renderer* renderer        = GetRenderer();

    auto renderForShadowMapFn = [this, &renderer](LightPtr light, RenderJobArray jobs) -> void
    {
      FrustumCull(jobs, light->m_shadowCamera);

      renderer->m_overrideMat = light->GetShadowMaterial();
      for (RenderJob& job : jobs)
      {
        MaterialPtr material = job.Material;
        renderer->m_overrideMat->SetRenderState(material->GetRenderState());
        renderer->m_overrideMat->UnInit();
        renderer->m_overrideMat->SetAlpha(material->GetAlpha());
        renderer->m_overrideMat->m_diffuseTexture                = material->m_diffuseTexture;
        renderer->m_overrideMat->GetRenderState()->blendFunction = BlendFunction::NONE;
        renderer->m_overrideMat->Init();
        renderer->Render(job, light->m_shadowCamera);
      }
    };

    switch (light->GetType())
    {
    case EntityType::Entity_PointLight:
    {
      renderer->SetFramebuffer(m_shadowFramebuffer, false);

      for (int i = 0; i < 6; ++i)
      {
        m_shadowFramebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                           m_shadowAtlas,
                                           0,
                                           light->m_shadowAtlasLayer + i);

        // Clear the layer if needed
        if (!m_clearedLayers[light->m_shadowAtlasLayer + i])
        {
          renderer->ClearBuffer(GraphicBitFields::AllBits, m_shadowClearColor);
          m_clearedLayers[light->m_shadowAtlasLayer + i] = true;
        }
        else
        {
          renderer->ClearBuffer(GraphicBitFields::DepthBits, m_shadowClearColor);
        }

        light->m_shadowCamera->m_node->SetTranslation(light->m_node->GetTranslation());
        light->m_shadowCamera->m_node->SetOrientation(m_cubeMapRotations[i]);

        // TODO: Scales are not needed. Remove.
        light->m_shadowCamera->m_node->SetScale(m_cubeMapScales[i]);

        renderer->SetViewportSize((uint) light->m_shadowAtlasCoord.x,
                                  (uint) light->m_shadowAtlasCoord.y,
                                  (uint) light->GetShadowResVal(),
                                  (uint) light->GetShadowResVal());

        renderForShadowMapFn(light, jobs);
      }
    }
    break;
    case EntityType::Entity_DirectionalLight:
    case EntityType::Entity_SpotLight:
    {

      renderer->SetFramebuffer(m_shadowFramebuffer, false);
      m_shadowFramebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                         m_shadowAtlas,
                                         0,
                                         light->m_shadowAtlasLayer);

      // Clear the layer if needed
      if (!m_clearedLayers[light->m_shadowAtlasLayer])
      {
        renderer->ClearBuffer(GraphicBitFields::AllBits, m_shadowClearColor);
        m_clearedLayers[light->m_shadowAtlasLayer] = true;
      }
      else
      {
        renderer->ClearBuffer(GraphicBitFields::DepthBits, m_shadowClearColor);
      }

      renderer->SetViewportSize((uint) light->m_shadowAtlasCoord.x,
                                (uint) light->m_shadowAtlasCoord.y,
                                (uint) light->GetShadowResVal(),
                                (uint) light->GetShadowResVal());

      renderForShadowMapFn(light, jobs);
    }
    break;
    default:
      break;
    }
  }

  int ShadowPass::PlaceShadowMapsToShadowAtlas(const LightPtrArray& lights)
  {
    int layerCount                           = -1;
    int lastLayerOfDirAndSpotLightShadowsUse = -1;

    // Create 2 arrays: dirandspotlights, point lights
    LightPtrArray dirAndSpotLights           = lights;
    LightPtrArray pointLights;
    LightPtrArray::iterator it = dirAndSpotLights.begin();
    while (it != dirAndSpotLights.end())
    {
      if ((*it)->GetType() == EntityType::Entity_PointLight)
      {
        pointLights.push_back(*it);
        it = dirAndSpotLights.erase(it);
      }
      else
      {
        ++it;
      }
    }

    // Sort lights based on resolutions (greater to smaller)
    auto sortByResFn = [](const Light* l1, const Light* l2) -> bool
    { return l1->GetShadowResVal() > l2->GetShadowResVal(); };

    std::sort(dirAndSpotLights.begin(), dirAndSpotLights.end(), sortByResFn);
    std::sort(pointLights.begin(), pointLights.end(), sortByResFn);

    // Get dir and spot lights into the pack
    std::vector<int> resolutions;
    resolutions.reserve(dirAndSpotLights.size());
    for (LightPtr light : dirAndSpotLights)
    {
      resolutions.push_back((int) light->GetShadowResVal());
    }

    std::vector<BinPack2D::PackedRect> rects =
        m_packer.Pack(resolutions, Renderer::m_rhiSettings::g_shadowAtlasTextureSize);

    for (int i = 0; i < rects.size(); ++i)
    {
      dirAndSpotLights[i]->m_shadowAtlasCoord = rects[i].Coord;
      dirAndSpotLights[i]->m_shadowAtlasLayer = rects[i].ArrayIndex;

      lastLayerOfDirAndSpotLightShadowsUse    = rects[i].ArrayIndex;
      layerCount                              = std::max(rects[i].ArrayIndex, layerCount);
    }

    // Get point light into another pack
    resolutions.clear();
    resolutions.reserve(pointLights.size());
    for (LightPtr light : pointLights)
    {
      resolutions.push_back((int) light->GetShadowResVal());
    }

    rects = m_packer.Pack(resolutions, Renderer::m_rhiSettings::g_shadowAtlasTextureSize);

    for (int i = 0; i < rects.size(); ++i)
    {
      pointLights[i]->m_shadowAtlasCoord = rects[i].Coord;
      pointLights[i]->m_shadowAtlasLayer = rects[i].ArrayIndex;
    }

    // Adjust point light parameters
    for (LightPtr light : pointLights)
    {
      light->m_shadowAtlasLayer += lastLayerOfDirAndSpotLightShadowsUse + 1;
      light->m_shadowAtlasLayer *= 6;
      layerCount                = std::max(light->m_shadowAtlasLayer + 5, layerCount);
    }

    return layerCount + 1;
  }

  void ShadowPass::InitShadowAtlas()
  {
    // Check if the shadow atlas needs to be updated
    bool needChange = false;

    // After this loop lastShadowLights is set with lights with shadows
    int nextId      = 0;
    for (int i = 0; i < m_params.Lights.size(); ++i)
    {
      LightPtr light = m_params.Lights[i];
      if (light->m_shadowResolutionUpdated)
      {
        light->m_shadowResolutionUpdated = false;
        needChange                       = true;
      }

      if (nextId >= m_previousShadowCasters.size())
      {
        needChange = true;
        m_previousShadowCasters.push_back(light->GetIdVal());
        nextId++;
        continue;
      }

      if (m_previousShadowCasters[nextId] != light->GetIdVal())
      {
        needChange = true;
      }

      m_previousShadowCasters[nextId] = light->GetIdVal();
      nextId++;
    }

    if (needChange)
    {
      m_previousShadowCasters.resize(nextId);

      // Place shadow textures to atlas
      m_layerCount        = PlaceShadowMapsToShadowAtlas(m_params.Lights);

      const int maxLayers = GetRenderer()->GetMaxArrayTextureLayers();
      if (maxLayers < m_layerCount)
      {
        m_layerCount = maxLayers;
        GetLogger()->Log("ERROR: Max array texture layer size is reached: " + std::to_string(maxLayers) + " !");
      }

      const RenderTargetSettigs set = {0,
                                       GraphicTypes::Target2DArray,
                                       GraphicTypes::UVClampToEdge,
                                       GraphicTypes::UVClampToEdge,
                                       GraphicTypes::UVClampToEdge,
                                       GraphicTypes::SampleNearest,
                                       GraphicTypes::SampleNearest,
                                       GraphicTypes::FormatRG32F,
                                       GraphicTypes::FormatRG,
                                       GraphicTypes::TypeFloat,
                                       m_layerCount};

      m_shadowAtlas->Reconstruct(Renderer::m_rhiSettings::g_shadowAtlasTextureSize,
                                 Renderer::m_rhiSettings::g_shadowAtlasTextureSize,
                                 set);

      if (!m_shadowFramebuffer->Initialized())
      {
        m_shadowFramebuffer->Init({Renderer::m_rhiSettings::g_shadowAtlasTextureSize,
                                   Renderer::m_rhiSettings::g_shadowAtlasTextureSize,
                                   false,
                                   true});
      }
    }
  }

} // namespace ToolKit