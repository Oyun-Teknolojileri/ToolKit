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

#include "UI.h"

#include <unordered_set>

namespace ToolKit
{
  namespace Editor
  {
    class OutlinerWindow : public Window
    {
     public:
      explicit OutlinerWindow(XmlNode* node);
      OutlinerWindow();
      virtual ~OutlinerWindow();
      void Show() override;
      Type GetType() const override;
      void DispatchSignals() const override;
      void Focus(Entity* ntt);

      // moves the entities below m_insertSelectedIndex
      // make sure m_insertSelectedIndex properly defined before calling this
      // function.
      bool TryReorderEntites(const EntityRawPtrArray& movedEntities);
      bool IsInsertingAtTheEndOfEntities();

     private:
      bool DrawRootHeader(const String& rootName, uint id, ImGuiTreeNodeFlags flags, TexturePtr icon);

      void ShowSearchBar(String& searchString);
      bool DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags, int depth);

      int ShowNode(Entity* e, int depth);
      void DrawRowBackground(int depth);
      void SetItemState(Entity* e);

      void SelectEntitiesBetweenNodes(class EditorScene* scene, Entity* a, Entity* b);

      bool FindShownEntities(Entity* e, const String& str);
      void PushSelectedEntitiesToReparentQueue(Entity* parent);

      void SortDraggedEntitiesByNodeIndex();
      bool IndicatingInBetweenNodes();
      int GetMouseHoveredNodeIndex(float treeStartY) const;

     private:
      /**
       * Focus uses this internal array, Show() opens all nodes and sets focus
       * to last ntt in the array.
       */
      EntityRawPtrArray m_nttFocusPath;
      std::unordered_set<Entity*> m_shownEntities;
      /**
       * entities up to down when we look at node tree.
       * these are imgui visible entities.
       */
      EntityRawPtrArray m_indexToEntity;

      EntityRawPtrArray m_draggingEntities;
      EntityRawPtrArray m_roots;
      Entity* m_lastClickedEntity = nullptr;
      Entity* m_rootsParent       = nullptr;

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
