#include "Sky.h"

#include <memory>

#include "ToolKit.h"
#include "DirectionComponent.h"

namespace ToolKit
{
  Sky::Sky()
  {
    EnvironmentComponent* envComp = new EnvironmentComponent;
    envComp->SetHdriVal
    (
      GetTextureManager()->Create<Hdri>(TexturePath("defaultHDRI.hdr", true))
    );
    AddComponent(envComp);

    // Mesh Component
    MeshComponent* mc = new MeshComponent();
    AddComponent(mc);

    DrawSky_Define
    (
      true,
      "Sky",
      90,
      true,
      true
    );

    SetNameVal("Sky");
  }

  Sky::~Sky()
  {
  }

  EntityType Sky::GetType() const
  {
    return EntityType::Entity_Sky;
  }

  void Sky::Init()
  {
    if (m_initialized)
    {
      return;
    }

    // Creating an invisible mesh
    MeshComponentPtr mc = GetComponent<MeshComponent>();
    Cube cube(Vec3(1.0f));
    MeshPtr meshPtr = cube.GetMeshComponent()->GetMeshVal();
    mc->SetMeshVal(meshPtr);
    m_node->SetScale(Vec3(0.35f));
    SetVisibility(false, false);

    // Environment Component
    EnvironmentComponentPtr envComp = GetComponent<EnvironmentComponent>();
    envComp->m_entity = this;
    envComp->GetHdriVal()->m_initiated = false;
    envComp->Init(true);

    // Do not expose bounding box corners since
    // there is no limit for sky bounding box
    envComp->ParamMax().m_editable = false;
    envComp->ParamMin().m_editable = false;
    envComp->ParamMax().m_exposed = false;
    envComp->ParamMin().m_exposed = false;

    // Skybox material
    ShaderPtr vert = GetShaderManager()->Create<Shader>
    (
      ShaderPath("skyboxVert.shader", true)
    );
    ShaderPtr frag = GetShaderManager()->Create<Shader>
    (
      ShaderPath("skyboxFrag.shader", true)
    );
    m_skyboxMaterial = std::make_shared<Material>();
    m_skyboxMaterial->m_cubeMap = envComp->GetHdriVal()->m_cubemap;
    m_skyboxMaterial->m_vertexShader = vert;
    m_skyboxMaterial->m_fragmetShader = frag;
    m_skyboxMaterial->Init();

    m_initialized = true;
  }

  MaterialPtr Sky::GetSkyboxMaterial()
  {
    HdriPtr hdri = GetComponent<EnvironmentComponent>()->GetHdriVal();
    m_skyboxMaterial->m_cubeMap = hdri->m_cubemap;
    return m_skyboxMaterial;
  }

}  // namespace ToolKit
