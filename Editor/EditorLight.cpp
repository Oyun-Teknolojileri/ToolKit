
#include "EditorLight.h"

#include "App.h"
#include "Material.h"
#include "Texture.h"

#include <memory>
#include <string>

namespace ToolKit
{
  namespace Editor
  {

    void EnableLightGizmo(Light* light, bool enable)
    {
      switch (light->GetType())
      {
      case EntityType::Entity_DirectionalLight:
        static_cast<EditorDirectionalLight*>(light)->EnableGizmo(enable);
        break;
      case EntityType::Entity_PointLight:
        static_cast<EditorPointLight*>(light)->EnableGizmo(enable);
        break;
      case EntityType::Entity_SpotLight:
        static_cast<EditorSpotLight*>(light)->EnableGizmo(enable);
        break;
      case EntityType::Entity_Light:
      default:
        assert(false && "Invalid Light Type");
        break;
      }
    }

    EditorLightBase::EditorLightBase(Light* light) : m_light(light)
    {
      m_gizmoUpdateFn = [this](Value& oldVal, Value& newVal) -> void {
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
      m_gizmoMC                        = std::make_shared<MeshComponent>();
      m_gizmoMC->ParamMesh().m_exposed = false;
      m_gizmoMC->SetCastShadowVal(false);

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

    EditorDirectionalLight::EditorDirectionalLight() : EditorLightBase(this)
    {
      m_gizmo = new DirectionalLightGizmo(this);
    }

    EditorDirectionalLight::~EditorDirectionalLight()
    {
    }

    void EditorDirectionalLight::ParameterEventConstructor()
    {
      Light::ParameterEventConstructor();
    }

    Entity* EditorDirectionalLight::Copy() const
    {
      EditorDirectionalLight* cpy = new EditorDirectionalLight();
      WeakCopy(cpy, false);
      cpy->Init();
      return cpy;
    }

    void EditorDirectionalLight::Serialize(XmlDocument* doc,
                                           XmlNode* parent) const
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

    LineBatch* EditorDirectionalLight::GetDebugShadowFrustum()
    {
      Vec3Array corners = GetShadowFrustumCorners();
      static Vec3Array vertices;
      vertices.resize(24);
      vertices[0]  = corners[3];
      vertices[1]  = corners[2];
      vertices[2]  = corners[2];
      vertices[3]  = corners[1];
      vertices[4]  = corners[1];
      vertices[5]  = corners[0];
      vertices[6]  = corners[0];
      vertices[7]  = corners[3];
      vertices[8]  = corners[6];
      vertices[9]  = corners[5];
      vertices[10] = corners[5];
      vertices[11] = corners[4];
      vertices[12] = corners[4];
      vertices[13] = corners[7];
      vertices[14] = corners[7];
      vertices[15] = corners[6];
      vertices[16] = corners[6];
      vertices[17] = corners[2];
      vertices[18] = corners[5];
      vertices[19] = corners[1];
      vertices[20] = corners[4];
      vertices[21] = corners[0];
      vertices[22] = corners[7];
      vertices[23] = corners[3];

      LineBatch* lb =
          new LineBatch(vertices, Vec3(1.0f, 0.0f, 0.0f), DrawType::Line, 0.5f);

      return lb;
    }

    EditorPointLight::EditorPointLight() : EditorLightBase(this)
    {
      m_gizmo = new PointLightGizmo(this);
      ParameterEventConstructor();
    }

    EditorPointLight::~EditorPointLight()
    {
    }

    void EditorPointLight::ParameterEventConstructor()
    {
      Light::ParameterEventConstructor();
      ParamRadius().m_onValueChangedFn.clear();
      ParamRadius().m_onValueChangedFn.push_back(m_gizmoUpdateFn);
    }

    Entity* EditorPointLight::Copy() const
    {
      EditorPointLight* cpy = new EditorPointLight();
      WeakCopy(cpy, false);
      cpy->Init();
      return cpy;
    }

    void EditorPointLight::Serialize(XmlDocument* doc, XmlNode* parent) const
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

    EditorSpotLight::EditorSpotLight() : EditorLightBase(this)
    {
      m_gizmo = new SpotLightGizmo(this);
      ParameterEventConstructor();
    }

    EditorSpotLight::~EditorSpotLight()
    {
    }

    void EditorSpotLight::ParameterEventConstructor()
    {
      Light::ParameterEventConstructor();

      ParamRadius().m_onValueChangedFn.clear();
      ParamRadius().m_onValueChangedFn.push_back(m_gizmoUpdateFn);
      ParamOuterAngle().m_onValueChangedFn.clear();
      ParamOuterAngle().m_onValueChangedFn.push_back(m_gizmoUpdateFn);
      ParamInnerAngle().m_onValueChangedFn.clear();
      ParamInnerAngle().m_onValueChangedFn.push_back(m_gizmoUpdateFn);
    }

    Entity* EditorSpotLight::Copy() const
    {
      EditorSpotLight* cpy = new EditorSpotLight();
      WeakCopy(cpy, false);
      cpy->Init();
      return cpy;
    }

    void EditorSpotLight::Serialize(XmlDocument* doc, XmlNode* parent) const
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

  } // namespace Editor
} // namespace ToolKit
