#pragma once

#include "Mod.h"
#include "Gizmo.h"

namespace ToolKit
{
  namespace Editor
  {
    // States.
    class StateTransformBase : public State
    {
    public:
      enum class TransformType
      {
        Translate,
        Rotate,
        Scale
      };

    public:
      StateTransformBase();
      virtual void Update(float deltaTime) override;
      virtual void TransitionIn(State* prevState) override;
      virtual void TransitionOut(State* nextState) override;

    protected:
      void MakeSureGizmoIsValid();
      Vec3 GetGrabbedAxis(int n); // {0: grabbed 1: orthogonal axis}.
      bool IsPlaneMod();

    public:
      Gizmo* m_gizmo;
      std::vector<Vec2> m_mouseData;
      PlaneEquation m_intersectionPlane;
      TransformType m_type;
    };

    class StateTransformBegin : public StateTransformBase
    {
    public:
      virtual void TransitionIn(State* prevState) override;
      virtual void TransitionOut(State* nextState) override;

      virtual void Update(float deltaTime) override;
      virtual String Signaled(SignalId signal) override;
      virtual String GetType() override { return StateType::StateTransformBegin; }

    private:
      void CalculateIntersectionPlane();
      void CalculateGrabPoint();
    };

    class TransformAction : public Action
    {
    public:
      TransformAction(Entity* ntt);
      virtual ~TransformAction();

      virtual void Undo();
      virtual void Redo();

    private:
      void Swap();

    private:
      Entity* m_entity;
      Mat4 m_transform;
    };

    class StateTransformTo : public StateTransformBase
    {
    public:
      virtual void TransitionIn(State* prevState) override;
      virtual void TransitionOut(State* prevState) override;
      virtual void Update(float deltaTime) override;
      virtual String Signaled(SignalId signal) override;
      virtual String GetType() override { return StateType::StateTransformTo; }

    private:
      void CalculateDelta();
      void Transform(const Vec3& delta);

    public:
      Vec3 m_delta;
    };

    class StateTransformEnd : public StateTransformBase
    {
    public:
      virtual void TransitionOut(State* nextState) override;
      virtual void Update(float deltaTime) override;
      virtual String Signaled(SignalId signal) override;
      virtual String GetType() override { return StateType::StateTransformEnd; }
    };

    // Mod.
    class TransformMod : public BaseMod
    {
    public:
      TransformMod(ModId id);
      virtual ~TransformMod();

      virtual void Init() override;
      virtual void UnInit() override;
      virtual void Update(float deltaTime) override;

    public:
      Gizmo* m_gizmo;
      TransformationSpace m_prevTransformSpace;
    };
  }
}