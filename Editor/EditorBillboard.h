/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

namespace ToolKit
{
  namespace Editor
  {

    class EditorBillboardBase : public Billboard
    {
     public:
      enum class BillboardType
      {
        Cursor,
        Axis3d,
        Gizmo,
        Move,
        Rotate,
        Scale,
        Sky,
        Light,
        Anchor
      };

     public:
      TKDeclareClass(EditorBillboardBase, Billboard);

      EditorBillboardBase();
      explicit EditorBillboardBase(const Settings& settings);
      virtual BillboardType GetBillboardType() const = 0;
      void NativeConstruct() override;

     protected:
      virtual void Generate();

     protected:
      TexturePtr m_iconImage = nullptr;
    };

    typedef std::shared_ptr<EditorBillboardBase> EditorBillboardPtr;
    typedef std::vector<EditorBillboardPtr> BillboardPtrArray;
    typedef std::vector<EditorBillboardBase*> BillboardRawPtrArray;

    class SkyBillboard : public EditorBillboardBase
    {
     public:
      TKDeclareClass(SkyBillboard, EditorBillboardBase);

      SkyBillboard();
      virtual ~SkyBillboard();
      BillboardType GetBillboardType() const override;
      void LookAt(CameraPtr cam, float scale) override;

     private:
      void Generate() override;
    };

    typedef std::shared_ptr<SkyBillboard> SkyBillboardPtr;

    class LightBillboard : public EditorBillboardBase
    {
     public:
      TKDeclareClass(LightBillboard, EditorBillboardBase);

      LightBillboard();
      virtual ~LightBillboard();
      BillboardType GetBillboardType() const override;
      void LookAt(CameraPtr cam, float scale) override;

     private:
      void Generate() override;
    };

    typedef std::shared_ptr<LightBillboard> LightBillboardPtr;

  } // namespace Editor
} // namespace ToolKit