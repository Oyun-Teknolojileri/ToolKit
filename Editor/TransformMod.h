#pragma once

#include <vector>

#include "Mod.h"
#include "Gizmo.h"
#include "Action.h"

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
      SignalId Update(float deltaTime) override;
      void TransitionIn(State* prevState) override;
      void TransitionOut(State* nextState) override;

     protected:
      void MakeSureGizmoIsValid();
      Vec3 GetGrabbedAxis(int n);  // {0: grabbed 1: orthogonal axis}.
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
      void TransitionIn(State* prevState) override;
      void TransitionOut(State* nextState) override;

      SignalId Update(float deltaTime) override;
      String Signaled(SignalId signal) override;
      String GetType() override;

     private:
      void CalculateIntersectionPlane();
      void CalculateGrabPoint();
    };

    class TransformAction : public Action
    {
     public:
      explicit TransformAction(Entity* ntt);
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
      void TransitionIn(State* prevState) override;
      void TransitionOut(State* prevState) override;
      SignalId Update(float deltaTime) override;
      String Signaled(SignalId signal) override;
      String GetType() override;

     private:
      void CalculateDelta();
      void Transform(const Vec3& delta);
      void Translate(Entity* ntt);
      void Rotate(Entity* ntt);
      void Scale(Entity* ntt);

     public:
      Vec3 m_delta;
      Vec3 m_deltaAccum;
      Vec3 m_initialLoc;

     private:
      IVec2 m_mouseInitialLoc;
    };

    class StateTransformEnd : public StateTransformBase
    {
     public:
      void TransitionOut(State* nextState) override;
      SignalId Update(float deltaTime) override;
      String Signaled(SignalId signal) override;
      String GetType() override;
    };

    // Mod.
    class TransformMod : public BaseMod
    {
     public:
      explicit TransformMod(ModId id);
      virtual ~TransformMod();

      void Init() override;
      void UnInit() override;
      void Update(float deltaTime) override;

     public:
      Gizmo* m_gizmo;
      TransformationSpace m_prevTransformSpace;
    };

  }  // namespace Editor
}  // namespace ToolKit
