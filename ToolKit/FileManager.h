#pragma once

#include <filesystem>
#include <unordered_map>

#include "zip.h"
#include "unzip.h"
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include "Types.h"

namespace ToolKit
{
  class TK_API FileManager
  {
   public:
    XmlFile GetXmlFile(const String& path);
    uint8* GetImageFile
    (
      const String& path,
      int* x,
      int* y,
      int* comp,
      int reqComp
    );
    void PackResources(const String& path);

   private:
    void LoadAllScenes(const String& path);
    void GetAllUsedResourcePaths(const String& path);

    bool ZipPack();
    bool AddFileToZip(zipFile zfile, const char* filename);

    void GetAnimationPaths(const String& path);
    void GetScenePaths(const String& path);

    String GetRelativeResourcesPath(const String& path);
    XmlFile ReadXmlFileFromZip
    (
      zipFile zfile,
      const char* relativePath,
      const char* path
    );
    uint8* ReadImageFileFromZip
    (
      zipFile zfile,
      const char* relativePath,
      const char* path,
      int* x,
      int* y,
      int* comp,
      int reqComp
    );
    XmlFile CreateXmlFileFromZip
    (
      zipFile zfile,
      const char* filename,
      unsigned int filesize
    );
    uint8* CreateImageFileFromZip
    (
      zipFile zfile,
      const char* filename,
      unsigned int filesize,
      int* x,
      int* y,
      int* comp,
      int reqComp
    );

    void GenerateOffsetTableForPakFiles(zipFile zfile);

   private:
    StringSet allPaths;
    std::unordered_map<String, ZPOS64_T> m_zipFilesOffsetTable;
    bool m_offsetTableCreated = false;

    struct _streambuf : std::streambuf
    {
      _streambuf(char* begin, char* end)
      {
        this->setg(begin, begin, end);
      }
    };
  };
}  // namespace ToolKit
