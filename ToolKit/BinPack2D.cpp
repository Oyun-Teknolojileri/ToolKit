/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "BinPack2D.h"

#include "ToolKit.h"

namespace ToolKit
{

  BinPack2D::PackedRectArray BinPack2D::Pack(const IntArray& squares, int atlasSize, int* layerCount)
  {
    PackedRectArray packed;
    packed.reserve(squares.size());

    if (squares.empty())
    {
      return packed;
    }

    PackedRect lastRect;
    lastRect.coordinate = Vec2(0.0);
    lastRect.layer      = 0;

    // Place the first at the beginning.
    int mapSize         = squares[0];

    auto sizeFailureFn  = [&packed, atlasSize](int mapSize) -> PackedRectArray
    {
      TK_LOG("Map can't fit into atlas. Atlas size %d < Map size %d", atlasSize, mapSize);
      return packed;
    };

    if (mapSize <= atlasSize)
    {
      packed.push_back(lastRect);
    }
    else
    {
      return sizeFailureFn(mapSize);
    }

    for (size_t i = 1; i < squares.size(); i++)
    {
      mapSize = squares[i];
      if (mapSize <= atlasSize)
      {
        int x = (int) lastRect.coordinate.x + mapSize;
        if (x < atlasSize)
        {
          lastRect.coordinate.x = (float) x;
          packed.push_back(lastRect);
        }
        else
        {
          int y = (int) lastRect.coordinate.y + mapSize;
          if (y < atlasSize)
          {
            lastRect.coordinate = Vec2(0.0f, y);
            packed.push_back(lastRect);
          }
          else
          {
            lastRect.coordinate  = Vec2(0.0);
            lastRect.layer      += 1;
            packed.push_back(lastRect);
          }
        }
      }
      else
      {
        return sizeFailureFn(mapSize);
      }
    }

    if (layerCount != nullptr)
    {
      *layerCount = lastRect.layer + 1;
    }

    return packed;
  }

} // namespace ToolKit
