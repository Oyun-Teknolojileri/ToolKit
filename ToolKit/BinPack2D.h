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

#include "Types.h"

#include <vector>

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

    std::vector<PackedRect> Pack(const std::vector<int>& squares, int atlasSize);
  };
} // namespace ToolKit
