#pragma once
#include "PropInspector.h"

namespace ToolKit
{
  namespace Editor
  {
    class PrefabView : public View
    {
     public:
      PrefabView();
      virtual ~PrefabView();
      virtual void Show();

     private:
      bool DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags);
      void ShowNode(Entity* e);

     private:
      Entity* m_activeChildEntity = nullptr;
    };
  } // namespace Editor
} // namespace ToolKit