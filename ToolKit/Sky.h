/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Light.h"

namespace ToolKit
{
  static VariantCategory SkyCategory {"Sky", 90};

  class TK_API SkyBase : public Entity
  {
   public:
    TKDeclareClass(SkyBase, Entity);

    SkyBase();
    void NativeConstruct() override;

    virtual void Init();
    virtual void ReInit();
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    bool IsInitialized();

    virtual MaterialPtr GetSkyboxMaterial();
    HdriPtr GetHdri();

    /** Returns a unit bounding box. */
    const BoundingBox& GetBoundingBox(bool inWorld = false) override;

    /** Returns true when hdr is loaded and initialized. */
    virtual bool IsReadyToRender();

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    void ConstructSkyMaterial(ShaderPtr vertexPrg, ShaderPtr fragPrg);
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

    /** Returns the environment map baked file name without mip level post fix. */
    String GetBakedEnvironmentFileBaseName(bool specular);

   public:
    TKDeclareParam(bool, DrawSky);
    TKDeclareParam(bool, Illuminate);
    TKDeclareParam(float, Intensity);
    TKDeclareParam(MultiChoiceVariant, IBLTextureSize);
    TKDeclareParam(VariantCallback, BakeIrradianceMap);

   protected:
    bool m_initialized           = false;
    MaterialPtr m_skyboxMaterial = nullptr;
  };

  class TK_API Sky : public SkyBase
  {
   public:
    TKDeclareClass(Sky, SkyBase);

    Sky();
    virtual ~Sky();

    void Init() override;
    MaterialPtr GetSkyboxMaterial() override;

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   public:
    TKDeclareParam(float, Exposure);
    TKDeclareParam(HdriPtr, Hdri);
  };

} // namespace ToolKit
