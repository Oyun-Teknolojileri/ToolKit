/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

namespace ToolKit
{

  /**
   * Packs 2D squares into an atlas (array of squares)
   * Squares must be sorted from smallest to largest to properly placed.
   * Algorithm is not optimal, meaning that it may not fit all the squares tightly
   * Algorithm fallows a sequence from left to right, goes one up and left to right
   * than next layer. This sequence is expected in the shader to find the map's layer and coordinate.
   */
  class BinPack2D
  {
   public:
    struct PackedRect
    {
      Vec2 coordinate = Vec2(-1.0f);
      int layer       = -1;
    };

    typedef std::vector<PackedRect> PackedRectArray;

    PackedRectArray Pack(const IntArray& squares, int atlasSize, int* layerCount = nullptr);
  };

} // namespace ToolKit
