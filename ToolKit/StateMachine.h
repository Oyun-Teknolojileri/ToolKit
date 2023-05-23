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

/**
 * @file StateMachine.h Header for State, StateMachine.
 */

#include "Types.h"

#include <unordered_map>

namespace ToolKit
{

  /**
   * A class that represents a state and implement transitioning mechanisms.
   */
  class TK_API State
  {
   public:
    /**
     * Empty constructor.
     */
    State();

    /**
     * Empty destructor.
     */
    virtual ~State();

    /**
     * Actions to perform while transitioning from previous state to
     * this state.
     * @param prevState State that has been left.
     */
    virtual void TransitionIn(State* prevState)  = 0;

    /**
     * Actions to perform while transitioning from previous state to
     * this state.
     * @param nextState State to transition to.
     */
    virtual void TransitionOut(State* nextState) = 0;

    /**
     * Update logic of the state.
     * @retrun Self signal. State machine signals
     * itself with the returning signal.
     */
    virtual SignalId Update(float deltaTime)     = 0;

    /**
     * Determines the next state, based on the incoming signal.
     * @param signal Incoming signal.
     * @retrun Next state's name.
     */
    virtual String Signaled(SignalId signal)     = 0;

    /**
     * Unique string identifier within the state machine. Two different states
     * should not share the same type.
     * @return String identifier of the state.
     */
    virtual String GetType()                     = 0;

    /**
     * Test fucntion to check state types.
     * Subject to delete. Don't use RTTI.
     * @returns True if the type of the current node is equal the queried type.
     */
    template <typename T>
    bool ThisIsA()
    {
      return typeid(*this) == typeid(T);
    }

   public:
    /**
     * Map that shows where to jump incase a state returns null but signaled
     * with SignalId. This utility provides state hijacking mechanism. That is
     * even if there is no transition between states, this link map helps
     * to make the jump.
     */
    std::unordered_map<int, String> m_links;

    /**
     * Global signal that tells not to take any action to the StateMachine.
     */
    static SignalId NullSignal = -1;
  };

  /**
   * A class that holds states and manages transitions based on signals.
   */
  class TK_API StateMachine
  {
   public:
    /**
     * Default constructor that assigns default values.
     */
    StateMachine();

    /**
     * Destructs all states and frees the memory.
     */
    ~StateMachine();

    /**
     * Signals the current state.
     * @param signal Incoming signal.
     */
    void Signal(SignalId signal);

    /**
     * Serach the given signal type.
     * @return Queried State if it found or nullptr.
     */
    State* QueryState(const String& type);

    /**
     * Add a state to the StateMachine.
     * @param state State to add.
     */
    void PushState(State* state);

    /**
     * Update the current state.
     * @param deltaTime Elapsed delta time on previous frame.
     */
    void Update(float deltaTime);

   public:
    State* m_currentState; //!< Current state.

   private:
    std::unordered_map<String, State*> m_states; //!< State container.
  };

} // namespace ToolKit
