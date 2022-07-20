#pragma once

#include "PropInspector.h"

namespace ToolKit
{
  namespace Editor
  {

    class MaterialView : public View
    {
     public:
      MaterialView() { m_viewID = 3; }
      virtual ~MaterialView() {}
      void Show() override;

     public:
      MaterialPtr m_material;
    };

    class MaterialInspector : public Window
    {
     public:
      explicit MaterialInspector(XmlNode* node);
      MaterialInspector();
      virtual ~MaterialInspector();

      void Show() override;
      Type GetType() const override;
      void DispatchSignals() const override;

     public:
      MaterialPtr m_material;

     private:
      MaterialView* m_view;
    };

  }
}