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

#include "Light.h"

namespace ToolKit
{

  class TK_API SkyBase : public Entity
  {
   public:
    TKDeclareClass(SkyBase, Entity);

    SkyBase();

    EntityType GetType() const override;

    virtual void Init();
    virtual void ReInit();
    void DeSerializeImp(XmlDocument* doc, XmlNode* parent) override;
    bool IsInitialized();

    virtual MaterialPtr GetSkyboxMaterial();
    virtual CubeMapPtr GetIrradianceMap();
    HdriPtr GetHdri();
    BoundingBox GetAABB(bool inWorld = false) const override;

   protected:
    virtual void ParameterConstructor();
    virtual void ParameterEventConstructor();
    void ConstructSkyMaterial(ShaderPtr vertexPrg, ShaderPtr fragPrg);

   public:
    TKDeclareParam(bool, DrawSky);
    TKDeclareParam(bool, Illuminate);
    TKDeclareParam(float, Intensity);

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

    EntityType GetType() const override;
    void Init() override;
    MaterialPtr GetSkyboxMaterial() override;

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;

   public:
    TKDeclareParam(float, Exposure);
    TKDeclareParam(HdriPtr, Hdri);
  };

} // namespace ToolKit
