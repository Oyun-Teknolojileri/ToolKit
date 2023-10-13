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

#include "EditorScene.h"
#include "UI.h"

#include <unordered_set>

namespace ToolKit
{
  namespace Editor
  {
    class OutlinerWindow : public Window
    {
     public:
      OutlinerWindow();
      virtual ~OutlinerWindow();
      void Show() override;
      Type GetType() const override;
      void DispatchSignals() const override;
      void Focus(EntityPtr ntt);
      void ClearOutliner();

      // moves the entities below m_insertSelectedIndex
      // make sure m_insertSelectedIndex properly defined before calling this
      // function.
      bool TryReorderEntites(const EntityPtrArray& movedEntities);
      bool IsInsertingAtTheEndOfEntities();

     private:
      bool DrawRootHeader(const String& rootName, uint id, ImGuiTreeNodeFlags flags, TexturePtr icon);

      void ShowSearchBar(String& searchString);
      bool DrawHeader(EntityPtr ntt, ImGuiTreeNodeFlags flags, int depth);

      int ShowNode(EntityPtr ntt, int depth);
      void DrawRowBackground(int depth);
      void SetItemState(EntityPtr ntt);

      void SelectEntitiesBetweenNodes(EditorScenePtr scene, EntityPtr a, EntityPtr b);

      bool FindShownEntities(EntityPtr ntt, const String& str);
      void PushSelectedEntitiesToReparentQueue(EntityPtr parent);

      void SortDraggedEntitiesByNodeIndex();
      bool IndicatingInBetweenNodes();
      int GetMouseHoveredNodeIndex(float treeStartY) const;

     private:
      /**
       * Focus uses this internal array, Show() opens all nodes and sets focus
       * to last ntt in the array.
       */
      EntityPtrArray m_nttFocusPath;
      TKSet<EntityPtr> m_shownEntities;
      /**
       * entities up to down when we look at node tree.
       * these are imgui visible entities.
       */
      EntityPtrArray m_indexToEntity;

      EntityPtrArray m_draggingEntities;
      EntityPtrArray m_roots;
      EntityPtr m_lastClickedEntity = nullptr;
      EntityPtr m_rootsParent       = nullptr;

      String m_searchString;
      bool m_stringSearchMode   = false;
      bool m_searchCaseSens     = true;
      bool m_anyEntityHovered   = false;
      // for even odd pattern
      int odd                   = 0;
      // the objects that we want to reorder will inserted at this index
      int m_insertSelectedIndex = TK_INT_MAX;
      float m_treeStartY        = 0.0;
      int m_searchStringSize    = 0;
    };

  } // namespace Editor
} // namespace ToolKit
