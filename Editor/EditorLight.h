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
    // Editor Light Utils.
    extern void EnableLightGizmo(Light* light, bool enable);

    class EditorLightBase
    {
     public:
      EditorLightBase(Light* light);
      virtual ~EditorLightBase();

      void EnableGizmo(bool enable);
      virtual void Init();
      virtual void ParameterEventConstructor() = 0;

     protected:
      ValueUpdateFn m_gizmoUpdateFn;

     protected:
      Light* m_light = nullptr;
      LightGizmoBase* m_gizmo = nullptr;
      MeshComponentPtr m_gizmoMC = nullptr;
      bool m_gizmoActive = false;
      bool m_initialized = false;
    };

    class EditorDirectionalLight :
      public DirectionalLight,
      public EditorLightBase
    {
     public:
      EditorDirectionalLight();
      virtual ~EditorDirectionalLight();
      void ParameterEventConstructor() override;

      Entity* Copy() const override;
      Entity* Instantiate() const override;
      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    };

    class EditorPointLight : public PointLight, public EditorLightBase
    {
     public:
      EditorPointLight();
      virtual ~EditorPointLight();
      void ParameterEventConstructor() override;

      Entity* Copy() const override;
      Entity* Instantiate() const override;
      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    };

    class EditorSpotLight : public SpotLight, public EditorLightBase
    {
     public:
      EditorSpotLight();
      virtual ~EditorSpotLight();
      void ParameterEventConstructor() override;

      Entity* Copy() const override;
      Entity* Instantiate() const override;
      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    };

  }  // namespace Editor
}  // namespace ToolKit
