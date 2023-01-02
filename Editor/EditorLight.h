#pragma once

#include "Gizmo.h"
#include "Light.h"
#include "Primative.h"
#include "ResourceComponent.h"
#include "ToolKit.h"
#include "Types.h"

#include <string>
#include <vector>

namespace ToolKit
{
  namespace Editor
  {

    class ThreePointLightSystem
    {
     public:
      ThreePointLightSystem();
      ~ThreePointLightSystem();

     public:
      LightRawPtrArray m_lights;
      Node* m_parentNode = nullptr;
    };

    typedef std::shared_ptr<ThreePointLightSystem> ThreePointLightSystemPtr;

    // Editor Light Utils.
    extern void EnableLightGizmo(Light* light, bool enable);

    class EditorLightBase
    {
     public:
      explicit EditorLightBase(Light* light);
      virtual ~EditorLightBase();

      void EnableGizmo(bool enable);
      virtual void Init();

     protected:
      ValueUpdateFn m_gizmoUpdateFn;

     public:
      LightGizmoBase* m_gizmo    = nullptr;
      MeshComponentPtr m_gizmoMC = nullptr;

     protected:
      Light* m_light     = nullptr;
      bool m_gizmoActive = false;
      bool m_initialized = false;
    };

    class EditorDirectionalLight : public DirectionalLight,
                                   public EditorLightBase
    {
     public:
      EditorDirectionalLight();
      virtual ~EditorDirectionalLight();
      void ParameterEventConstructor() override;

      Entity* Copy() const override;
      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

      LineBatch* GetDebugShadowFrustum();
    };

    class EditorPointLight : public PointLight, public EditorLightBase
    {
     public:
      EditorPointLight();
      virtual ~EditorPointLight();
      void ParameterEventConstructor() override;

      Entity* Copy() const override;
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
      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    };

  } // namespace Editor
} // namespace ToolKit
