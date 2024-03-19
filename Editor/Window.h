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

    class Window : public Serializable
    {
     public:
      enum class Type
      {
        Viewport       = 0,
        Console        = 1,
        InputPopup     = 2,
        Browser        = 3,
        Outliner       = 4,
        Inspector      = 5,
        UNUSEDSLOT_1   = 6,
        PluginWindow   = 7,
        Viewport2d     = 8,
        RenderSettings = 9,
        Stats          = 10
      };

     public:
      Window();
      virtual ~Window();
      virtual void Show()          = 0;
      virtual Type GetType() const = 0;
      void SetVisibility(bool visible);

      // Window queries.
      bool IsActive() const;
      bool IsVisible() const;
      bool IsMoving() const;
      bool MouseHovers() const;
      bool CanDispatchSignals() const; // If active & visible & mouse hovers.
      bool IsViewport() const;

      // System calls.
      virtual void DispatchSignals() const;

     protected:
      // Internal window handling.
      void HandleStates();
      void SetActive();
      void ModShortCutSignals(const IntArray& mask = {}) const;
      XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
      XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

     protected:
      // States.
      bool m_visible    = true;
      bool m_active     = false;
      bool m_mouseHover = false;
      bool m_moving     = false; //!< States if window is moving.

     public:
      String m_name;
      uint m_id;
      UVec2 m_size;
      IVec2 m_location;

     private:
      // Internal unique id generator.
      static uint m_baseId;
    };

    typedef std::vector<Window*> WindowRawPtrArray;

  } // namespace Editor
} // namespace ToolKit