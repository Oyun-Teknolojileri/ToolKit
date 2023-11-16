/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once
#include "MultiChoiceWindow.h"
#include "PropInspector.h"

namespace ToolKit
{
  namespace Editor
  {

    class CustomDataView : public View
    {
     public:
      static void ShowMaterialPtr(const String& uniqueName, const String& file, MaterialPtr& var, bool isEditable);
      static void ShowMaterialVariant(const String& uniqueName, const String& file, ParameterVariant* var);
      static void ShowCustomData(EntityPtr m_entity,
                                 String headerName,
                                 ParameterVariantRawPtrArray& vars,
                                 bool isListEditable);

      static bool BeginShowVariants(StringView header);
      static void ShowVariant(ParameterVariant* var, ParameterVariant*& remove, size_t i, bool isEditable);
      static void EndShowVariants();

      static void ShowVariant(ParameterVariant* var, ComponentPtr comp);
      static ValueUpdateFn MultiUpdate(ParameterVariant* var);
      CustomDataView();
      virtual ~CustomDataView();
      virtual void Show();

     private:
      static MultiChoiceCraeteWindow m_multiChoiceParamWindow;
    };

  } // namespace Editor
} // namespace ToolKit