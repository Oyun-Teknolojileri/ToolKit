/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
      LightPtrArray m_lights;
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

    class EditorDirectionalLight : public DirectionalLight, public LightGizmoController
    {
     public:
      TKDeclareClass(EditorDirectionalLight, DirectionalLight);

      EditorDirectionalLight();
      virtual ~EditorDirectionalLight();
      TKObjectPtr Copy() const override;
      LineBatchPtr GetDebugShadowFrustum();

     protected:
      XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
      XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    };

    typedef std::shared_ptr<EditorDirectionalLight> EditorDirectionalLightPtr;

    class EditorPointLight : public PointLight, public LightGizmoController
    {
     public:
      TKDeclareClass(EditorPointLight, PointLight);

      EditorPointLight();
      virtual ~EditorPointLight();
      TKObjectPtr Copy() const override;

     protected:
      XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
      XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
      void ParameterEventConstructor() override;
    };

    typedef std::shared_ptr<EditorPointLight> EditorPointLightPtr;

    class EditorSpotLight : public SpotLight, public LightGizmoController
    {
     public:
      TKDeclareClass(EditorSpotLight, SpotLight);

      EditorSpotLight();
      virtual ~EditorSpotLight();
      TKObjectPtr Copy() const override;

     protected:
      XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
      XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
      void ParameterEventConstructor() override;
    };

    typedef std::shared_ptr<EditorSpotLight> EditorSpotLightPtr;

  } // namespace Editor
} // namespace ToolKit
