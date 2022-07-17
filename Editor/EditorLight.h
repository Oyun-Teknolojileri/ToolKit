#pragma once

#include <vector>
#include <string>

#include "ToolKit.h"
#include "Light.h"
#include "Types.h"
#include "Primative.h"
#include "Gizmo.h"
#include "ResourceComponent.h"


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

      void Init() override;
      void EnableGizmo(bool enable);

     private:
       DirectionalLightGizmo* m_gizmo = nullptr;
       MeshComponent* m_gizmoMC = nullptr;

       bool m_gizmoActive = false;
    };

    class EditorPointLight : public PointLight
    {
     public:
      EditorPointLight();
      explicit EditorPointLight(const EditorPointLight* light);
      virtual ~EditorPointLight();

      Entity* Copy() const override;
      Entity* Instantiate() const override;

      void Init() override;
      void EnableGizmo(bool enable);

     private:
      PointLightGizmo* m_gizmo = nullptr;
      bool m_gizmoActive = false;
    };

    class EditorSpotLight : public SpotLight
    {
     public:
      EditorSpotLight();
      explicit EditorSpotLight(const EditorSpotLight* light);
      virtual ~EditorSpotLight();

      Entity* Copy() const override;
      Entity* Instantiate() const override;

      void Init() override;
      void EnableGizmo(bool enable);

     private:
      SpotLightGizmo* m_gizmo = nullptr;

      bool m_gizmoActive = false;
    };

  }  // namespace Editor
}  // namespace ToolKit
