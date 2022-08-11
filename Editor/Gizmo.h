#pragma once

#include <vector>

#include "Primative.h"
#include "MathUtil.h"
#include "Light.h"
#include "Renderer.h"
#include "Viewport.h"

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
        Sky
      };

     public:
      explicit EditorBillboardBase(const Settings& settings);
      virtual BillboardType GetBillboardType() const = 0;
    };

    class Cursor : public EditorBillboardBase
    {
     public:
      Cursor();
      virtual ~Cursor();
      BillboardType GetBillboardType() const override;

     private:
      void Generate();
    };

    class Axis3d : public EditorBillboardBase
    {
     public:
      Axis3d();
      virtual ~Axis3d();
      BillboardType GetBillboardType() const override;

     private:
      void Generate();
    };

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
        // Worldspace data.
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
      MeshPtr m_mesh;
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
      explicit Gizmo(const Billboard::Settings& set);
      virtual ~Gizmo();
      BillboardType GetBillboardType() const override;

      virtual AxisLabel HitTest(const Ray& ray) const;
      virtual void Update(float deltaTime) = 0;
      bool IsLocked(AxisLabel axis) const;
      void Lock(AxisLabel axis);
      void UnLock(AxisLabel axis);
      bool IsGrabbed(AxisLabel axis) const;
      void Grab(AxisLabel axis);
      AxisLabel GetGrabbedAxis() const;

      void LookAt(class Camera* cam, float windowHeight) override;

     protected:
      virtual GizmoHandle::Params GetParam() const;

     public:
      Vec3 m_grabPoint;
      Vec3 m_initialPoint;
      Mat3 m_normalVectors;
      AxisLabel m_lastHovered;
      std::vector<GizmoHandle*> m_handles;

     protected:
      std::vector<AxisLabel> m_lockedAxis;
      AxisLabel m_grabbedAxis;
    };

    class LinearGizmo : public Gizmo
    {
     public:
      LinearGizmo();
      virtual ~LinearGizmo();

      void Update(float deltaTime) override;

     protected:
      GizmoHandle::Params GetParam() const override;
    };

    class MoveGizmo : public LinearGizmo
    {
     public:
      MoveGizmo();
      virtual ~MoveGizmo();
    };

    class ScaleGizmo : public LinearGizmo
    {
     public:
      ScaleGizmo();
      virtual ~ScaleGizmo();

     protected:
      GizmoHandle::Params GetParam() const override;
    };

    class PolarGizmo : public Gizmo
    {
     public:
      PolarGizmo();
      virtual ~PolarGizmo();

      void Update(float deltaTime) override;
      void Render(Renderer* renderer, Camera* cam);
    };

    class SkyBillboard : public EditorBillboardBase
    {
     public:
      SkyBillboard();
      virtual ~SkyBillboard();
      BillboardType GetBillboardType() const override;

     private:
      void Generate();
    };

    class LightGizmoBase
    {
     public:
      LightGizmoBase();
      virtual ~LightGizmoBase();

      virtual void InitGizmo(Light* light) = 0;
      LineBatchRawPtrArray GetGizmoLineBatches();

     protected:
      LineBatchRawPtrArray m_gizmoLineBatches;
    };

    class SpotLightGizmo : public Entity, public LightGizmoBase
    {
     public:
      explicit SpotLightGizmo(SpotLight* light);
      void InitGizmo(Light* light) override;

     private:
      int m_circleVertexCount;
      Vec3Array m_pnts;
      Vec3Array m_innerCirclePnts;
      Vec3Array m_outerCirclePnts;
      Vec3Array m_conePnts;
      Mat4 m_identityMatrix;
      Mat4 m_rot;
    };

    class DirectionalLightGizmo : public Entity, public LightGizmoBase
    {
     public:
      explicit DirectionalLightGizmo(DirectionalLight* light);
      void InitGizmo(Light* light) override;

     private:
      Vec3Array m_pnts;
    };

    class PointLightGizmo : public Entity, public LightGizmoBase
    {
     public:
      explicit PointLightGizmo(PointLight* light);
      void InitGizmo(Light* light) override;

     private:
      int m_circleVertexCount = 30;
      Vec3Array m_circlePntsXY;
      Vec3Array m_circlePntsYZ;
      Vec3Array m_circlePntsXZ;
    };

  }  // namespace Editor
}  // namespace ToolKit
