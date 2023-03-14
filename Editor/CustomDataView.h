#pragma once
#include "MultiChoiceParameterWindow.h"
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

      static bool BeginShowVariants(StringView header);
      static void ShowVariant(ParameterVariant* var,
                              ParameterVariant*& remove,
                              size_t i,
                              bool isEditable);
      static void EndShowVariants();

      static void ShowVariant(ParameterVariant* var, ComponentPtr comp);
      static ValueUpdateFn MultiUpdate(ParameterVariant* var);
      CustomDataView();
      virtual ~CustomDataView();
      virtual void Show();

     private:
      static MultiChoiceParameterWindow m_multiChoiceParamWindow;
    };
  } // namespace Editor
} // namespace ToolKit