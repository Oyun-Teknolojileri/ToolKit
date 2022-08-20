
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
      switch (light->GetType())
      {
        case EntityType::Entity_DirectionalLight:
          static_cast<EditorDirectionalLight*>(light)->EnableGizmo
          (
            light,
            enable
          );
        break;
        case EntityType::Entity_PointLight:
          static_cast<EditorPointLight*>(light)->EnableGizmo
          (
            light,
            enable
          );
        break;
        case EntityType::Entity_SpotLight:
          static_cast<EditorSpotLight*>(light)->EnableGizmo
          (
            light,
            enable
          );
        break;
        case EntityType::Entity_Light:
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
      SafeDel(m_gizmo);
    }

    void EditorLightBase::Init(Light* light)
    {
      if (m_initialized)
      {
        return;
      }

      // Mesh component for gizmo
      m_gizmoMC = std::make_shared<MeshComponent>();
      m_gizmoMC->ParamMesh().m_exposed = false;

      m_gizmo->InitGizmo(light);

      // Add gizmo meshes to mesh component
      for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
      {
        MeshPtr lbMesh = lb->GetComponent<MeshComponent>()->GetMeshVal();
        lbMesh->Init();
        m_gizmoMC->GetMeshVal()->m_subMeshes.push_back(lbMesh);
      }
      m_gizmoMC->ParamMesh().m_exposed = false;

      m_gizmoActive = false;
      m_initialized = true;
    }

    void EditorLightBase::EnableGizmo(Light* light, bool enable)
    {
      if (m_gizmoMC == nullptr)
      {
        return;
      }

      if (enable != m_gizmoActive)
      {
        if (enable)
        {
          light->AddComponent(m_gizmoMC);
          m_gizmoActive = true;
        }
        else
        {
          light->RemoveComponent(m_gizmoMC->m_id);
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

    void EditorDirectionalLight::ParameterEventConstructor()
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

    void EditorDirectionalLight::Serialize
    (
      XmlDocument* doc,
      XmlNode* parent
    ) const
    {
      m_gizmoMC->ParamMesh().m_exposed = false;
      Light::Serialize(doc, parent);
    }

    void EditorDirectionalLight::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      Light::DeSerialize(doc, parent);

      // Remove all mesh components
      MeshComponentPtrArray mcs;
      GetComponent<MeshComponent>(mcs);
      for (MeshComponentPtr mc : mcs)
      {
        RemoveComponent(mc->m_id);
      }

      ParameterEventConstructor();
    }

    void EditorDirectionalLight::Init()
    {
      EditorLightBase::Init(this);
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
        for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
        {
          MeshPtr lbMesh = lb->GetComponent<MeshComponent>()->GetMeshVal();
          lbMesh->Init();
          m_gizmoMC->GetMeshVal()->m_subMeshes.push_back(lbMesh);
        }
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

    void EditorPointLight::Serialize
    (
      XmlDocument* doc,
      XmlNode* parent
    ) const
    {
      m_gizmoMC->ParamMesh().m_exposed = false;
      Light::Serialize(doc, parent);
    }

    void EditorPointLight::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      Light::DeSerialize(doc, parent);

      // Remove all mesh components
      MeshComponentPtrArray mcs;
      GetComponent<MeshComponent>(mcs);
      for (MeshComponentPtr mc : mcs)
      {
        RemoveComponent(mc->m_id);
      }

      ParameterEventConstructor();
    }

    void EditorPointLight::Init()
    {
      EditorLightBase::Init(this);
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
        for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
        {
          MeshPtr lbMesh = lb->GetComponent<MeshComponent>()->GetMeshVal();
          lbMesh->Init();
          m_gizmoMC->GetMeshVal()->m_subMeshes.push_back(lbMesh);
        }
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

    void EditorSpotLight::Serialize
    (
      XmlDocument* doc,
      XmlNode* parent
    ) const
    {
      m_gizmoMC->ParamMesh().m_exposed = false;
      Light::Serialize(doc, parent);
    }

    void EditorSpotLight::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      Light::DeSerialize(doc, parent);

      // Remove all mesh components
      MeshComponentPtrArray mcs;
      GetComponent<MeshComponent>(mcs);
      for (MeshComponentPtr mc : mcs)
      {
        RemoveComponent(mc->m_id);
      }

      ParameterEventConstructor();
    }

    void EditorSpotLight::Init()
    {
      EditorLightBase::Init(this);
    }
  }  // namespace Editor
}  // namespace ToolKit
