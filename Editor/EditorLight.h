#pragma once

#include "Gizmo.h"
#include "Light.h"
#include "LightMeshGenerator.h"
#include "Primative.h"
#include "ResourceComponent.h"
#include "ToolKit.h"
#include "Types.h"

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

    class LightGizmoController
    {
     public:
      explicit LightGizmoController(Light* light);
      virtual ~LightGizmoController();

      void EnableGizmo(bool enable) const;
      virtual void Init();

     protected:
      ValueUpdateFn m_gizmoUpdateFn;

     public:
      LightMeshGenerator* m_gizmoGenerator = nullptr;

     protected:
      Light* m_light             = nullptr;
      bool m_initialized         = false;
      mutable bool m_gizmoActive = false;
    };

    class EditorDirectionalLight : public DirectionalLight,
                                   public LightGizmoController
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

    class EditorPointLight : public PointLight, public LightGizmoController
    {
     public:
      EditorPointLight();
      virtual ~EditorPointLight();
      void ParameterEventConstructor() override;

      Entity* Copy() const override;
      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    };

    class EditorSpotLight : public SpotLight, public LightGizmoController
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
