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

#include "Action.h"
#include "Gizmo.h"
#include "Mod.h"

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

     private:
      TransformationSpace m_prevTransformSpace;
    };

  } // namespace Editor
} // namespace ToolKit
