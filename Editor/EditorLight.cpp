
#include "EditorLight.h"

#include <memory>
#include <string>

#include "Material.h"
#include "Texture.h"
#include "App.h"


namespace ToolKit
{
  namespace Editor
  {

    void EnableLightGizmo(Light* light, bool enable)
    {
      switch (light->GetLightType())
      {
        case LightTypeEnum::LightDirectional:
          static_cast<EditorDirectionalLight*> (light)->EnableGizmo(enable);
        break;
        case LightTypeEnum::LightPoint:
          static_cast<EditorPointLight*> (light)->EnableGizmo(enable);
        break;
        case LightTypeEnum::LightSpot:
          static_cast<EditorSpotLight*> (light)->EnableGizmo(enable);
        break;
        case LightTypeEnum::LightBase:
        default:
          assert(false && "Invalid Light Type");
        break;
      }
    }

    EditorLightBase::EditorLightBase()
    {
    }

    EditorLightBase::~EditorLightBase()
    {
      if (m_initialized)
      {
        m_initialized = false;
        SafeDel(m_gizmo);
      }
    }

    void EditorLightBase::EnableGizmo(bool enable)
    {
      if (m_gizmoMC == nullptr)
      {
        return;
      }

      if (enable != m_gizmoActive)
      {
        if (enable)
        {
          // Add submeshes to mesh component
          for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
          {
            MeshPtr lbMesh = lb->GetComponent<MeshComponent>()->GetMeshVal();
            lbMesh->Init();
            m_gizmoMC->GetMeshVal()->m_subMeshes.push_back(lbMesh);
          }

          m_gizmoActive = true;
        }
        else
        {
          // Remove submeshes from mesh component
          m_gizmoMC->GetMeshVal()->m_subMeshes.clear();

          m_gizmoActive = false;
        }
      }
    }

    EditorDirectionalLight::EditorDirectionalLight()
    {
      m_gizmo = new DirectionalLightGizmo(this);
    }

    EditorDirectionalLight::EditorDirectionalLight
    (
      const EditorDirectionalLight* light
    )
    {
      light->CopyTo(this);
      m_gizmo = new DirectionalLightGizmo(this);
    }

    EditorDirectionalLight::~EditorDirectionalLight()
    {
    }

    Entity* EditorDirectionalLight::Copy() const
    {
      EditorDirectionalLight* cpy = new EditorDirectionalLight();
      WeakCopy(cpy, false);
      cpy->Init();
      return cpy;
    }

    Entity* EditorDirectionalLight::Instantiate() const
    {
      EditorDirectionalLight* instance = new EditorDirectionalLight();
      WeakCopy(instance, false);
      instance->Init();
      return instance;
    }

    void EditorDirectionalLight::Init()
    {
      if (m_initialized)
      {
        return;
      }

      // Light sphere
      std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(0.1f);

      MeshComponent* mc = new MeshComponent();
      mc->SetMeshVal
      (
        sphere->GetMeshComponent()->GetMeshVal()
      );

      mc->GetMeshVal()->m_material =
        GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      mc->GetMeshVal()->CalculateAABB();
      AddComponent(mc);

      m_gizmo->InitGizmo(this);

      // Directional light gizmo
      for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
      {
        MeshPtr mesh = lb->GetMeshComponent()->GetMeshVal();
        mesh->Init();
      }

      m_gizmoMC = GetMeshComponent();
      m_gizmoActive = false;
      m_initialized = true;
    }

    EditorPointLight::EditorPointLight()
    {
      m_gizmo = new PointLightGizmo(this);
      ParameterEventConstructor();
    }

    EditorPointLight::EditorPointLight(const EditorPointLight* light)
    {
      light->CopyTo(this);
      m_gizmo = new PointLightGizmo(this);
      ParameterEventConstructor();
    }

    EditorPointLight::~EditorPointLight()
    {
    }

    void EditorPointLight::ParameterEventConstructor()
    {
      ParamRadius().m_onValueChangedFn =
      [this](Value& oldVal, Value& newVal) -> void
      {
        m_gizmo->InitGizmo(this);
      };
    }

    Entity* EditorPointLight::Copy() const
    {
      EditorPointLight* cpy = new EditorPointLight();
      WeakCopy(cpy, false);
      cpy->Init();
      return cpy;
    }

    Entity* EditorPointLight::Instantiate() const
    {
      EditorPointLight* instance = new EditorPointLight();
      WeakCopy(instance, false);
      instance->Init();
      return instance;
    }

    void EditorPointLight::Init()
    {
      if (m_initialized)
      {
        return;
      }

      // Light sphere
      std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(0.1f);

      MeshComponent* mc = new MeshComponent();
      mc->SetMeshVal
      (
        sphere->GetMeshComponent()->GetMeshVal()
      );

      MeshPtr compMesh = mc->GetMeshVal();
      compMesh->m_material =
        GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      compMesh->CalculateAABB();
      AddComponent(mc);

      m_gizmo->InitGizmo(this);

      // Gizmo
      for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
      {
        MeshPtr mesh = lb->GetMeshComponent()->GetMeshVal();
        mesh->Init();
      }

      m_gizmoMC = GetMeshComponent();
      m_gizmoActive = false;
      m_initialized = true;
    }

    EditorSpotLight::EditorSpotLight()
    {
      m_gizmo = new SpotLightGizmo(this);
      ParameterEventConstructor();
    }

    EditorSpotLight::EditorSpotLight(const EditorSpotLight* light)
    {
      light->CopyTo(this);
      m_gizmo = new SpotLightGizmo(this);
      ParameterEventConstructor();
    }

    EditorSpotLight::~EditorSpotLight()
    {
    }

    void EditorSpotLight::ParameterEventConstructor()
    {
      auto GizmoUpdateFn =
      [this](Value& oldVal, Value& newVal) -> void
      {
        m_gizmo->InitGizmo(this);
      };

      ParamRadius().m_onValueChangedFn = GizmoUpdateFn;

      ParamOuterAngle().m_onValueChangedFn = GizmoUpdateFn;

      ParamInnerAngle().m_onValueChangedFn = GizmoUpdateFn;
    }

    Entity* EditorSpotLight::Copy() const
    {
      EditorSpotLight* cpy = new EditorSpotLight();
      WeakCopy(cpy, false);
      cpy->Init();
      return cpy;
    }

    Entity* EditorSpotLight::Instantiate() const
    {
      EditorSpotLight* instance = new EditorSpotLight();
      WeakCopy(instance, false);
      instance->Init();
      return instance;
    }

    void EditorSpotLight::Init()
    {
      if (m_initialized)
      {
        return;
      }

      m_gizmo->InitGizmo(this);

      // Light sphere
      std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(0.1f);

      MeshComponent* mc = new MeshComponent();
      mc->SetMeshVal
      (
        sphere->GetMeshComponent()->GetMeshVal()
      );

      MeshPtr compMesh = mc->GetMeshVal();
      compMesh->m_material =
        GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      compMesh->CalculateAABB();
      AddComponent(mc);

      // Gizmo
      for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
      {
        MeshPtr mesh = lb->GetComponent<MeshComponent>()->GetMeshVal();
        mesh->Init();
      }

      m_gizmoMC = GetMeshComponent();
      m_gizmoActive = false;
      m_initialized = true;
    }
  }  // namespace Editor
}  // namespace ToolKit
