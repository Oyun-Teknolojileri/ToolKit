#pragma once

#include <filesystem>
#include <unordered_map>
#include <utility>
#include <variant>

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
    ~FileManager();

    XmlFilePtr GetXmlFile(const String& filePath);
    uint8* GetImageFile
    (
      const String& filePath,
      int* x,
      int* y,
      int* comp,
      int reqComp
    );
    void PackResources(const String& path);

    bool CheckFileFromResources(const String& path);

   private:
    typedef std::variant
      <
      XmlFilePtr,
      uint8*,
      float*
      > FileDataType;

    enum class FileType
    {
      Xml,
      ImageUint8,
      ImageFloat
    };

    struct FileInfo
    {
      String& filePath;
      int* x;
      int* y;
      int* comp;
      int reqComp;
    };

    FileDataType GetFile(FileType fileType, FileInfo& fileSettings);
    void LoadAllScenes(const String& path);
    void GetAllUsedResourcePaths(const String& path);

    bool ZipPack(const String& zipName);
    bool AddFileToZip(zipFile zfile, const char* filename);

    void GetAnimationPaths(const String& path);
    void GetScenePaths(const String& path);
    void GetExtraFilePaths(const String& path);

    void GetRelativeResourcesPath(String& path);
    XmlFilePtr ReadXmlFileFromZip
    (
      zipFile zfile,
      const String& relativePath,
      const char* path
    );
    uint8* ReadImageFileFromZip
    (
      zipFile zfile,
      const String& relativePath,
      FileInfo& fileSettings
    );
    XmlFilePtr CreateXmlFileFromZip
    (
      zipFile zfile,
      const String& filename,
      uint filesize
    );
    uint8* CreateImageFileFromZip
    (
      zipFile zfile,
      uint filesize,
      FileInfo& fileSettings
    );

    void GenerateOffsetTableForPakFiles();
    bool IsFileInPak(const String& filename);

   private:
    StringSet m_allPaths;
    std::unordered_map<String, std::pair<ZPOS64_T, uint32_t>>
    m_zipFilesOffsetTable;
    bool m_offsetTableCreated = false;
    zipFile m_zfile = nullptr;

    struct _streambuf : std::streambuf
    {
      _streambuf(char* begin, char* end)
      {
        this->setg(begin, begin, end);
      }
    };
  };
}  // namespace ToolKit
