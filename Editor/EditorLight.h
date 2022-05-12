#pragma once

#include <vector>
#include <string>

#include "ToolKit.h"
#include "Light.h"
#include "Types.h"
#include "Primative.h"
#include "Gizmo.h"


namespace ToolKit
{
  namespace Editor
  {
    class DirectionalLightGimzo;
    class SpotLightGizmo;

    class EditorDirectionalLight : public DirectionalLight
    {
     public:
      EditorDirectionalLight();
      explicit EditorDirectionalLight(const EditorDirectionalLight* light);
      virtual ~EditorDirectionalLight();

      Entity* Copy() const override;
      Entity* Instantiate() const override;
      bool IsDrawable() const override;

      void Init() override;
      void EnableGizmo(bool enable) override;

     private:
       DirectionalLightGizmo* m_gizmo = nullptr;
       MeshComponent* m_gizmoMC = nullptr;
    };

    class EditorPointLight : public PointLight
    {
     public:
      EditorPointLight();
      explicit EditorPointLight(const EditorPointLight* light);
      virtual ~EditorPointLight();

      Entity* Copy() const override;
      Entity* Instantiate() const override;
      bool IsDrawable() const override;

      void Init() override;
    };

    class EditorSpotLight : public SpotLight
    {
     public:
      EditorSpotLight();
      explicit EditorSpotLight(const EditorSpotLight* light);
      virtual ~EditorSpotLight();

      Entity* Copy() const override;
      Entity* Instantiate() const override;
      bool IsDrawable() const override;

      void Init() override;
      void EnableGizmo(bool enable) override;

     private:
      SpotLightGizmo* m_gizmo = nullptr;
    };
  }  // namespace Editor
}  // namespace ToolKit
