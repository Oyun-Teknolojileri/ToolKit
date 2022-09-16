#pragma once

#include "Action.h"
#include "Anchor.h"
#include "Mod.h"

#include <vector>

namespace ToolKit
{
  namespace Editor
  {

    // States.
    class StateAnchorBase : public State
    {
     public:
      enum class TransformType
      {
        Translate
      };

     public:
      StateAnchorBase();
      SignalId Update(float deltaTime) override;
      void TransitionIn(State* prevState) override;
      void TransitionOut(State* nextState) override;

     protected:
      void MakeSureAnchorIsValid();
      void ReflectAnchorTransform(Entity* ntt);

     public:
      AnchorPtr m_anchor;
      std::vector<Vec2> m_mouseData;
      PlaneEquation m_intersectionPlane;
      TransformType m_type;
      bool m_signalConsumed = true;

     protected:
      Vec3 m_anchorDeltaTransform; // Anchor medallion change.
    };

    class StateAnchorBegin : public StateAnchorBase
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

    class AnchorAction : public Action
    {
     public:
      explicit AnchorAction(Entity* ntt);
      virtual ~AnchorAction();

      virtual void Undo();
      virtual void Redo();

     private:
      void Swap();

     private:
      Entity* m_entity;
      Mat4 m_transform;
    };

    class StateAnchorTo : public StateAnchorBase
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

     public:
      Vec3 m_deltaAccum;
      Vec3 m_initialLoc;

     private:
      IVec2 m_mouseInitialLoc;
    };

    class StateAnchorEnd : public StateAnchorBase
    {
     public:
      void TransitionOut(State* nextState) override;
      SignalId Update(float deltaTime) override;
      String Signaled(SignalId signal) override;
      String GetType() override;
    };

    // Mod.
    class AnchorMod : public BaseMod
    {
     public:
      explicit AnchorMod(ModId id);
      virtual ~AnchorMod();

      void Init() override;
      void UnInit() override;
      void Update(float deltaTime) override;

     public:
      AnchorPtr m_anchor;
      TransformationSpace m_prevTransformSpace;
    };

  } // namespace Editor
} // namespace ToolKit
