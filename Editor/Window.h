/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "EditorTypes.h"

#include <Object.h>
#include <Types.h>

namespace ToolKit
{
  namespace Editor
  {

    class TK_EDITOR_API Window : public Object
    {
     public:
      TKDeclareClass(Window, Object);

     public:
      Window();
      virtual ~Window();
      virtual void Show() = 0;

      // Window queries.

      /**
       * States if the window is visible and showing its content. That is, the window is not minimized or in an
       * inactive tab.
       */
      bool IsShown();

      /** States if the window is active window. A window is considered active if it has input focus. */
      bool IsActive() const;

      /**
       * States if the window is visible, it can be tabbed or minimized.
       * If you want to check if its rendering content, use IsShown().
       */
      bool IsVisible() const;

      /** States if the window is moving or not. */
      bool IsMoving() const;

      /** States if the mouse is over the window or not. */
      bool MouseHovers() const;

      // Window functions.

      /**
       * Set window as visible, if its not visible it won't be shown. However, it may not be shown due to being an
       * inactive tab or minimized window. Use IsShown to detect if the window is being shown or not.
       */
      void SetVisibility(bool visible);

      /**
       * States if a window can dispatch signals or not.
       * If window is active & visible & mouse hovers on it, it can dispatch signals.
       */
      bool CanDispatchSignals() const;

      /** Dispatch signals such as short cuts or system events. Mode manager captures and process signals. */
      virtual void DispatchSignals() const;

      /**
       * Adds this window to UI, only after that it processed by the UI system.
       * Otherwise nothing appears on the editor window.
       */
      void AddToUI();

      /**
       * UI system removes its reference and UI stops processing the window.
       * Object does not gets destroyed and can be re added.
       */
      void RemoveFromUI();

      /** Internally used to reset per frame state such as m_isShown. */
      void ResetState();

     protected:
      // Internal window handling.

      /**
       *Handle internal states of the window.
       * Should be called in between the Show::ImGui::Begin / End function.
       */
      void HandleStates();

      /** Set the window as active window. */
      void SetActive();

      /** Internally used by HandleStates to try activating window with any mouse click. */
      void TryActivateWindow();

      /** Global shortcuts handled in this function. Can be overridden for extending it with new short cuts. */
      void ModShortCutSignals(const IntArray& mask = {}) const;

      XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
      XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

     protected:
      // States.

      /** States if the window is visible, doesn't mean that its being shown. May be in a tab and hidden. */
      bool m_visible    = true;

      /** States if the visible window is shown, not in a hidden tab or minimized. */
      bool m_isShown    = false;

      /** States if the input focus is on this window. */
      bool m_active     = false;

      /** States if the mouse is over this window. */
      bool m_mouseHover = false;

      /** States if the user is dragging the window. */
      bool m_moving     = false;

     public:
      String m_name;    //!< Name of the window.
      uint m_id;        //!< Unique window id for the current runtime.
      UVec2 m_size;     //!< Total window size.
      IVec2 m_location; //!< Screen space window location.

     private:
      static uint m_baseId; //!< Internal counter for unique ids in current runtime.
    };

    typedef std::shared_ptr<Window> WindowPtr;
    typedef std::vector<WindowPtr> WindowPtrArray;

  } // namespace Editor
} // namespace ToolKit