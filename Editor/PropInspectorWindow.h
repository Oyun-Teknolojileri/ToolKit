/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "EditorViewport.h"
#include "UI.h"
#include "View.h"

namespace ToolKit
{

  namespace Editor
  {

    class PropInspectorWindow : public Window
    {
     public:
      TKDeclareClass(PropInspectorWindow, Window);

      PropInspectorWindow();
      explicit PropInspectorWindow(XmlNode* node);
      virtual ~PropInspectorWindow();
      void SetActiveView(ViewType viewType);
      class MaterialView* GetMaterialView();

      void Show() override;
      void DispatchSignals() const override;
      void SetMaterials(const MaterialPtrArray& mat);
      void SetMeshView(MeshPtr mesh);

     private:
      void DeterminateSelectedMaterial(EntityPtr curEntity);

     public:
      ViewRawPtrArray m_views;
      UIntArray m_prefabViews;
      UIntArray m_entityViews;

      ViewType m_activeView = ViewType::Entity;
    };

    typedef std::shared_ptr<PropInspectorWindow> PropInspectorWindowPtr;

  } // namespace Editor
} // namespace ToolKit
