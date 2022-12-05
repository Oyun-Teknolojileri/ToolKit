#pragma once
#include "PropInspector.h"

namespace ToolKit
{
  namespace Editor
  {
    class CustomDataView : public View
    {
     public:
      static void ShowMaterialPtr(const String& uniqueName,
                                  const String& file,
                                  MaterialPtr& var,
                                  bool isEditable);
      static void ShowMaterialVariant(const String& uniqueName,
                                      const String& file,
                                      ParameterVariant* var);
      static void ShowCustomData(Entity* m_entity,
                                 String headerName,
                                 ParameterVariantRawPtrArray& vars,
                                 bool isListEditable);
      static void ShowVariant(ParameterVariant* var, ComponentPtr comp);
      static ValueUpdateFn MultiUpdate(ParameterVariant* var);
      CustomDataView();
      virtual ~CustomDataView();
      virtual void Show();
    };
  } // namespace Editor
} // namespace ToolKit