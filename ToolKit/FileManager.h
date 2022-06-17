#pragma once

#include <filesystem>

#include "zip.h"

#include "Types.h"

namespace ToolKit
{
  class TK_API FileManager
  {
   public:
    void PackResources(const String& path);

   private:
    void LoadAllScenes(const String& path);
    void GetAllUsedResourcePaths(const String& path);

    bool ZipPack();
    bool AddFileToZip(zipFile zfile, const char* filename);

    void GetAnimationPaths(const String& path);
    void GetScenePaths(const String& path);

   private:
    StringSet allPaths;
  };
}  // namespace ToolKit
