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
#include "Anchor.h"
#include "Mod.h"

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
      void ReflectAnchorTransform(EntityPtr ntt);

     public:
      AnchorPtr m_anchor;
      std::vector<Vec2> m_mouseData;
      PlaneEquation m_intersectionPlane;
      TransformType m_type;
      bool m_signalConsumed = true;

     protected:
      Vec3 m_anchorDeltaTransform; // Anchor medallion change.
      Vec3 m_deltaAccum;
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
      explicit AnchorAction(EntityPtr ntt);
      virtual ~AnchorAction();

      virtual void Undo();
      virtual void Redo();

     private:
      void Swap();

     private:
      EntityPtr m_entity;
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
