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
   * The Pack() function does the packing.
   * Pack() function returns a PackedRect array. Each PackedRect holds
   * the layer index and the coordinates of the square in the layer.
   *
   *
   * Shelves are holding squares
   * Layer are holding shelves
   *
   * The packing algorithm works as follows:
   * Iterate trough squares that are going to be packed:
   *    If sqaure can fit in any active shelf
   *       Place the sqaure inside that shelf
   *    Else
   *       If there is an available layer to create a shelf
   *         Create a shelf in that layer and place the square
   *       Else
   *         Create a new layer, create a new shelf, place the square
   *
   */

  /**
   * Packs 2D sqaures into an atlas (array of squares)
   */
  class BinPack2D
  {
   public:
    struct PackedRect
    {
      Vec2 Coord     = Vec2(-1.0f);
      int ArrayIndex = -1;
    };

    struct Shelf
    {
      Vec2 Coord         = Vec2(-1.0f);
      int Height         = -1;
      int AvailableWidth = -1;

      bool Fits(int size);
      Vec2 Place(int size);
    };

    struct Layer
    {
      std::vector<Shelf> Shelves;
      int CurrentHeight = -1;
      int MaxHeight     = -1;

      bool Fits(int size);
      void CreateShelf(int size, int atlasSize);
    };

    std::vector<PackedRect> Pack(const IntArray& squares, int atlasSize);
  };
} // namespace ToolKit
