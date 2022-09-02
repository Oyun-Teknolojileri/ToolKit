
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
            enable
          );
        break;
        case EntityType::Entity_PointLight:
          static_cast<EditorPointLight*>(light)->EnableGizmo
          (
            enable
          );
        break;
        case EntityType::Entity_SpotLight:
          static_cast<EditorSpotLight*>(light)->EnableGizmo
          (
            enable
          );
        break;
        case EntityType::Entity_Light:
        default:
          assert(false && "Invalid Light Type");
        break;
      }
    }

    EditorLightBase::EditorLightBase(Light* light)
      : m_light(light)
    {
      m_gizmoUpdateFn =
      [this](Value& oldVal, Value& newVal) -> void
      {
        m_gizmo->InitGizmo(m_light);
      };
    }

    EditorLightBase::~EditorLightBase()
    {
      SafeDel(m_gizmo);
    }

    void EditorLightBase::Init()
    {
      if (m_initialized)
      {
        return;
      }

      // Mesh component for gizmo
      m_gizmoMC = std::make_shared<MeshComponent>();
      m_gizmoMC->ParamMesh().m_exposed = false;

      m_gizmo->InitGizmo(m_light);
      m_gizmoMC->ParamMesh().m_exposed = false;

      m_gizmoActive = false;
      m_initialized = true;
    }

    void EditorLightBase::EnableGizmo(bool enable)
    {
      if (m_gizmoMC == nullptr || m_light == nullptr)
      {
        return;
      }

      if (enable != m_gizmoActive)
      {
        if (enable)
        {
          m_light->AddComponent(m_gizmoMC);
          m_gizmoActive = true;
        }
        else
        {
          m_light->RemoveComponent(m_gizmoMC->m_id);
          m_gizmoActive = false;
        }
      }
    }

    EditorDirectionalLight::EditorDirectionalLight()
      : EditorLightBase(this)
    {
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

    EditorPointLight::EditorPointLight()
      : EditorLightBase(this)
    {
      m_gizmo = new PointLightGizmo(this);
      ParameterEventConstructor();
    }

    EditorPointLight::~EditorPointLight()
    {
    }

    void EditorPointLight::ParameterEventConstructor()
    {
      ParamRadius().m_onValueChangedFn = m_gizmoUpdateFn;
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

    EditorSpotLight::EditorSpotLight()
      : EditorLightBase(this)
    {
      m_gizmo = new SpotLightGizmo(this);
      ParameterEventConstructor();
    }

    EditorSpotLight::~EditorSpotLight()
    {
    }

    void EditorSpotLight::ParameterEventConstructor()
    {
      ParamRadius().m_onValueChangedFn = m_gizmoUpdateFn;
      ParamOuterAngle().m_onValueChangedFn = m_gizmoUpdateFn;
      ParamInnerAngle().m_onValueChangedFn = m_gizmoUpdateFn;
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

  }  // namespace Editor
}  // namespace ToolKit
