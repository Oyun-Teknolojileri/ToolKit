/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Pass.h"

namespace ToolKit
{

  struct BillboardPassParams
  {
    Viewport* Viewport = nullptr;
    EntityPtrArray Billboards;
  };

  class TK_API BillboardPass : public Pass
  {
   public:
    BillboardPass();
    explicit BillboardPass(const BillboardPassParams& params);
    ~BillboardPass();

    void Render() override;
    void PreRender() override;

   public:
    BillboardPassParams m_params;
    EntityPtrArray m_noDepthBillboards;

   private:
    RenderData m_renderData;
  };

  typedef std::shared_ptr<BillboardPass> BillboardPassPtr;

} // namespace ToolKit