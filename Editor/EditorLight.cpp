
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
      if (m_initialized)
      {
        m_initialized = false;
        SafeDel(m_gizmo);
      }
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

    bool EditorDirectionalLight::IsDrawable() const
    {
      return true;
    }

    void EditorDirectionalLight::Init()
    {
      if (m_initialized)
      {
        return;
      }

      m_initialized = true;

      // Light sphere
      std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(0.1f);

      MeshComponent* mc = new MeshComponent();
      mc->SetMeshVal
      (
        sphere->GetMeshComponent()->GetMeshVal()
      );

      GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      mc->GetMeshVal()->CalculateAABB();
      AddComponent(mc);

      m_gizmo->InitGizmo(this);

      // Directional light gizmo
      for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
      {
        MeshPtr mesh = lb->GetMeshComponent()->GetMeshVal();
        mesh->Init();
        mc->GetMeshVal()->m_subMeshes.push_back(mesh);
      }

      m_gizmoActive = true;
    }

    void EditorDirectionalLight::EnableGizmo(bool enable)
    {
      if (enable != m_gizmoActive)
      {
        if (enable)
        {
          // Add submeshes to mesh component
          MeshComponentPtr mc = GetComponent<MeshComponent>();

          for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
          {
            MeshPtr lbMesh = lb->GetComponent<MeshComponent>()->GetMeshVal();
            mc->GetMeshVal()->m_subMeshes.push_back(lbMesh);
          }

          m_gizmoActive = true;
        }
        else
        {
          // Remove submeshes from mesh component
          MeshComponentPtr mc = GetComponent<MeshComponent>();
          mc->GetMeshVal()->m_subMeshes.clear();

          m_gizmoActive = false;
        }
      }
    }

    EditorPointLight::EditorPointLight()
    {
      m_gizmo = new PointLightGizmo(this);
    }

    EditorPointLight::EditorPointLight(const EditorPointLight* light)
    {
      light->CopyTo(this);
      m_gizmo = new PointLightGizmo(this);
    }

    EditorPointLight::~EditorPointLight()
    {
      if (m_initialized)
      {
        m_initialized = false;
        SafeDel(m_gizmo);
      }
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

    bool EditorPointLight::IsDrawable() const
    {
      return true;
    }

    void EditorPointLight::Init()
    {
      if (m_initialized)
      {
        return;
      }

      m_initialized = true;

      // Light sphere
      std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(0.1f);

      MeshComponent* mc = new MeshComponent();
      mc->SetMeshVal
      (
        sphere->GetMeshComponent()->GetMeshVal()
      );

      MeshPtr compMesh = mc->GetMeshVal();
      compMesh->m_material = GetMaterialManager()->
        GetCopyOfUnlitColorMaterial();
      compMesh->CalculateAABB();
      AddComponent(mc);

      m_gizmo->InitGizmo(this);

      // Gizmo
      for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
      {
        MeshPtr mesh = lb->GetMeshComponent()->GetMeshVal();
        mesh->Init();
        compMesh->m_subMeshes.push_back(mesh);
      }

      m_gizmoActive = true;
    }

    void EditorPointLight::EnableGizmo(bool enable)
    {
      if (enable != m_gizmoActive)
      {
        if (enable)
        {
          // Add submeshes to mesh component
          MeshComponentPtr mc = GetComponent<MeshComponent>();

          for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
          {
            mc->GetMeshVal()->m_subMeshes.push_back
            (
              lb->GetMeshComponent()->GetMeshVal()
            );
          }

          m_gizmoActive = true;
        }
        else
        {
          // Remove submeshes from mesh component
          MeshComponentPtr mc = GetComponent<MeshComponent>();
          mc->GetMeshVal()->m_subMeshes.clear();

          m_gizmoActive = false;
        }
      }
    }

    EditorSpotLight::EditorSpotLight()
    {
      m_gizmo = new SpotLightGizmo(this);
    }

    EditorSpotLight::EditorSpotLight(const EditorSpotLight* light)
    {
      light->CopyTo(this);
      m_gizmo = new SpotLightGizmo(this);
    }

    EditorSpotLight::~EditorSpotLight()
    {
      if (m_initialized)
      {
        m_initialized = false;
        SafeDel(m_gizmo);
      }
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

    bool EditorSpotLight::IsDrawable() const
    {
      return true;
    }

    void EditorSpotLight::Init()
    {
      if (m_initialized)
      {
        return;
      }

      m_initialized = true;
      m_gizmo->InitGizmo(this);

      // Light sphere
      std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(0.1f);

      MeshComponent* mc = new MeshComponent();
      mc->SetMeshVal
      (
        sphere->GetMeshComponent()->GetMeshVal()
      );

      MeshPtr compMesh = mc->GetMeshVal();
      compMesh->m_material = GetMaterialManager()->
        GetCopyOfUnlitColorMaterial();
      compMesh->CalculateAABB();
      AddComponent(mc);

      // Gizmo
      for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
      {
        MeshPtr mesh = lb->GetComponent<MeshComponent>()->GetMeshVal();
        mesh->Init();
        compMesh->m_subMeshes.push_back(mesh);
      }

      m_gizmoActive = true;
    }

    void EditorSpotLight::EnableGizmo(bool enable)
    {
      if (enable != m_gizmoActive)
      {
        if (enable)
        {
          // Add submeshes to mesh component
          MeshComponentPtr mc = GetComponent<MeshComponent>();
          for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
          {
            MeshPtr lbMesh = lb->GetComponent<MeshComponent>()->GetMeshVal();
            mc->GetMeshVal()->m_subMeshes.push_back(lbMesh);
          }

          m_gizmoActive = true;
        }
        else
        {
          // Remove submeshes from mesh component
          MeshComponentPtr mc = GetComponent<MeshComponent>();
          mc->GetMeshVal()->m_subMeshes.clear();

          m_gizmoActive = false;
        }
      }
    }

  }  // namespace Editor
}  // namespace ToolKit

