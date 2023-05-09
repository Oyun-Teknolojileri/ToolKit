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
      bool HasActiveEntity() const;

     private:
      bool DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags);
      void ShowNode(Entity* e);

     public:
      Entity* m_activeChildEntity = nullptr;
    };
  } // namespace Editor
} // namespace ToolKit