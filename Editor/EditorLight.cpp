
#include "EditorLight.h"

#include <string>

#include "Material.h"
#include "Texture.h"


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
    }

    EditorDirectionalLight::~EditorDirectionalLight()
    {
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

    EditorPointLight::EditorPointLight()
    {
    }

    EditorPointLight::EditorPointLight(const EditorPointLight* light)
    {
      light->CopyTo(this);
    }

    EditorPointLight::~EditorPointLight()
    {
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

    EditorSpotLight::EditorSpotLight()
    {
    }

    EditorSpotLight::EditorSpotLight(const EditorSpotLight* light)
    {
      light->CopyTo(this);
    }

    EditorSpotLight::~EditorSpotLight()
    {
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

  }  // namespace Editor
}  // namespace ToolKit

