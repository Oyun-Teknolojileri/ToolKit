
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
    }

    EditorDirectionalLight::EditorDirectionalLight
    (
      const EditorDirectionalLight* light
    )
    {
      light->CopyTo(this);

      AddComponent(new DirectionalComponent(this));
    }

    EditorDirectionalLight::~EditorDirectionalLight()
    {
      if (m_initialized)
      {
        m_initialized = false;

        SafeDel(m_gizmo);
      }
    }

    Entity* ToolKit::Editor::EditorDirectionalLight::Copy() const
    {
      EditorDirectionalLight* cpy = new EditorDirectionalLight();
      return CopyTo(cpy);
    }

    Entity* EditorDirectionalLight::Instantiate() const
    {
      EditorDirectionalLight* instance = new EditorDirectionalLight();
      return InstantiateTo(instance);
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
      mc->Mesh() = sphere->GetMesh();
      mc->Mesh()->m_material =
      GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      mc->Mesh()->CalculateAABB();
      AddComponent(mc);

      // Directional light gizmo
      m_gizmo = new DirectionalLightGizmo(this);
      int i = 0;
      for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
      {
        mc->Mesh()->m_subMeshes.push_back(lb->GetMesh());
        mc->Mesh()->m_subMeshes[i]->m_material->Init();
        i++;
      }
    }

    EditorPointLight::EditorPointLight()
    {
    }

    EditorPointLight::EditorPointLight(const EditorPointLight* light)
    {
      light->CopyTo(this);
    }

    EditorPointLight::~EditorPointLight()
    {
      if (m_initialized)
      {
        m_initialized = false;
      }
    }

    Entity* EditorPointLight::Copy() const
    {
      EditorPointLight* cpy = new EditorPointLight();
      return CopyTo(cpy);
    }

    Entity* EditorPointLight::Instantiate() const
    {
      EditorPointLight* instance = new EditorPointLight();
      return InstantiateTo(instance);
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
      mc->Mesh() = sphere->GetMesh();
      mc->Mesh()->m_material =
      GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      mc->Mesh()->CalculateAABB();
      AddComponent(mc);
    }

    EditorSpotLight::EditorSpotLight()
    {
    }

    EditorSpotLight::EditorSpotLight(const EditorSpotLight* light)
    {
      light->CopyTo(this);

      AddComponent(new DirectionalComponent(this));
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
      return CopyTo(cpy);
    }

    Entity* EditorSpotLight::Instantiate() const
    {
      EditorSpotLight* instance = new EditorSpotLight();
      return InstantiateTo(instance);
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

      // Light sphere
      std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(0.1f);

      MeshComponent* mc = new MeshComponent();
      mc->Mesh() = sphere->GetMesh();
      mc->Mesh()->m_material =
      GetMaterialManager()->GetCopyOfUnlitColorMaterial();
      mc->Mesh()->CalculateAABB();
      AddComponent(mc);

      // Gizmo
      m_gizmo = new SpotLightGizmo(this);

      int i = 0;
      for (LineBatch* lb : m_gizmo->GetGizmoLineBatches())
      {
        mc->Mesh()->m_subMeshes.push_back(lb->GetMesh());
        mc->Mesh()->m_subMeshes[i]->m_material->Init();
        i++;
      }
    }
  }  // namespace Editor
}  // namespace ToolKit
