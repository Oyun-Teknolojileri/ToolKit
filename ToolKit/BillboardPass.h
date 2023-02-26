#pragma once
#include "Pass.h"

namespace ToolKit
{

  struct BillboardPassParams
  {
    Viewport* Viewport = nullptr;
    EntityRawPtrArray Billboards;
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
    EntityRawPtrArray m_noDepthBillboards;
  };

  typedef std::shared_ptr<BillboardPass> BillboardPassPtr;

} // namespace ToolKit