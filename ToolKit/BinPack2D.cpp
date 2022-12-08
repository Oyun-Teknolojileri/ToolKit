#include "BinPack2D.h"

namespace ToolKit
{
  std::vector<BinPack2D::PackedRect> BinPack2D::Pack(std::vector<int> squares,
                                                     int atlasSize)
  {
    std::vector<PackedRect> packed;
    packed.resize(squares.size());

    std::vector<Layer> layers;
    layers.push_back({{}, 0, atlasSize});

    for (int i = 0; i < squares.size(); ++i)
    {
      int size = squares[i];

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
            layer.CreateShelf(size);
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
          layers.back().CreateShelf(size);
          Vec2 coord = layers.back().Shelves.back().Place(size);
          packed[i]  = {coord, (int) layers.size() - 1};
        }
      }
    }

    return packed;
  }

  bool BinPack2D::Shelf::Fits(int size)
  {
    return size <= CurrentWidth;
  }

  Vec2 BinPack2D::Shelf::Place(int size)
  {
    // TODO remove assert, only debug purpose
    assert(size <= CurrentWidth);

    Vec2 rectCoord = Coord;
    Coord.x += size;
    CurrentWidth -= size;
    
    return rectCoord;
  }

  bool BinPack2D::Layer::Fits(int size)
  {
    return size <= MaxHeight - CurrentHeight;
  }

  void BinPack2D::Layer::CreateShelf(int size)
  {
    // TODO remove assert, only debug purpose
    assert(size <= MaxHeight - CurrentHeight);

    Shelves.push_back({Vec2(CurrentHeight, 0.0f), size, MaxHeight});
    CurrentHeight += size;
  }
} // namespace ToolKit
