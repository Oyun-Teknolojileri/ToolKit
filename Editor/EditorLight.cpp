/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "EditorLight.h"

#include "App.h"
#include "LightMeshGenerator.h"

#include <DirectionComponent.h>
#include <Light.h>
#include <Material.h>
#include <Texture.h>

#include <DebugNew.h>

namespace ToolKit
{
  namespace Editor
  {

    void EnableLightGizmo(Light* light, bool enable)
    {
      switch (light->GetLightType())
      {
      case Light::Directional:
        static_cast<EditorDirectionalLight*>(light)->EnableGizmo(enable);
        break;
      case Light::Point:
        static_cast<EditorPointLight*>(light)->EnableGizmo(enable);
        break;
      case Light::Spot:
        static_cast<EditorSpotLight*>(light)->EnableGizmo(enable);
        break;
      default:
        assert(false && "Invalid Light Type");
        break;
      }
    }

    ThreePointLightSystem::ThreePointLightSystem()
    {
      m_parentNode    = new Node();

      float intensity = 1.5f;
      LightPtr light  = MakeNewPtr<DirectionalLight>();
      light->SetColorVal(Vec3(0.55f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(-20.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(-20.0f));
      light->SetCastShadowVal(false);
      m_parentNode->AddChild(light->m_node);
      m_lights.push_back(light);

      light = MakeNewPtr<DirectionalLight>();
      light->SetColorVal(Vec3(0.15f));
      light->SetIntensityVal(intensity);
      light->GetComponent<DirectionComponent>()->Yaw(glm::radians(90.0f));
      light->GetComponent<DirectionComponent>()->Pitch(glm::radians(-45.0f));
      light->SetCastShadowVal(false);
      m_parentNode->AddChild(light->m_node);
      m_lights.push_back(light);

      light = MakeNewPtr<DirectionalLight>();
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
      m_lights.clear();
      SafeDel(m_parentNode);
    }

    LightGizmoController::LightGizmoController(Light* light) : m_light(light)
    {
      switch (light->GetLightType())
      {
      case Light::Directional:
        m_gizmoGenerator = new DirectionalLightMeshGenerator(static_cast<DirectionalLight*>(light));
        break;
      case Light::Point:
        m_gizmoGenerator = new PointLightMeshGenerator(static_cast<PointLight*>(light));
        break;
      case Light::Spot:
        m_gizmoGenerator = new SpotLightMeshGenerator(static_cast<SpotLight*>(light));
        break;
      default:
        assert(false && "Invalid Light Type");
        break;
      }

      m_gizmoUpdateFn = [this](Value& oldVal, Value& newVal) -> void { m_gizmoGenerator->InitGizmo(); };
    }

    LightGizmoController::~LightGizmoController() { SafeDel(m_gizmoGenerator); }

    bool LightGizmoController::GizmoActive() const { return m_gizmoActive; }

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
            m_light->RemoveComponent(mc->Class());
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

    void LightGizmoController::InitController()
    {
      if (m_initialized)
      {
        return;
      }

      m_gizmoGenerator->InitGizmo();

      m_gizmoActive = false;
      m_initialized = true;
    }

    TKDefineClass(EditorDirectionalLight, DirectionalLight);

    EditorDirectionalLight::EditorDirectionalLight() : LightGizmoController(this) {}

    EditorDirectionalLight::~EditorDirectionalLight() {}

    ObjectPtr EditorDirectionalLight::Copy() const
    {
      EditorDirectionalLightPtr cpy = MakeNewPtr<EditorDirectionalLight>();
      WeakCopy(cpy.get(), false);
      cpy->InitController();
      return cpy;
    }

    XmlNode* EditorDirectionalLight::SerializeImp(XmlDocument* doc, XmlNode* parent) const
    {
      XmlNode* lightNode = nullptr;
      if (m_gizmoActive)
      {
        EnableGizmo(false);
        lightNode = Super::SerializeImp(doc, parent);
        EnableGizmo(true);
      }
      else
      {
        lightNode = Super::SerializeImp(doc, parent);
      }

      return lightNode;
    }

    XmlNode* EditorDirectionalLight::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
    {
      XmlNode* dirLightNode = Super::DeSerializeImp(info, parent);
      assert(m_light->GetMeshComponent() == nullptr && "MeshComponents should not be serialized.");

      return dirLightNode;
    }

    LineBatchPtr EditorDirectionalLight::GetDebugShadowFrustum()
    {
      Vec3Array corners = m_shadowCamera->ExtractFrustumCorner();
      Vec3Array vertices;

      vertices.resize(24);
      vertices[0]     = corners[3];
      vertices[1]     = corners[2];
      vertices[2]     = corners[2];
      vertices[3]     = corners[1];
      vertices[4]     = corners[1];
      vertices[5]     = corners[0];
      vertices[6]     = corners[0];
      vertices[7]     = corners[3];
      vertices[8]     = corners[6];
      vertices[9]     = corners[5];
      vertices[10]    = corners[5];
      vertices[11]    = corners[4];
      vertices[12]    = corners[4];
      vertices[13]    = corners[7];
      vertices[14]    = corners[7];
      vertices[15]    = corners[6];
      vertices[16]    = corners[6];
      vertices[17]    = corners[2];
      vertices[18]    = corners[5];
      vertices[19]    = corners[1];
      vertices[20]    = corners[4];
      vertices[21]    = corners[0];
      vertices[22]    = corners[7];
      vertices[23]    = corners[3];

      LineBatchPtr lb = MakeNewPtr<LineBatch>();
      lb->Generate(vertices, Vec3(1.0f, 0.0f, 0.0f), DrawType::Line, 0.5f);

      return lb;
    }

    TKDefineClass(EditorPointLight, PointLight);

    EditorPointLight::EditorPointLight() : LightGizmoController(this) {}

    EditorPointLight::~EditorPointLight() {}

    void EditorPointLight::ParameterEventConstructor()
    {
      Super::ParameterEventConstructor();

      // ParamRadius().m_onValueChangedFn.clear();
      ParamRadius().m_onValueChangedFn.push_back(m_gizmoUpdateFn);
    }

    ObjectPtr EditorPointLight::Copy() const
    {
      EditorPointLightPtr cpy = MakeNewPtr<EditorPointLight>();
      WeakCopy(cpy.get(), false);
      cpy->InitController();
      return cpy;
    }

    XmlNode* EditorPointLight::SerializeImp(XmlDocument* doc, XmlNode* parent) const
    {
      XmlNode* lightNode = nullptr;
      if (m_gizmoActive)
      {
        EnableGizmo(false);
        lightNode = PointLight::SerializeImp(doc, parent);
        EnableGizmo(true);
      }
      else
      {
        lightNode = PointLight::SerializeImp(doc, parent);
      }

      return lightNode;
    }

    XmlNode* EditorPointLight::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
    {
      XmlNode* pointLightNode = Super::DeSerializeImp(info, parent);
      assert(m_light->GetMeshComponent() == nullptr && "MeshComponents should not be serialized.");

      return pointLightNode;
    }

    TKDefineClass(EditorSpotLight, SpotLight);

    EditorSpotLight::EditorSpotLight() : LightGizmoController(this) {}

    EditorSpotLight::~EditorSpotLight() {}

    void EditorSpotLight::ParameterEventConstructor()
    {
      Super::ParameterEventConstructor();

      ParamRadius().m_onValueChangedFn.push_back(m_gizmoUpdateFn);
      ParamOuterAngle().m_onValueChangedFn.push_back(m_gizmoUpdateFn);
      ParamInnerAngle().m_onValueChangedFn.push_back(m_gizmoUpdateFn);
    }

    ObjectPtr EditorSpotLight::Copy() const
    {
      EditorSpotLightPtr cpy = MakeNewPtr<EditorSpotLight>();
      WeakCopy(cpy.get(), false);
      cpy->InitController();
      return cpy;
    }

    XmlNode* EditorSpotLight::SerializeImp(XmlDocument* doc, XmlNode* parent) const
    {
      XmlNode* lightNode = nullptr;
      if (m_gizmoActive)
      {
        EnableGizmo(false);
        lightNode = SpotLight::SerializeImp(doc, parent);
        EnableGizmo(true);
      }
      else
      {
        lightNode = SpotLight::SerializeImp(doc, parent);
      }

      return lightNode;
    }

    XmlNode* EditorSpotLight::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
    {
      XmlNode* spotLightNode = Super::DeSerializeImp(info, parent);
      assert(m_light->GetMeshComponent() == nullptr && "MeshComponents should not be serialized.");

      return spotLightNode;
    }

  } // namespace Editor
} // namespace ToolKit
