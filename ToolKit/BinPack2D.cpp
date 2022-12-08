#include "BinPack2D.h"

namespace ToolKit
{
  std::vector<BinPack2D::PackedRect> BinPack2D::Pack(std::vector<int> squares,
                                                     int atlasSize)
  {
    std::vector<PackedRect> packed;
    packed.resize(squares.size());

    std::vector<Shelf> shelves;

    int currentHeight = atlasSize;
    int atlasIndex    = 0;

    for (int i = 0; i < squares.size(); ++i)
    {
      int square = squares[i];
      if (shelves.empty() || currentHeight < ) // TODO
    }

    return packed;
  }
} // namespace ToolKit
