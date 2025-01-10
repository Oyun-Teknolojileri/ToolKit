/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "SpriteSheet.h"
#include "Surface.h"
#include "Types.h"

namespace ToolKit
{
  static VariantCategory CanvasCategory {"Canvas", 90};

  class TK_API Canvas : public Surface
  {
   public:
    TKDeclareClass(Canvas, Surface);

    Canvas();
    void NativeConstruct() override;

    void UpdateGeometry(bool byTexture) override;
    void ApplyRecursiveResizePolicy(float width, float height);

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

    virtual void DeserializeComponents(const SerializationFileInfo& info, XmlNode* entityNode);
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent);

   private:
    void CreateQuadLines();

   public:
    // Local events.
    SurfaceEventCallback m_onMouseEnterLocal;
    SurfaceEventCallback m_onMouseExitLocal;

   private:
    MaterialPtr m_canvasMaterial;
  };
} //  namespace ToolKit
