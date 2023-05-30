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

#include "BinPack2D.h"

#include "DebugNew.h"

namespace ToolKit
{
  std::vector<BinPack2D::PackedRect> BinPack2D::Pack(const std::vector<int>& squares, int atlasSize)
  {
    std::vector<PackedRect> packed;
    packed.resize(squares.size());

    std::vector<Layer> layers;
    layers.push_back({{}, 0, atlasSize});

    for (int i = 0; i < squares.size(); ++i)
    {
      int size        = squares[i];

      bool foundShelf = false;
      int layerIndex  = 0;

      // Check active shelves
      for (Layer& layer : layers)
      {
        for (Shelf& shelf : layer.Shelves)
        {
          if (shelf.Fits(size))
          {
            Vec2 coord = shelf.Place(size);
            packed[i]  = {coord, layerIndex};

            foundShelf = true;
          }
        }

        layerIndex++;
      }

      // No shelves found for placing
      if (!foundShelf)
      {
        // Create new shelf in an available layer and place the rectangle there
        bool foundLayer = false;
        int layerIndex  = 0;
        for (Layer& layer : layers)
        {
          if (layer.Fits(size))
          {
            layer.CreateShelf(size, atlasSize);
            Vec2 coord = layer.Shelves.back().Place(size);
            packed[i]  = {coord, layerIndex};

            foundLayer = true;
          }
          layerIndex++;
        }

        // No layer is available for creating this sized shelf, create new layer
        // and new shelf inside of it. Place the rectangle
        if (!foundLayer)
        {
          layers.push_back({{}, 0, atlasSize});
          layers.back().CreateShelf(size, atlasSize);
          Vec2 coord = layers.back().Shelves.back().Place(size);
          packed[i]  = {coord, (int) layers.size() - 1};
        }
      }
    }

    return packed;
  }

  bool BinPack2D::Shelf::Fits(int size) { return size <= AvailableWidth; }

  Vec2 BinPack2D::Shelf::Place(int size)
  {
    Vec2 rectCoord = Coord;
    Coord.x        += size;
    AvailableWidth -= size;

    return rectCoord;
  }

  bool BinPack2D::Layer::Fits(int size) { return size <= MaxHeight - CurrentHeight; }

  void BinPack2D::Layer::CreateShelf(int size, int atlasSize)
  {
    Shelves.push_back({Vec2(0.0f, CurrentHeight), size, atlasSize});
    CurrentHeight += size;
  }
} // namespace ToolKit
