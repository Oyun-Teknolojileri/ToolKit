/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include <Primative.h>

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

    class Cursor : public EditorBillboardBase
    {
     public:
      TKDeclareClass(Cursor, EditorBillboardBase);

      Cursor();
      virtual ~Cursor();
      BillboardType GetBillboardType() const override;

     protected:
      void Generate() override;
    };

    typedef std::shared_ptr<Cursor> CursorPtr;

    class Axis3d : public EditorBillboardBase
    {
     public:
      TKDeclareClass(Axis3d, EditorBillboardBase);

      Axis3d();
      virtual ~Axis3d();
      BillboardType GetBillboardType() const override;

     private:
      void Generate() override;
    };

    typedef std::shared_ptr<Axis3d> Axis3dPtr;

    class GizmoHandle
    {
     public:
      enum class SolidType
      {
        Cube,
        Cone,
        Circle
      };

      struct Params
      {
        // World space data.
        Vec3 worldLoc;
        Vec3 grabPnt;
        Vec3 initialPnt;
        Mat3 normals;
        // Billboard values.
        Vec3 scale;
        Vec3 translate;
        // Geometry.
        AxisLabel axis;
        Vec3 toeTip;
        Vec3 solidDim;
        Vec3 color;
        SolidType type;
      };

     public:
      GizmoHandle();
      virtual ~GizmoHandle();

      virtual void Generate(const Params& params);
      virtual bool HitTest(const Ray& ray, float& t) const;
      Mat4 GetTransform() const;

     public:
      Vec3 m_tangentDir;
      Params m_params;
      MeshPtr m_mesh = nullptr;
    };

    class PolarHandle : public GizmoHandle
    {
     public:
      void Generate(const Params& params) override;
      bool HitTest(const Ray& ray, float& t) const override;
    };

    class QuadHandle : public GizmoHandle
    {
     public:
      void Generate(const Params& params) override;
      bool HitTest(const Ray& ray, float& t) const override;
    };

    class Gizmo : public EditorBillboardBase
    {
     public:
      TKDeclareClass(Gizmo, EditorBillboardBase);

      Gizmo();
      explicit Gizmo(const Billboard::Settings& set);
      virtual ~Gizmo();
      void NativeConstruct() override;
      BillboardType GetBillboardType() const override;

      virtual AxisLabel HitTest(const Ray& ray) const;
      virtual void Update(float deltaTime) = 0;
      bool IsLocked(AxisLabel axis) const;
      void Lock(AxisLabel axis);
      void UnLock(AxisLabel axis);
      bool IsGrabbed(AxisLabel axis) const;
      void Grab(AxisLabel axis);
      AxisLabel GetGrabbedAxis() const;

      void LookAt(CameraPtr cam, float windowHeight) override;

     protected:
      virtual GizmoHandle::Params GetParam() const;

      /**
       * Collects all the handles under a non empty root mesh for drawing.
       */
      void Consume();

     public:
      Vec3 m_grabPoint;
      Vec3 m_initialPoint;
      Mat3 m_normalVectors;
      AxisLabel m_lastHovered;
      std::vector<GizmoHandle*> m_handles;

     protected:
      std::vector<AxisLabel> m_lockedAxis;
      AxisLabel m_grabbedAxis = AxisLabel::None;
    };

    typedef std::shared_ptr<Gizmo> GizmoPtr;

    class LinearGizmo : public Gizmo
    {
     public:
      TKDeclareClass(LinearGizmo, Gizmo);

      LinearGizmo();
      virtual ~LinearGizmo();

      void Update(float deltaTime) override;

     protected:
      GizmoHandle::Params GetParam() const override;
    };

    class MoveGizmo : public LinearGizmo
    {
     public:
      TKDeclareClass(MoveGizmo, Gizmo);

      MoveGizmo();
      virtual ~MoveGizmo();
      BillboardType GetBillboardType() const override;
    };

    class ScaleGizmo : public LinearGizmo
    {
     public:
      TKDeclareClass(ScaleGizmo, Gizmo);

      ScaleGizmo();
      virtual ~ScaleGizmo();
      BillboardType GetBillboardType() const override;

     protected:
      GizmoHandle::Params GetParam() const override;
    };

    class PolarGizmo : public Gizmo
    {
     public:
      TKDeclareClass(PolarGizmo, Gizmo);

      PolarGizmo();
      virtual ~PolarGizmo();
      BillboardType GetBillboardType() const override;

      void Update(float deltaTime) override;
    };

    class SkyBillboard : public EditorBillboardBase
    {
     public:
      TKDeclareClass(SkyBillboard, EditorBillboardBase);

      SkyBillboard();
      virtual ~SkyBillboard();
      BillboardType GetBillboardType() const override;

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

      void Generate() override;
    };

    typedef std::shared_ptr<LightBillboard> LightBillboardPtr;

  } // namespace Editor
} // namespace ToolKit
