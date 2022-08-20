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
      EditorLightBase();
      virtual ~EditorLightBase();

      void EnableGizmo(Light* light, bool enable);
      virtual void Init() = 0;
      virtual void Init(Light* light);
      virtual void ParameterEventConstructor() = 0;

     protected:
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
      explicit EditorDirectionalLight(const EditorDirectionalLight* light);
      virtual ~EditorDirectionalLight();
      void ParameterEventConstructor() override;

      Entity* Copy() const override;
      Entity* Instantiate() const override;
      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

      void Init() override;
    };

    class EditorPointLight : public PointLight, public EditorLightBase
    {
     public:
      EditorPointLight();
      explicit EditorPointLight(const EditorPointLight* light);
      virtual ~EditorPointLight();
      void ParameterEventConstructor() override;

      Entity* Copy() const override;
      Entity* Instantiate() const override;
      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

      void Init() override;
    };

    class EditorSpotLight : public SpotLight, public EditorLightBase
    {
     public:
      EditorSpotLight();
      explicit EditorSpotLight(const EditorSpotLight* light);
      virtual ~EditorSpotLight();
      void ParameterEventConstructor() override;

      Entity* Copy() const override;
      Entity* Instantiate() const override;
      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

      void Init() override;
    };

  }  // namespace Editor
}  // namespace ToolKit
