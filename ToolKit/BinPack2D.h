#pragma once

#include "Types.h"

#include <vector>

namespace ToolKit
{
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

    std::vector<PackedRect> Pack(std::vector<int> squares, int atlasSize);
  };
} // namespace ToolKit
