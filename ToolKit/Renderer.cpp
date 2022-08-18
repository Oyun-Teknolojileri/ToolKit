#include "Renderer.h"

#include <algorithm>

#include "Mesh.h"
#include "Drawable.h"
#include "Texture.h"
#include "Camera.h"
#include "Node.h"
#include "Material.h"
#include "Surface.h"
#include "Skeleton.h"
#include "GlobalCache.h"
#include "Viewport.h"
#include "Scene.h"
#include "UIManager.h"
#include "Shader.h"
#include "ToolKit.h"
#include "DirectionComponent.h"
#include "ResourceComponent.h"
#include "GL/glew.h"
#include "DebugNew.h"

namespace ToolKit
{
#define BUFFER_OFFSET(idx) (static_cast<char*>(0) + (idx))

  Renderer::Renderer()
  {
  }

  Renderer::~Renderer()
  {
  }

  void Renderer::RenderScene
  (
    const ScenePtr scene,
    Viewport* viewport,
    const LightRawPtrArray& editorLights
  )
  {
    Camera* cam = viewport->GetCamera();
    EntityRawPtrArray entities = scene->GetEntities();

    SetViewport(viewport);

    RenderEntities(entities, cam, viewport, editorLights);

    if (!cam->IsOrtographic())
    {
      RenderSky(scene->GetSky(), cam);
    }
  }

  /**
  * Renders given UILayer's to given Viewport.
  * @param uiLayers UILayer pointer array that will be rendered.
  * @param viewport Viewport that UILayer's are going to rendered with.
  */
  void Renderer::RenderUI(const UILayerPtrArray& uiLayers, Viewport* viewport)
  {
    if (uiLayers[0] == nullptr)
    {
      return;
    }

    for (UILayer* layer : uiLayers)
    {
      if (Entity* rootNode = layer->GetLayer(layer->m_layerName))
      {
        EntityRawPtrArray allEntities;
        GetChildren(rootNode, allEntities);

        float halfWidth = viewport->m_width * 0.5f;
        float halfHeight = viewport->m_height * 0.5f;

        layer->m_cam->SetLens
        (
          -halfWidth,
          halfWidth,
          -halfHeight,
          halfHeight,
          0.5f,
          1000.0f
        );

        RenderEntities(allEntities, layer->m_cam, viewport);
      }
    }
  }

  void Renderer::Render
  (
    Entity* ntt,
    Camera* cam,
    const LightRawPtrArray& lights
  )
  {
    MeshComponentPtrArray meshComponents;
    ntt->GetComponent<MeshComponent>(meshComponents);

    MaterialPtr nttMat;
    if (MaterialComponentPtr matCom = ntt->GetComponent<MaterialComponent>())
    {
      if (nttMat = matCom->GetMaterialVal())
      {
        nttMat->Init();
      }
    }

    for (MeshComponentPtr meshCom : meshComponents)
    {
      MeshPtr mesh = meshCom->GetMeshVal();
      m_lights = GetBestLights(ntt, lights);
      m_cam = cam;
      SetProjectViewModel(ntt, cam);
      if (mesh->IsSkinned())
      {
        RenderSkinned(ntt, cam);
        return;
      }

      mesh->Init();

      MeshRawPtrArray meshCollector;
      mesh->GetAllMeshes(meshCollector);


      for (Mesh* mesh : meshCollector)
      {
        m_textureIdCount = 0;
        if (m_overrideMat != nullptr)
        {
          m_mat = m_overrideMat.get();
        }
        else
        {
          m_mat = nttMat ? nttMat.get() : mesh->m_material.get();
        }

        ProgramPtr prg = CreateProgram
        (
          m_mat->m_vertexShader,
          m_mat->m_fragmetShader
        );

        BindProgram(prg);
        FeedUniforms(prg);

        RenderState* rs = m_mat->GetRenderState();
        SetRenderState(rs, prg);

        glBindVertexArray(mesh->m_vaoId);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vboVertexId);
        SetVertexLayout(VertexLayout::Mesh);

        if (mesh->m_indexCount != 0)
        {
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_vboIndexId);
          glDrawElements
          (
            (GLenum)rs->drawType,
            mesh->m_indexCount,
            GL_UNSIGNED_INT,
            nullptr
          );
        }
        else
        {
          glDrawArrays((GLenum)rs->drawType, 0, mesh->m_vertexCount);
        }
      }
    }
  }

  void Renderer::RenderSkinned(Entity* object, Camera* cam)
  {
    MeshPtr mesh = object->GetMeshComponent()->GetMeshVal();
    SetProjectViewModel(object, cam);

    static ShaderPtr skinShader = GetShaderManager()->Create<Shader>
    (
      ShaderPath("defaultSkin.shader", true)
    );
    static ProgramPtr skinProg = CreateProgram
    (
      skinShader,
      mesh->m_material->m_fragmetShader
    );
    BindProgram(skinProg);
    FeedUniforms(skinProg);

    SkeletonPtr skeleton = static_cast<SkinMesh*> (mesh.get())->m_skeleton;
    skeleton->UpdateTransformationTexture();
    for (int i = 0; i < static_cast<int>(skeleton->m_bones.size()); i++)
    {
      Bone* bone = skeleton->m_bones[i];;
      GLint loc = glGetUniformLocation
      (
        skinProg->m_handle,
        "boneTransform_textures"
      );
      glUniform1i(loc, m_textureIdCount);
      glActiveTexture(GL_TEXTURE0 + m_textureIdCount++);
      glBindTexture
      (
        GL_TEXTURE_2D,
        skeleton->m_boneTransformTexture->m_textureId
      );

      loc = glGetUniformLocation
      (
        skinProg->m_handle,
        "boneBindPose_textures"
      );
      glUniform1i(loc, m_textureIdCount);
      glActiveTexture(GL_TEXTURE0 + m_textureIdCount++);
      glBindTexture(GL_TEXTURE_2D, skeleton->m_bindPoseTexture->m_textureId);

      loc = glGetUniformLocation(skinProg->m_handle, "numBones");
      float boneCount = static_cast<float>(skeleton->m_bones.size());
      glUniform1fv(loc, 1, &boneCount);
    }

    MeshRawPtrArray meshCollector;
    mesh->GetAllMeshes(meshCollector);

    for (Mesh* mesh : meshCollector)
    {
      RenderState* rs = mesh->m_material->GetRenderState();
      SetRenderState(rs, skinProg);

      glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vboVertexId);
      SetVertexLayout(VertexLayout::SkinMesh);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_vboIndexId);
      glDrawElements
      (
        (GLenum)rs->drawType,
        mesh->m_indexCount,
        GL_UNSIGNED_INT,
        nullptr
      );
    }
  }

  void Renderer::Render2d(Surface* object, glm::ivec2 screenDimensions)
  {
    static ShaderPtr vertexShader = GetShaderManager()->Create<Shader>
    (
      ShaderPath("defaultVertex.shader", true)
    );
    static ShaderPtr fragShader = GetShaderManager()->Create<Shader>
    (
      ShaderPath("unlitFrag.shader", true)
    );
    static ProgramPtr prog = CreateProgram(vertexShader, fragShader);
    BindProgram(prog);

    MeshPtr mesh = object->GetMeshComponent()->GetMeshVal();
    mesh->Init();

    RenderState* rs = mesh->m_material->GetRenderState();
    SetRenderState(rs, prog);

    GLint pvloc = glGetUniformLocation(prog->m_handle, "ProjectViewModel");
    Mat4 pm = glm::ortho
    (
      0.0f,
      static_cast<float>(screenDimensions.x),
      0.0f,
      static_cast<float>(screenDimensions.y),
      0.0f,
      100.0f
    );

    Mat4 mul = pm * object->m_node->GetTransform
    (
      TransformationSpace::TS_WORLD
    );

    glUniformMatrix4fv(pvloc, 1, false, reinterpret_cast<float*>(&mul));

    glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vboVertexId);
    SetVertexLayout(VertexLayout::Mesh);

    glDrawArrays((GLenum)rs->drawType, 0, mesh->m_vertexCount);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    SetVertexLayout(VertexLayout::None);
  }

  void Renderer::Render2d(SpriteAnimation* object, glm::ivec2 screenDimensions)
  {
    Surface* surface = object->GetCurrentSurface();

    Node* backup = surface->m_node;
    surface->m_node = object->m_node;

    Render2d(surface, screenDimensions);

    surface->m_node = backup;
  }

  void Renderer::SetRenderState
  (
    const RenderState* const state,
    ProgramPtr program
  )
  {
    if (m_renderState.cullMode != state->cullMode)
    {
      if (state->cullMode == CullingType::TwoSided)
      {
        glDisable(GL_CULL_FACE);
      }

      if (state->cullMode == CullingType::Front)
      {
        if (m_renderState.cullMode == CullingType::TwoSided)
        {
          glEnable(GL_CULL_FACE);
        }
        glCullFace(GL_FRONT);
      }

      if (state->cullMode == CullingType::Back)
      {
        if (m_renderState.cullMode == CullingType::TwoSided)
        {
          glEnable(GL_CULL_FACE);
        }
        glCullFace(GL_BACK);
      }

      m_renderState.cullMode = state->cullMode;
    }

    if (m_renderState.depthTestEnabled != state->depthTestEnabled)
    {
      if (state->depthTestEnabled)
      {
        glEnable(GL_DEPTH_TEST);
      }
      else
      {
        glDisable(GL_DEPTH_TEST);
      }
      m_renderState.depthTestEnabled = state->depthTestEnabled;
    }

    if (m_renderState.blendFunction != state->blendFunction)
    {
      switch (state->blendFunction)
      {
      case BlendFunction::NONE:
        glDisable(GL_BLEND);
        break;
      case BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
      }

      m_renderState.blendFunction = state->blendFunction;
    }

    if (state->diffuseTextureInUse)
    {
      m_renderState.diffuseTexture = state->diffuseTexture;
      GLint loc = glGetUniformLocation(program->m_handle, "s_texture");
      glUniform1i(loc, m_textureIdCount);
      glActiveTexture(GL_TEXTURE0 + m_textureIdCount);
      glBindTexture(GL_TEXTURE_2D, m_renderState.diffuseTexture);
      m_textureIdCount++;
    }

    if (state->cubeMapInUse)
    {
      m_renderState.cubeMap = state->cubeMap;
      GLint loc = glGetUniformLocation(program->m_handle, "cubeMap");
      glUniform1i(loc, m_textureIdCount);
      glActiveTexture(GL_TEXTURE0 + m_textureIdCount);
      glBindTexture(GL_TEXTURE_CUBE_MAP, m_renderState.cubeMap);
      m_textureIdCount++;
    }

    if (m_renderState.lineWidth != state->lineWidth)
    {
      m_renderState.lineWidth = state->lineWidth;
      glLineWidth(m_renderState.lineWidth);
    }
  }

  void Renderer::SetRenderTarget
  (
    RenderTarget* renderTarget,
    bool clear,
    const Vec4& color
  )
  {
    if (m_renderTarget == renderTarget && m_renderTarget != nullptr)
    {
      return;
    }

    if (renderTarget != nullptr)
    {
      glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->m_frameBufferId);
      glViewport(0, 0, renderTarget->m_width, renderTarget->m_height);

      if (glm::all(glm::epsilonNotEqual(color, m_bgColor, 0.001f)))
      {
        glClearColor(color.r, color.g, color.b, color.a);
      }

      if (clear)
      {
        glClear
        (
          GL_COLOR_BUFFER_BIT
          | GL_DEPTH_BUFFER_BIT
          | GL_STENCIL_BUFFER_BIT
        );
      }

      if (glm::all(glm::epsilonNotEqual(color, m_bgColor, 0.001f)))
      {
        glClearColor(m_bgColor.r, m_bgColor.g, m_bgColor.b, m_bgColor.a);
      }
    }
    else
    {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glViewport(0, 0, m_windowWidth, m_windowHeight);
    }

    m_renderTarget = renderTarget;
  }

  void Renderer::SwapRenderTarget
  (
    RenderTarget** renderTarget,
    bool clear,
    const Vec4& color
  )
  {
    RenderTarget* tmp = *renderTarget;
    *renderTarget = m_renderTarget;
    SetRenderTarget(tmp, clear, color);
  }

  void Renderer::SetViewport(Viewport* viewport)
  {
    m_windowWidth = static_cast<uint>(viewport->m_width);
    m_windowHeight = static_cast<uint>(viewport->m_height);
    SetRenderTarget(viewport->m_viewportImage);
  }

  void Renderer::SetViewportSize(uint width, uint height)
  {
    m_windowWidth = width;
    m_windowHeight = height;
    glViewport(0, 0, width, height);
  }

  void Renderer::DrawFullQuad(ShaderPtr fragmentShader)
  {
    static ShaderPtr fullQuadVert = GetShaderManager()->Create<Shader>
    (
      ShaderPath("fullQuadVert.shader", true)
    );
    static MaterialPtr material = std::make_shared<Material>();
    material->UnInit();

    material->m_vertexShader = fullQuadVert;
    material->m_fragmetShader = fragmentShader;
    material->Init();

    static Quad quad;
    quad.GetMeshComponent()->GetMeshVal()->m_material = material;

    static Camera quadCam;
    Render(&quad, &quadCam);
  }

  void Renderer::DrawCube(Camera* cam, MaterialPtr mat)
  {
    static Cube cube;
    cube.Generate();

    MaterialComponentPtr matc = cube.GetMaterialComponent();
    if (matc == nullptr)
    {
      cube.AddComponent(new MaterialComponent);
    }
    cube.GetMaterialComponent()->SetMaterialVal(mat);

    Render(&cube, cam);
  }

  void Renderer::RenderEntities
  (
    EntityRawPtrArray& entities,
    Camera* cam,
    Viewport* viewport,
    const LightRawPtrArray& lights
  )
  {
    GetEnvironmentLightEntities(entities);

    // Dropout non visible & drawable entities.
    entities.erase
    (
      std::remove_if
      (
        entities.begin(),
        entities.end(),
        [](Entity* ntt) -> bool
        {
          return !ntt->GetVisibleVal() || !ntt->IsDrawable();
        }
      ),
      entities.end()
    );

    FrustumCull(entities, cam);

    EntityRawPtrArray blendedEntities;
    GetTransparentEntites(entities, blendedEntities);

    RenderOpaque(entities, cam, viewport->m_zoom, lights);

    RenderTransparent(blendedEntities, cam, viewport->m_zoom, lights);
  }

  void Renderer::FrustumCull(EntityRawPtrArray& entities, Camera* camera)
  {
    // Frustum cull
    Mat4 pr = camera->GetProjectionMatrix();
    Mat4 v = camera->GetViewMatrix();
    Frustum frustum = ExtractFrustum(pr * v, false);

    auto delFn = [frustum](Entity* ntt) -> bool
    {
      IntersectResult res = FrustumBoxIntersection
      (
        frustum,
        ntt->GetAABB(true)
      );
      if (res == IntersectResult::Outside)
      {
        return true;
      }
      else
      {
        return false;
      }
    };
    entities.erase
    (
      std::remove_if(entities.begin(), entities.end(), delFn), entities.end()
    );
  }

  void Renderer::GetTransparentEntites
  (
    EntityRawPtrArray& entities,
    EntityRawPtrArray& blendedEntities
  )
  {
    auto delTrFn = [&blendedEntities](Entity* ntt) -> bool
    {
      // Check too see if there are any material with blend state.
      MaterialComponentPtrArray materials;
      ntt->GetComponent<MaterialComponent>(materials);

      if (!materials.empty())
      {
        for (MaterialComponentPtr& mt : materials)
        {
          if
          (
            mt->GetMaterialVal()
            && mt->GetMaterialVal()->GetRenderState()->blendFunction !=
            BlendFunction::NONE
          )
          {
            blendedEntities.push_back(ntt);
            return true;
          }
        }
      }
      else
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
          for (const Mesh* m : all)
          {
            if
            (
              m->m_material->GetRenderState()->blendFunction !=
              BlendFunction::NONE
            )
            {
              blendedEntities.push_back(ntt);
              return true;
            }
          }
        }
      }

      return false;
    };

    entities.erase
    (
      std::remove_if(entities.begin(), entities.end(), delTrFn), entities.end()
    );
  }

  void Renderer::RenderOpaque
  (
    EntityRawPtrArray entities,
    Camera* cam,
    float zoom,
    const LightRawPtrArray& editorLights
  )
  {
    // Render opaque objects
    for (Entity* ntt : entities)
    {
      if (ntt->GetType() == EntityType::Entity_Billboard)
      {
        Billboard* billboard = static_cast<Billboard*> (ntt);
        billboard->LookAt(cam, zoom);
      }

      FindEnvironmentLight(ntt, cam);

      Render(ntt, cam, editorLights);
    }
  }

  void Renderer::RenderTransparent
  (
    EntityRawPtrArray entities,
    Camera* cam,
    float zoom,
    const LightRawPtrArray& editorLights
  )
  {
    StableSortByDistanceToCamera(entities, cam);
    StableSortByMaterialPriority(entities);

    // Render transparent entities
    for (Entity* ntt : entities)
    {
      if (ntt->GetType() == EntityType::Entity_Billboard)
      {
        Billboard* billboard = static_cast<Billboard*> (ntt);
        billboard->LookAt(cam, zoom);
      }

      FindEnvironmentLight(ntt, cam);

      // For two sided materials,
      // first render back of transparent objects then render front
      MaterialPtr renderMaterial = GetRenderMaterial(ntt);
      if (renderMaterial->GetRenderState()->cullMode == CullingType::TwoSided)
      {
        renderMaterial->GetRenderState()->cullMode = CullingType::Front;
        Render(ntt, cam, editorLights);

        renderMaterial->GetRenderState()->cullMode = CullingType::Back;
        Render(ntt, cam, editorLights);

        renderMaterial->GetRenderState()->cullMode = CullingType::TwoSided;
      }
      else
      {
        Render(ntt, cam, editorLights);
      }
    }
  }

  void Renderer::RenderSky(Sky* sky, Camera* cam)
  {
    if (sky == nullptr || !sky->GetDrawSkyVal())
    {
      return;
    }

    glDepthFunc(GL_LEQUAL);

    DrawCube(cam, sky->GetSkyboxMaterial());

    glDepthFunc(GL_LESS);  // Return to default depth test
  }

  LightRawPtrArray Renderer::GetBestLights
  (
    Entity* entity,
    const LightRawPtrArray& lights
  )
  {
    LightRawPtrArray bestLights;
    LightRawPtrArray outsideRadiusLights;
    bestLights.reserve(lights.size());

    // Find the end of directional lights
    for (int i = 0; i < lights.size(); i++)
    {
      if(lights[i]->GetLightType() == LightTypeEnum::LightDirectional)
      {
        bestLights.push_back(lights[i]);
      }
    }

    // Add the lights inside of the radius first
    for (int i = 0; i < lights.size(); i++)
    {
      {
        float radius;
        if (lights[i]->GetLightType() == LightTypeEnum::LightPoint)
        {
          radius = static_cast<PointLight*>(lights[i])->GetRadiusVal();
        }
        else if (lights[i]->GetLightType() == LightTypeEnum::LightSpot)
        {
          radius = static_cast<SpotLight*>(lights[i])->GetRadiusVal();
        }
        else
        {
          continue;
        }

        float distance = glm::length2
        (
          entity->m_node->GetTranslation(TransformationSpace::TS_WORLD) -
          lights[i]->m_node->GetTranslation(TransformationSpace::TS_WORLD)
        );

        if (distance < radius * radius)
        {
          bestLights.push_back(lights[i]);
        }
        else
        {
          outsideRadiusLights.push_back(lights[i]);
        }
      }
    }
    bestLights.insert
    (
      bestLights.end(),
      outsideRadiusLights.begin(),
      outsideRadiusLights.end()
    );

    return bestLights;
  }

  void Renderer::GetEnvironmentLightEntities(EntityRawPtrArray entities)
  {
    // Find entities which have environment component
    m_environmentLightEntities.clear();
    for (Entity* ntt : entities)
    {
      if (ntt->GetType() == EntityType::Entity_Sky)
      {
        continue;
      }

      EnvironmentComponentPtr envCom =
      ntt->GetComponent<EnvironmentComponent>();
      if
      (
        envCom != nullptr
        && envCom->GetHdriVal() != nullptr
        && envCom->GetHdriVal()->IsTextureAssigned()
        && envCom->GetIlluminateVal()
      )
      {
        envCom->Init(true);
        m_environmentLightEntities.push_back(ntt);
      }
    }
  }

  void Renderer::FindEnvironmentLight
  (
    Entity* entity,
    Camera* camera
  )
  {
    if (camera->IsOrtographic())
    {
      return;
    }

    // Note: If multiple bounding boxes are intersecting and the intersection
    // volume includes the entity, the smaller bounding box is taken

    // Iterate all entities and mark the ones which should
    // be lit with environment light

    Entity* env = nullptr;

    MaterialPtr mat = GetRenderMaterial(entity);
    if (mat == nullptr)
    {
      return;
    }

    Vec3 pos = entity->m_node->GetTranslation(TransformationSpace::TS_WORLD);
    BoundingBox bestBox;
    bestBox.max = ZERO;
    bestBox.min = ZERO;
    BoundingBox currentBox;
    env = nullptr;
    for (Entity* envNtt : m_environmentLightEntities)
    {
      currentBox.max =
      envNtt->GetComponent<EnvironmentComponent>()->GetBBoxMax();
      currentBox.min =
      envNtt->GetComponent<EnvironmentComponent>()->GetBBoxMin();

      if (PointInsideBBox(pos, currentBox.max, currentBox.min))
      {
        auto setCurrentBBox =
        [&bestBox, &env]
        (
          const BoundingBox& box,
          Entity* ntt
        ) -> void
        {
          bestBox = box;
          env = ntt;
        };

        if (bestBox.max == bestBox.min && bestBox.max == ZERO)
        {
          setCurrentBBox(currentBox, envNtt);
          continue;
        }

        bool change = false;
        if (BoxBoxIntersection(bestBox, currentBox))
        {
          // Take the smaller box
          if (bestBox.Volume() > currentBox.Volume())
          {
            change = true;
          }
        }
        else
        {
          change = true;
        }

        if (change)
        {
          setCurrentBBox(currentBox, envNtt);
        }
      }
    }

    if (env != nullptr)
    {
      mat->GetRenderState()->IBLInUse = true;
      EnvironmentComponentPtr envCom =
      env->GetComponent<EnvironmentComponent>();
      mat->GetRenderState()->iblIntensity = envCom->GetIntensityVal();
      mat->GetRenderState()->irradianceMap =
      envCom->GetHdriVal()->GetIrradianceCubemapId();
    }
    else
    {
      // Sky light
      Sky* sky = GetSceneManager()->GetCurrentScene()->GetSky();
      if (sky != nullptr && sky->GetIlluminateVal())
      {
        mat->GetRenderState()->IBLInUse = true;
        EnvironmentComponentPtr envCom =
        sky->GetComponent<EnvironmentComponent>();
        mat->GetRenderState()->iblIntensity = envCom->GetIntensityVal();
        mat->GetRenderState()->irradianceMap =
        envCom->GetHdriVal()->GetIrradianceCubemapId();
      }
      else
      {
        mat->GetRenderState()->IBLInUse = false;
        mat->GetRenderState()->irradianceMap = 0;
      }
    }
  }

  void Renderer::SetProjectViewModel(Entity* ntt, Camera* cam)
  {
    m_view = cam->GetViewMatrix();
    m_project = cam->GetProjectionMatrix();
    m_model = ntt->m_node->GetTransform(TransformationSpace::TS_WORLD);
  }

  void Renderer::BindProgram(ProgramPtr program)
  {
    if (m_currentProgram == program->m_handle)
    {
      return;
    }

    m_currentProgram = program->m_handle;
    glUseProgram(program->m_handle);
  }

  void Renderer::LinkProgram(GLuint program, GLuint vertexP, GLuint fragmentP)
  {
    glAttachShader(program, vertexP);
    glAttachShader(program, fragmentP);

    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
      GLint infoLen = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
      if (infoLen > 1)
      {
        char* log = new char[infoLen];
        glGetProgramInfoLog(program, infoLen, nullptr, log);
        GetLogger()->Log(log);

        SafeDelArray(log);
      }
      assert(linked);

      glDeleteProgram(program);
    }
  }

  ProgramPtr Renderer::CreateProgram(ShaderPtr vertex, ShaderPtr fragment)
  {
    assert(vertex);
    assert(fragment);
    vertex->Init();
    fragment->Init();

    String tag;
    tag = vertex->m_tag + fragment->m_tag;
    if (m_programs.find(tag) == m_programs.end())
    {
      ProgramPtr program = std::make_shared<Program>(vertex, fragment);
      program->m_handle = glCreateProgram();
      LinkProgram
      (
        program->m_handle,
        vertex->m_shaderHandle,
        fragment->m_shaderHandle
      );
      m_programs[program->m_tag] = program;
    }

    return m_programs[tag];
  }

  void Renderer::FeedUniforms(ProgramPtr program)
  {
    for (ShaderPtr shader : program->m_shaders)
    {
      // Built-in variables.
      for (Uniform uni : shader->m_uniforms)
      {
        switch (uni)
        {
        case Uniform::PROJECT_MODEL_VIEW:
        {
          GLint loc = glGetUniformLocation
          (
            program->m_handle,
            "ProjectViewModel"
          );
          Mat4 mul = m_project * m_view * m_model;
          glUniformMatrix4fv(loc, 1, false, &mul[0][0]);
        }
        break;
        case Uniform::MODEL:
        {
          GLint loc = glGetUniformLocation(program->m_handle, "Model");
          glUniformMatrix4fv(loc, 1, false, &m_model[0][0]);
        }
        break;
        case Uniform::INV_TR_MODEL:
        {
          GLint loc = glGetUniformLocation
          (
            program->m_handle,
            "InverseTransModel"
          );
          Mat4 invTrModel = glm::transpose(glm::inverse(m_model));
          glUniformMatrix4fv(loc, 1, false, &invTrModel[0][0]);
        }
        break;
        case Uniform::LIGHT_DATA:
        {
          FeedLightUniforms(program);
        }
        break;
        case Uniform::CAM_DATA:
        {
          if (m_cam == nullptr)
            break;

          Camera::CamData data = m_cam->GetData();
          GLint loc = glGetUniformLocation(program->m_handle, "CamData.pos");
          glUniform3fv(loc, 1, &data.pos.x);
          loc = glGetUniformLocation(program->m_handle, "CamData.dir");
          glUniform3fv(loc, 1, &data.dir.x);
        }
        break;
        case Uniform::COLOR:
        {
          if (m_mat == nullptr)
            return;

          Vec4 color = Vec4(m_mat->m_color, m_mat->m_alpha);
          if
          (
            m_mat->GetRenderState()->blendFunction
            != BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA
          )
          {
            color.a = 1.0f;
          }

          GLint loc = glGetUniformLocation(program->m_handle, "Color");
          glUniform4fv(loc, 1, &color.x);
        }
        break;
        case Uniform::FRAME_COUNT:
        {
          GLint loc = glGetUniformLocation(program->m_handle, "FrameCount");
          glUniform1ui(loc, m_frameCount);
        }
        break;
        case Uniform::GRID_SETTINGS:
        {
          GLint locCellSize =
            glGetUniformLocation(program->m_handle, "GridData.cellSize");
          glUniform1fv(locCellSize, 1, &m_gridCellSize);
          GLint locGridSize =
            glGetUniformLocation(program->m_handle, "GridData.gridSize");
          glUniform1fv(locGridSize, 1, &m_gridSize);
          GLint locHorizontalAxisColor = glGetUniformLocation
          (
            program->m_handle,
            "GridData.horizontalAxisColor"
          );
          glUniform3fv(locHorizontalAxisColor, 1, &m_gridHorizontalAxisColor.x);
          GLint locVerticalAxisColor = glGetUniformLocation
          (
            program->m_handle,
            "GridData.verticalAxisColor"
          );
          glUniform3fv(locVerticalAxisColor, 1, &m_gridVerticalAxisColor.x);
        }
        break;
        case Uniform::EXPOSURE:
        {
          GLint loc = glGetUniformLocation(program->m_handle, "Exposure");
          glUniform1f(loc, shader->m_shaderParams["Exposure"].GetVar<float>());
        }
        break;
        case Uniform::PROJECTION_VIEW_NO_TR:
        {
          GLint loc = glGetUniformLocation
          (
            program->m_handle,
            "ProjectionViewNoTr"
          );
          // Zero transalate variables in model matrix
          m_view[0][3] = 0.0f;
          m_view[1][3] = 0.0f;
          m_view[2][3] = 0.0f;
          m_view[3][3] = 1.0f;
          m_view[3][0] = 0.0f;
          m_view[3][1] = 0.0f;
          m_view[3][2] = 0.0f;
          Mat4 mul = m_project * m_view;
          glUniformMatrix4fv(loc, 1, false, &mul[0][0]);
        }
        break;
        case Uniform::USE_IBL:
        {
          m_renderState.IBLInUse = m_mat->GetRenderState()->IBLInUse;
          GLint loc = glGetUniformLocation(program->m_handle, "UseIbl");
          glUniform1f(loc, static_cast<float>(m_renderState.IBLInUse));
        }
        break;
        case Uniform::IBL_INTENSITY:
        {
          m_renderState.iblIntensity = m_mat->GetRenderState()->iblIntensity;
          GLint loc = glGetUniformLocation(program->m_handle, "IblIntensity");
          glUniform1f(loc, static_cast<float>(m_renderState.iblIntensity));
        }
        break;
        case Uniform::IBL_IRRADIANCE:
        {
          m_renderState.irradianceMap = m_mat->GetRenderState()->irradianceMap;
          GLint loc = glGetUniformLocation
          (
            program->m_handle,
            "IBLIrradianceMap"
          );
          glUniform1i(loc, m_textureIdCount);
          glActiveTexture(GL_TEXTURE0 + m_textureIdCount);
          glBindTexture(GL_TEXTURE_CUBE_MAP, m_renderState.irradianceMap);
          m_textureIdCount++;
        }
        break;
        default:
          assert(false);
        break;
        }
      }

      // Custom variables.
      for (auto var : shader->m_shaderParams)
      {
        GLint loc = glGetUniformLocation(program->m_handle, var.first.c_str());
        if (loc == -1)
        {
          continue;
        }

        switch (var.second.GetType())
        {
        case ParameterVariant::VariantType::Float:
          glUniform1f(loc, var.second.GetVar<float>());
          break;
        case ParameterVariant::VariantType::Int:
          glUniform1i(loc, var.second.GetVar<int>());
          break;
        case ParameterVariant::VariantType::Vec3:
          glUniform3fv
          (
            loc,
            1,
            reinterpret_cast<float*>(&var.second.GetVar<Vec3>())
          );
          break;
        case ParameterVariant::VariantType::Vec4:
          glUniform4fv
          (
            loc,
            1,
            reinterpret_cast<float*>(&var.second.GetVar<Vec4>())
          );
          break;
        case ParameterVariant::VariantType::Mat3:
          glUniformMatrix3fv
          (
            loc,
            1,
            false,
            reinterpret_cast<float*>(&var.second.GetVar<Mat3>())
          );
          break;
        case ParameterVariant::VariantType::Mat4:
          glUniformMatrix4fv
          (
            loc,
            1,
            false,
            reinterpret_cast<float*>(&var.second.GetVar<Mat4>())
          );
          break;
        default:
          assert(false && "Invalid type.");
          break;
        }
      }
    }
  }

  void Renderer::FeedLightUniforms(ProgramPtr program)
  {
    size_t size = glm::min(m_lights.size(), m_maxLightsPerObject);
    for (size_t i = 0; i < size; i++)
    {
      Light* currLight = m_lights[i];

      LightTypeEnum type = currLight->GetLightType();

      // Point light uniforms
      if (type == LightTypeEnum::LightPoint)
      {
        Vec3 color = currLight->GetColorVal();
        float intensity = currLight->GetIntensityVal();
        Vec3 pos = currLight->m_node->GetTranslation
        (
          TransformationSpace::TS_WORLD
        );
        float radius = static_cast<PointLight*>(currLight)->GetRadiusVal();

        GLuint loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightTypeStrCache[i].c_str()
        );
        glUniform1i(loc, static_cast<GLuint>(type));
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightColorStrCache[i].c_str()
        );
        glUniform3fv(loc, 1, &color.x);
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightIntensityStrCache[i].c_str()
        );
        glUniform1f(loc, intensity);
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightPosStrCache[i].c_str()
        );
        glUniform3fv(loc, 1, &pos.x);
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightRadiusStrCache[i].c_str()
        );
        glUniform1f(loc, radius);
      }
      // Directional light uniforms
      else if (type == LightTypeEnum::LightDirectional)
      {
        Vec3 color = currLight->GetColorVal();
        float intensity = currLight->GetIntensityVal();
        Vec3 dir = static_cast<DirectionalLight*>(currLight)->
          GetComponent<DirectionComponent>()->GetDirection();

        GLuint loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightTypeStrCache[i].c_str()
        );
        glUniform1i(loc, static_cast<GLuint>(type));
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightColorStrCache[i].c_str()
        );
        glUniform3fv(loc, 1, &color.x);
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightIntensityStrCache[i].c_str()
        );
        glUniform1f(loc, intensity);
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightDirStrCache[i].c_str()
        );
        glUniform3fv(loc, 1, &dir.x);
      }
      // Spot light uniforms
      else if (type == LightTypeEnum::LightSpot)
      {
        Vec3 color = currLight->GetColorVal();
        float intensity = currLight->GetIntensityVal();
        Vec3 pos = currLight->m_node->GetTranslation
        (
          TransformationSpace::TS_WORLD
        );
        SpotLight* spotLight = static_cast<SpotLight*>(currLight);
        Vec3 dir =
        spotLight->GetComponent<DirectionComponent>()->GetDirection();
        float radius = spotLight->GetRadiusVal();
        float outAngle = glm::cos
        (
          glm::radians(spotLight->GetOuterAngleVal() / 2.0f)
        );
        float innAngle = glm::cos
        (
          glm::radians(spotLight->GetInnerAngleVal() / 2.0f)
        );

        GLuint loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightTypeStrCache[i].c_str()
        );
        glUniform1i(loc, static_cast<GLuint>(type));
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightColorStrCache[i].c_str()
        );
        glUniform3fv(loc, 1, &color.x);
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightIntensityStrCache[i].c_str()
        );
        glUniform1f(loc, intensity);
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightPosStrCache[i].c_str()
        );
        glUniform3fv(loc, 1, &pos.x);
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightDirStrCache[i].c_str()
        );
        glUniform3fv(loc, 1, &dir.x);
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightRadiusStrCache[i].c_str()
        );
        glUniform1f(loc, radius);
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightOuterAngleStrCache[i].c_str()
        );
        glUniform1f(loc, outAngle);
        loc = glGetUniformLocation
        (
          program->m_handle,
          g_lightInnerAngleStrCache[i].c_str()
        );
        glUniform1f(loc, innAngle);
      }
    }

    GLint loc = glGetUniformLocation
    (
      program->m_handle,
      "LightData.activeCount"
    );
    glUniform1i(loc, static_cast<int>(m_lights.size()));
  }

  void Renderer::SetVertexLayout(VertexLayout layout)
  {
    if (m_renderState.vertexLayout == layout)
    {
      return;
    }

    if (layout == VertexLayout::None)
    {
      for (int i = 0; i < 6; i++)
      {
        glDisableVertexAttribArray(i);
      }
    }

    if (layout == VertexLayout::Mesh)
    {
      GLuint offset = 0;
      glEnableVertexAttribArray(0);  // Vertex
      glVertexAttribPointer
      (
        0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0
      );
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(1);  // Normal
      glVertexAttribPointer
      (
        1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset)
      );
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(2);  // Texture
      glVertexAttribPointer
      (
        2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset)
      );
      offset += 2 * sizeof(float);

      glEnableVertexAttribArray(3);  // BiTangent
      glVertexAttribPointer
      (
        3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset)
      );
    }

    if (layout == VertexLayout::SkinMesh)
    {
      GLuint offset = 0;
      glEnableVertexAttribArray(0);  // Vertex
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), 0);
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(1);  // Normal
      glVertexAttribPointer
      (
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(SkinVertex),
        BUFFER_OFFSET(offset)
      );
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(2);  // Texture
      glVertexAttribPointer
      (
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(SkinVertex),
        BUFFER_OFFSET(offset)
      );
      offset += 2 * sizeof(float);

      glEnableVertexAttribArray(3);  // BiTangent
      glVertexAttribPointer
      (
        3,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(SkinVertex),
        BUFFER_OFFSET(offset)
      );
      offset += 3 * sizeof(uint);

      glEnableVertexAttribArray(4);  // Bones
      glVertexAttribPointer
      (
        4,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(SkinVertex),
        BUFFER_OFFSET(offset)
      );
      offset += 4 * sizeof(unsigned int);

      glEnableVertexAttribArray(5);  // Weights
      glVertexAttribPointer
      (
        5,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(SkinVertex),
        BUFFER_OFFSET(offset)
      );
    }
  }
}  // namespace ToolKit
