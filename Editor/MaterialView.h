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
#include "PropInspector.h"

namespace ToolKit
{
  namespace Editor
  {
    class MaterialView : public View
    {
     public:
      MaterialView();
      ~MaterialView();

      void Show() override;
      void SetMaterials(const MaterialPtrArray& mat);
      void ResetCamera();
      void SetSelectedMaterial(MaterialPtr m_mat);

     private:
      void UpdatePreviewScene();
      void ShowMaterial(MaterialPtr m_mat);

     private:
      PreviewViewport* m_viewport = nullptr;
      MaterialPtrArray m_materials;
      uint m_activeObjectIndx    = 0;
      bool m_isMeshChanged       = true;
      int m_currentMaterialIndex = 0;

     public:
      bool m_isTempView = false;
    };

    typedef std::shared_ptr<MaterialView> MaterialViewPtr;

    class TempMaterialWindow : public TempWindow
    {
     public:
      TempMaterialWindow();
      ~TempMaterialWindow();
      void SetMaterial(MaterialPtr mat);
      void OpenWindow();
      void Show() override;

     private:
      MaterialViewPtr m_view;
      bool m_isOpen = true;
    };

    typedef std::shared_ptr<TempMaterialWindow> TempMaterialWindowPtr;

  } // namespace Editor
} // namespace ToolKit