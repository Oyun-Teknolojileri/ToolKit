
#include "EditorLight.h"

#include "App.h"
#include "DirectionComponent.h"
#include "Material.h"
#include "Texture.h"

#include <memory>
#include <string>

#include "DebugNew.h"

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

    ThreePointLightSystem::ThreePointLightSystem()
    {
      m_parentNode            = new Node();

      float intensity         = 1.5f;
      DirectionalLight* light = new DirectionalLight();
      light->SetColorVal(Vec3(0.55f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(-20.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(-20.0f));
      light->SetCastShadowVal(false);
      m_parentNode->AddChild(light->m_node);
      m_lights.push_back(light);

      light = new DirectionalLight();
      light->SetColorVal(Vec3(0.15f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(90.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(-45.0f));
      light->SetCastShadowVal(false);
      m_parentNode->AddChild(light->m_node);
      m_lights.push_back(light);

      light = new DirectionalLight();
      light->SetColorVal(Vec3(0.1f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(120.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(60.0f));
      light->SetCastShadowVal(false);
      m_parentNode->AddChild(light->m_node);
      m_lights.push_back(light);
    }

    ThreePointLightSystem::~ThreePointLightSystem()
    {
      for (Light* light : m_lights)
      {
        SafeDel(light);
      }

      m_lights.clear();
      SafeDel(m_parentNode);
    }

    LightGizmoController::LightGizmoController(Light* light) : m_light(light)
    {
      switch (light->GetType())
      {
      case EntityType::Entity_DirectionalLight:
        m_gizmoGenerator = new DirectionalLightMeshGenerator(
            static_cast<DirectionalLight*>(light));
        break;
      case EntityType::Entity_PointLight:
        m_gizmoGenerator =
            new PointLightMeshGenerator(static_cast<PointLight*>(light));
        break;
      case EntityType::Entity_SpotLight:
        m_gizmoGenerator =
            new SpotLightMeshGenerator(static_cast<SpotLight*>(light));
        break;
      case EntityType::Entity_Light:
      default:
        assert(false && "Invalid Light Type");
        break;
      }

      m_gizmoUpdateFn = [this](Value& oldVal, Value& newVal) -> void
      { m_gizmoGenerator->InitGizmo(); };
    }

    LightGizmoController::~LightGizmoController() { SafeDel(m_gizmoGenerator); }

    void LightGizmoController::EnableGizmo(bool enable) const
    {
      if (m_gizmoGenerator == nullptr)
      {
        assert(false && "Gizmo generator must be created.");
        return;
      }

      if (enable != m_gizmoActive)
      {
        auto removeMeshCmpFn = [this]() -> void
        {
          if (MeshComponentPtr mc = m_light->GetMeshComponent())
          {
            m_light->RemoveComponent(mc->m_id);
          }
        };

        if (enable)
        {
          removeMeshCmpFn();
          m_light->AddComponent(m_gizmoGenerator->m_lightMesh);
          m_gizmoActive = true;
        }
        else
        {
          removeMeshCmpFn();
          m_gizmoActive = false;
        }
      }
    }

    void LightGizmoController::Init()
    {
      if (m_initialized)
      {
        return;
      }

      m_gizmoGenerator->InitGizmo();

      m_gizmoActive = false;
      m_initialized = true;
    }

    EditorDirectionalLight::EditorDirectionalLight()
        : LightGizmoController(this)
    {
    }

    EditorDirectionalLight::~EditorDirectionalLight() {}

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
      if (m_gizmoActive)
      {
        EnableGizmo(false);
        DirectionalLight::Serialize(doc, parent);
        EnableGizmo(true);
      }
      else
      {
        DirectionalLight::Serialize(doc, parent);
      }
    }

    void EditorDirectionalLight::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      DirectionalLight::DeSerialize(doc, parent);

      assert(m_light->GetMeshComponent() == nullptr &&
             "MeshComponents should not be serialized.");

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

    EditorPointLight::EditorPointLight() : LightGizmoController(this)
    {
      ParameterEventConstructor();
    }

    EditorPointLight::~EditorPointLight() {}

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
      if (m_gizmoActive)
      {
        EnableGizmo(false);
        PointLight::Serialize(doc, parent);
        EnableGizmo(true);
      }
      else
      {
        PointLight::Serialize(doc, parent);
      }
    }

    void EditorPointLight::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      PointLight::DeSerialize(doc, parent);

      assert(m_light->GetMeshComponent() == nullptr &&
             "MeshComponents should not be serialized.");

      ParameterEventConstructor();
    }

    EditorSpotLight::EditorSpotLight() : LightGizmoController(this)
    {
      ParameterEventConstructor();
    }

    EditorSpotLight::~EditorSpotLight() {}

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
      if (m_gizmoActive)
      {
        EnableGizmo(false);
        SpotLight::Serialize(doc, parent);
        EnableGizmo(true);
      }
      else
      {
        SpotLight::Serialize(doc, parent);
      }
    }

    void EditorSpotLight::DeSerialize(XmlDocument* doc, XmlNode* parent)
    {
      SpotLight::DeSerialize(doc, parent);

      assert(m_light->GetMeshComponent() == nullptr &&
             "MeshComponents should not be serialized.");

      ParameterEventConstructor();
    }

  } // namespace Editor
} // namespace ToolKit
