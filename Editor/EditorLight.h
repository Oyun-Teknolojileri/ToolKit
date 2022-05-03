#pragma once

#include <string>

#include "ToolKit.h"
#include "Light.h"
#include "Types.h"
#include "Primative.h"


namespace ToolKit
{
  namespace Editor
  {
    class EditorDirectionalLight : public DirectionalLight
    {
     public:
      EditorDirectionalLight();
      explicit EditorDirectionalLight(const EditorDirectionalLight* light);
      virtual ~EditorDirectionalLight();

      Entity* Copy() const override;
      Entity* Instantiate() const override;
    };

    class EditorPointLight : public PointLight
    {
     public:
      EditorPointLight();
      explicit EditorPointLight(const EditorPointLight* light);
      virtual ~EditorPointLight();

      Entity* Copy() const override;
      Entity* Instantiate() const override;
    };

    class EditorSpotLight : public SpotLight
    {
     public:
      EditorSpotLight();
      explicit EditorSpotLight(const EditorSpotLight* light);
      virtual ~EditorSpotLight();

      Entity* Copy() const override;
      Entity* Instantiate() const override;
    };
  }  // namespace Editor
}  // namespace ToolKit
