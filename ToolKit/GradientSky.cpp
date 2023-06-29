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

#include "GradientSky.h"

#include "EnvironmentComponent.h"
#include "RenderSystem.h"

#include "DebugNew.h"

namespace ToolKit
{

  TKDefineClass(GradientSky, SkyBase);

  GradientSky::GradientSky()
  {
    ParameterConstructor();
    ParameterEventConstructor();
  }

  GradientSky::~GradientSky() {}

  EntityType GradientSky::GetType() const { return EntityType::Entity_GradientSky; }

  void GradientSky::Init()
  {
    if (m_initialized)
    {
      return;
    }

    SkyBase::Init();

    // Skybox material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(ShaderPath("skyboxVert.shader", true));

    ShaderPtr frag = GetShaderManager()->Create<Shader>(ShaderPath("gradientSkyboxFrag.shader", true));

    ConstructSkyMaterial(vert, frag);

    if (m_onInit)
    {
      return;
    }

    m_onInit = true;
    RenderTask task {[this](Renderer* renderer) -> void
                     {
                       // Render gradient to cubemap and store the output
                       GenerateGradientCubemap();

                       // Create irradiance map from cubemap and set
                       GenerateIrradianceCubemap();
                     }};

    GetRenderSystem()->AddRenderTask(task);
  }

  MaterialPtr GradientSky::GetSkyboxMaterial()
  {
    Init();

    m_skyboxMaterial->m_fragmentShader->SetShaderParameter("topColor", ParameterVariant(GetTopColorVal()));

    m_skyboxMaterial->m_fragmentShader->SetShaderParameter("middleColor", ParameterVariant(GetMiddleColorVal()));

    m_skyboxMaterial->m_fragmentShader->SetShaderParameter("bottomColor", ParameterVariant(GetBottomColorVal()));

    m_skyboxMaterial->m_fragmentShader->SetShaderParameter("exponent", ParameterVariant(GetGradientExponentVal()));

    return m_skyboxMaterial;
  }

  void GradientSky::ParameterConstructor()
  {
    TopColor_Define(Vec3(0.3f, 0.3f, 1.0f), "Sky", 90, true, true, {true});
    MiddleColor_Define(Vec3(1.0f, 1.0f, 0.8f), "Sky", 90, true, true, {true});
    BottomColor_Define(Vec3(0.5f, 0.3f, 0.1f), "Sky", 90, true, true, {true});
    GradientExponent_Define(0.3f, "Sky", 90, true, true, {false, true, 0.0f, 10.0f, 0.02f});

    IrradianceResolution_Define(64.0f, "Sky", 90, true, true, {false, true, 32.0f, 2048.0f, 2.0f});

    SetNameVal("Gradient Sky");
  }

  void GradientSky::ParameterEventConstructor() { SkyBase::ParameterEventConstructor(); }

  void GradientSky::GenerateGradientCubemap()
  {
    RenderTask task = {
        [this](Renderer* renderer) -> void
        {
          FramebufferPtr fb = std::make_shared<Framebuffer>();
          fb->Init({m_size, m_size, false, true});

          const RenderTargetSettigs set = {0,
                                           GraphicTypes::TargetCubeMap,
                                           GraphicTypes::UVClampToEdge,
                                           GraphicTypes::UVClampToEdge,
                                           GraphicTypes::UVClampToEdge,
                                           GraphicTypes::SampleLinear,
                                           GraphicTypes::SampleLinear,
                                           GraphicTypes::FormatRGB,
                                           GraphicTypes::FormatRGB,
                                           GraphicTypes::TypeUnsignedByte};

          RenderTargetPtr cubemap       = std::make_shared<RenderTarget>(m_size, m_size, set);

          cubemap->Init();

          // Create material
          m_skyboxMaterial->m_fragmentShader->SetShaderParameter("topColor", ParameterVariant(GetTopColorVal()));

          m_skyboxMaterial->m_fragmentShader->SetShaderParameter("middleColor", ParameterVariant(GetMiddleColorVal()));

          m_skyboxMaterial->m_fragmentShader->SetShaderParameter("bottomColor", ParameterVariant(GetBottomColorVal()));

          m_skyboxMaterial->m_fragmentShader->SetShaderParameter("exponent",
                                                                 ParameterVariant(GetGradientExponentVal()));

          renderer->EnableDepthTest(false);

          // Views for 6 different angles
          CameraPtr cam = std::make_shared<Camera>();
          cam->SetLens(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

          Mat4 views[] = {glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                          glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                          glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
                          glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
                          glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
                          glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

          for (int i = 0; i < 6; ++i)
          {
            Vec3 pos;
            Quaternion rot;
            Vec3 sca;
            DecomposeMatrix(views[i], &pos, &rot, &sca);

            cam->m_node->SetTranslation(ZERO, TransformationSpace::TS_WORLD);
            cam->m_node->SetOrientation(rot, TransformationSpace::TS_WORLD);
            cam->m_node->SetScale(sca);

            fb->SetAttachment(Framebuffer::Attachment::ColorAttachment0, cubemap, 0, -1, (Framebuffer::CubemapFace) i);

            renderer->SetFramebuffer(fb, true, Vec4(0.0f));
            renderer->DrawCube(cam.get(), m_skyboxMaterial);
          }

          renderer->EnableDepthTest(true);

          // Take the ownership of render target.
          GetHdri()->m_cubemap = std::make_shared<CubeMap>(cubemap->m_textureId);

          cubemap->m_textureId = 0;
          cubemap              = nullptr;
        }};

    GetRenderSystem()->AddRenderTask(task);
  }

  void GradientSky::GenerateIrradianceCubemap()
  {
    RenderTask task = {[this](Renderer* renderer) -> void
                       {
                         HdriPtr hdr              = GetHdri();
                         uint irRes               = (uint) GetIrradianceResolutionVal();

                         hdr->m_irradianceCubemap = renderer->GenerateEnvIrradianceMap(hdr->m_cubemap, irRes, irRes);

                         hdr->m_prefilteredEnvMap =
                             renderer->GenerateEnvPrefilteredMap(hdr->m_cubemap,
                                                                 irRes,
                                                                 irRes,
                                                                 Renderer::RHIConstants::specularIBLLods);

                         if (m_onInit)
                         {
                           m_initialized = true;
                           m_onInit      = false;
                         }
                       }};

    GetRenderSystem()->AddRenderTask(task);
  }

  XmlNode* GradientSky::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

} // namespace ToolKit