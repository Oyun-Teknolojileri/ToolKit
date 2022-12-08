#pragma once

#include <vector>
#include "Types.h"

namespace ToolKit
{
  /**
  * Note: This class works with squares only!
  */

  /**
  * Packs 2D sqaures into an atlas (array of squares)
  */
  class BinPack2D
  {
    struct Shelf
    {
      Vec2 min;
      Vec2 max;
    };

    struct PackedRect
    {
      Vec2 coord;
      Vec2 arrayIndex;
    };

    std::vector<PackedRect> Pack(std::vector<int> squares, int atlasSize);

  };
} // namespace ToolKit
