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

#include "FileManager.h"

#include <Animation.h>
#include <Material.h>
#include <RenderSystem.h>
#include <TKOpenGL.h>
#include <Texture.h>
#include <ToolKit.h>
#include <Util.h>
#include "Common/Win32Utils.h"
#include "TKSocket.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <mutex>
#include <thread>

#include "DebugNew.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

extern "C" unsigned char* stbi_load(const char* filename, int* x, int* y, int* comp, int req_comp);

#include <SDL.h>

namespace ToolKit
{
  static String activeProjectName;
  static String workspacePath    ;

  std::vector<String> messages;

  enum class PublishPlatform
  {
    Web,
    Windows,
    Linux,
    Android
  };

  enum Oriantation
  {
    Undefined,
    Landscape,
    Portrait
  };

  class Packer
  {
   public:
    int Publish();
    int WindowsPublish();
    int WebPublish();
    int AndroidPublish();

    void AndroidPrepareIcon();
    void EditAndroidManifest();
    void AndroidRunOnPhone();
    int PackResources();

   public:
    TexturePtr m_icon = nullptr;
    String m_appName {};
    int m_minSdk            = 27;
    int m_maxSdk            = 32;
    bool m_deployAfterBuild = false;
    bool m_isDebugBuild     = false;

    Oriantation m_oriantation;
    PublishPlatform m_platform = PublishPlatform::Windows;
  };

  int RunPipe(const String& command) 
  {
#ifdef _WIN32
    FILE* fp = _popen(command.c_str(), "r");
#else
    FILE* fp = popen(command.c_str(), "r");
#endif
    if (fp == nullptr) 
    {
      TK_LOG("pipe run failed! command: %s", command.c_str());
      return 1;
    }
    char path[512] {};
    while (fgets(path, sizeof(path), fp) != NULL)
    {
      TK_LOG("%s", path);
    }
    return fclose(fp);
  }

  int Packer::PackResources()
  {
    String projectName = activeProjectName;
    if (projectName.empty())
    {
      GetLogger()->WriteConsole(LogType::Error, "No project is loaded!");
      return 0;
    }

    String sceneResourcesPath = ConcatPaths({ResourcePath(), "Scenes"});

    return GetFileManager()->PackResources(sceneResourcesPath);
  }

  int Packer::Publish()
  {
    if (!PackResources())
    {
      return 1;
    }

    switch (m_platform)
    {
    case ToolKit::PublishPlatform::Web:
      return WebPublish();
      break;
    case ToolKit::PublishPlatform::Windows:
      return WindowsPublish();
      break;
    case ToolKit::PublishPlatform::Android:
      return AndroidPublish();
    default:
      TK_ERR("unknown publish platform: %i", (int) m_platform);
      return 0;
    }
  }

  int Packer::WindowsPublish()
  {
    TK_LOG("Building for Windows");
    Path workDir = std::filesystem::current_path();

    std::error_code ec;
    auto returnLoggingError = [&ec, &workDir](String path = "", bool setPathBack = false) -> bool
    {
      bool ret     = false;
      bool setback = setPathBack;
      if (ec)
      {
        TK_ERR("%s %s", ec.message().c_str(), path.c_str());
        setback = true;
        ret     = true;
      }

      if (setback)
      {
        std::filesystem::current_path(workDir, ec);
        if (ec)
        {
          TK_ERR("%s", ec.message().c_str(), path.c_str());
          TK_ERR("%s", "******** PLEASE RESTART THE EDITOR ********");
        }

        ret = true;
      }

      return ret;
    };

    // Move files to publish directory
    const String projectName      = activeProjectName;
    const String publishDirectory = ConcatPaths({ResourcePath(), "..", "Publish", "Windows"});
    const String publishBinDir    = ConcatPaths({publishDirectory, "Bin"});
    const String publishConfigDir = ConcatPaths({publishDirectory, "Config"});

    // Create directories
    std::filesystem::create_directories(publishDirectory, ec);
    if (returnLoggingError(publishDirectory))
    {
      return 1;
    }
    std::filesystem::create_directories(publishBinDir, ec);
    if (returnLoggingError(publishBinDir))
    {
      return 1;
    }
    std::filesystem::create_directories(publishConfigDir, ec);
    if (returnLoggingError(publishConfigDir))
    {
      return 1;
    }

    TK_LOG("Run toolkit compile script");
    Path newWorkDir(ConcatPaths({"..", "BuildScripts"}));
    std::filesystem::current_path(newWorkDir);
    int toolKitCompileResult = RunPipe("WinBuildRelease.bat");
    if (toolKitCompileResult != 0)
    {
      returnLoggingError("WinBuildRelease", true);
      TK_ERR("ToolKit could not be compiled");
      return 1;
    }

    // Run plugin compile script
    newWorkDir = Path(ConcatPaths({ResourcePath(), "..", "Windows"}));
    std::filesystem::current_path(newWorkDir, ec);
    if (returnLoggingError(newWorkDir.string()))
    {
      return 1;
    }

    int pluginCompileResult = RunPipe("WinBuildRelease.bat");
    if (pluginCompileResult != 0)
    {
      returnLoggingError("WinBuildRelease.bat", true);
      TK_ERR("Windows build has failed!");
      return 1;
    }
    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError(workDir.string()))
    {
      return 1;
    }

    std::filesystem::create_directories(ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}));

    const String exeFile =
        ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}) + GetPathSeparatorAsStr() + projectName + ".exe";

    const String pakFile                = ConcatPaths({ResourcePath(), "..", "MinResources.pak"});
    const String sdlDllPath             = ConcatPaths({workDir.string(), "SDL2.dll"});
    const String configDirectory        = ConcatPaths({ResourcePath(), "..", "Config"});
    const String engineSettingsPath     = ConcatPaths({ConfigPath(), "Engine.settings"});
    const String destEngineSettingsPath = ConcatPaths({publishConfigDir, "Engine.settings"});

    TK_LOG("windows build done moving files");

    // Copy exe file
    std::filesystem::copy(exeFile, publishBinDir, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError(publishBinDir))
    {
      return 1;
    }

    // Copy SDL2.dll from ToolKit bin folder to publish bin folder
    std::filesystem::copy(sdlDllPath, publishBinDir, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError(publishBinDir))
    {
      return 1;
    }

    // Copy pak
    std::filesystem::copy(pakFile, publishDirectory, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError(publishDirectory))
    {
      return 1;
    }

    // Copy engine settings to config folder
    std::filesystem::copy(engineSettingsPath,
                          destEngineSettingsPath,
                          std::filesystem::copy_options::overwrite_existing,
                          ec);
    if (returnLoggingError(engineSettingsPath))
    {
      return 1;
    }

    // Tell user about where the location of output files is
    GetLogger()->WriteConsole(LogType::Success, "Building for WINDOWS has been completed successfully.");
    GetLogger()->WriteConsole(LogType::Memo,
                              "Output files location: %s",
                              std::filesystem::absolute(publishDirectory).string().c_str());

    PlatformHelpers::OpenExplorer(publishDirectory);
    return 0;
  }

  void Packer::AndroidPrepareIcon()
  {
    String assetsPath = NormalizePathInplace("Android/app/src/main/res");

    String projectName = activeProjectName;
    String resLocation = ConcatPaths({workspacePath, projectName, assetsPath});
    if (m_icon == nullptr)
    {
      return;
    }
    TK_LOG("Preparing Icons");
    int refWidth, refHeight, refComp;
    unsigned char* refImage = stbi_load(m_icon->GetFile().c_str(), &refWidth, &refHeight, &refComp, 0);

    // search each folder in res folder and find icons, replace that icons with new ones
    for (const auto& entry : std::filesystem::directory_iterator(resLocation))
    {
      if (!entry.is_directory())
      {
        continue;
      }

      for (auto& file : std::filesystem::directory_iterator(entry))
      {
        String name;
        String extension;
        String path = file.path().string();
        DecomposePath(path, nullptr, &name, &extension);
        // if this is image replace with new one, don't touch if this is background image
        if (name.find("background") == std::string::npos && extension == ".png")
        {
          int width, height, comp;
          // get the image that we want to replace
          unsigned char* img = stbi_load(path.c_str(), &width, &height, &comp, 0);
          assert(img && "cannot load android icon");
          int res;
          res = stbir_resize_uint8(refImage, refWidth, refHeight, 0, img, width, height, 0, comp);
          assert(res && "cannot resize android icon");
          // write resized image
          res = stbi_write_png(path.c_str(), width, height, comp, img, 0);
          assert(res && "cannot write to android icon");
          free(img);
        }
      }
    }
    free(refImage);
  }

  void Packer::AndroidRunOnPhone()
  {
    // adb path is in: 'android-sdk/platform-tools'
    String sdkPath = String(getenv("ANDROID_HOME"));
    if (sdkPath.empty())
    {
      TK_WRN("ANDROID_HOME environment variable is not set.");
      return;
    }
    Path oldPath = std::filesystem::current_path();
    std::filesystem::current_path(ConcatPaths({sdkPath, "platform-tools"}));
    TK_LOG("Trying to execute the app on your phone...");

    const auto checkIfFailedFn = [oldPath](int execResult, const String& command) -> bool
    {
      if (execResult == 1)
      {
        TK_LOG("%s command failed! exec result: %i", command.c_str(), execResult);
        TK_WRN("Make sure that an android device is connected to your PC");
        TK_WRN("if still doesn't work uninstall application and rebuild.");
        std::filesystem::current_path(oldPath);
        return true;
      }
      return false;
    };

    String apkPath = NormalizePathInplace("Android\\app\\build\\outputs\\apk");
    apkPath            = ConcatPaths({apkPath, m_isDebugBuild ? "debug" : "release"});
    apkPath            = ConcatPaths({apkPath, m_isDebugBuild ? "app-debug.apk" : "app-release-unsigned.apk"});

    String projectName = activeProjectName;
    String apkLocation = ConcatPaths({workspacePath, projectName, apkPath});
    String packageName = "com.otyazilim.toolkit/com.otyazilim.toolkit.MainActivity"; // adb uses / forward slash

    int execResult;
    execResult = PlatformHelpers::SysComExec("adb install " + apkLocation, false, true, nullptr);
    if (checkIfFailedFn(execResult, "adb install " + apkLocation))
    {
      return;
    }

    execResult = PlatformHelpers::SysComExec("adb shell am start -n " + packageName, true, true, nullptr);
    if (checkIfFailedFn(execResult, "adb shell am start -n " + packageName))
    {
      return;
    }
    std::filesystem::current_path(oldPath);
  }

  void Packer::EditAndroidManifest()
  {
    TK_LOG("Editing Android Manifest");
    String projectName     = activeProjectName;
    String applicationName = m_appName;
    String mainPath        = NormalizePathInplace("Android/app/src/main");
    // get manifest file from template
    String androidManifest = GetFileManager()->ReadAllText(
        std::filesystem::absolute(ConcatPaths({"..", "Template", mainPath, "AndroidManifest.xml"})).string());
    // replace template values with our settings
    ReplaceFirstStringInPlace(androidManifest, "@string/app_name", applicationName);
    ReplaceFirstStringInPlace(androidManifest,
                              "minSdkVersion=\"26\"",
                              "minSdkVersion=\"" + std::to_string(m_minSdk) + "\"");
    ReplaceFirstStringInPlace(androidManifest,
                              "maxSdkVersion=\"33\"",
                              "maxSdkVersion=\"" + std::to_string(m_maxSdk) + "\"");
    // 0 undefined 1 landscape 2 Portrait
    String oriantationNames[3] {"fullSensor", "landscape", "portrait"};
    ReplaceFirstStringInPlace(androidManifest,
                              "screenOrientation=\"portrait\"",
                              "screenOrientation=\"" + oriantationNames[(int) m_oriantation] + "\"");

    String mainLocation = ConcatPaths({workspacePath, projectName, mainPath});
    String manifestLoc  = ConcatPaths({mainLocation, "AndroidManifest.xml"});

    GetFileManager()->WriteAllText(manifestLoc, androidManifest);
  }

  int Packer::AndroidPublish()
  {
    TK_LOG("Building for android");
    Path workDir = std::filesystem::current_path();

    std::error_code ec;
    auto returnLoggingError = [&ec, &workDir](const String& path = " ", bool setPathBack = false) -> bool
    {
      bool ret     = false;
      bool setback = setPathBack;
      if (ec)
      {
        TK_ERR("%s path: %s", ec.message().c_str(), path.c_str());
        setback = true;
        ret     = true;
      }

      if (setback)
      {
        std::filesystem::current_path(workDir, ec);
        if (ec)
        {
          TK_ERR("%s", ec.message().c_str());
          TK_ERR("%s", "******** PLEASE RESTART THE EDITOR ********");
        }

        ret = true;
      }

      return ret;
    };

    String projectName = activeProjectName;

    if (projectName.empty())
    {
      GetLogger()->WriteConsole(LogType::Error, "No project is loaded!");
      return 1;
    }

    String assetsPath = NormalizePathInplace("Android/app/src/main/assets");
    String projectLocation                         = ConcatPaths({workspacePath, projectName});
    String sceneResourcesPath                      = ConcatPaths({projectLocation, "MinResources.pak"});
    String androidResourcesPath                    = ConcatPaths({projectLocation, assetsPath, "MinResources.pak"});

    const std::filesystem::copy_options copyOption = std::filesystem::copy_options::overwrite_existing;

    std::filesystem::copy(sceneResourcesPath, androidResourcesPath, copyOption, ec);
    if (returnLoggingError())
    {
      return 1;
    }

    EditAndroidManifest();

    std::filesystem::current_path(ConcatPaths({projectLocation, "Android"}), ec);
    if (returnLoggingError())
    {
      return 1;
    }

    AndroidPrepareIcon();

    TK_LOG("Building android apk, Gradle scripts running...");

    // use "gradlew bundle" command to build .aab project or use "gradlew assemble" to release build
    String command    = m_isDebugBuild ? "gradlew assembleDebug" : "gradlew assemble";
    
    int compileResult = RunPipe(command);
    
    {
      if (compileResult == 1)
      {
        GetLogger()->WriteConsole(LogType::Error, "Android build failed.");
        return 1;
      }

      String buildLocation = NormalizePathInplace(ConcatPaths({projectLocation, "Android/app/build/outputs/apk"}));
      buildLocation        = ConcatPaths({buildLocation, m_isDebugBuild ? "debug" : "release"});
      const String publishDirStr   = ConcatPaths({ResourcePath(), "..", "Publish", "Android"});
      const String apkName         = m_isDebugBuild ? "app-debug.apk" : "app-release-unsigned.apk";
      const String apkPathStr      = ConcatPaths({buildLocation, apkName});

      projectName                  = m_appName;
      projectName                 += m_isDebugBuild ? "_debug.apk" : "_release.apk";
      const String publishApkPath  = ConcatPaths({publishDirStr, projectName});

      // Create directories
      std::filesystem::create_directories(publishDirStr, ec);
      if (returnLoggingError())
      {
        return 1;
      }

      std::filesystem::copy(apkPathStr, publishApkPath, std::filesystem::copy_options::overwrite_existing, ec);
      if (returnLoggingError())
      {
        return 1;
      }

      // Tell user about where the location of output files is
      GetLogger()->WriteConsole(LogType::Success, "Building for ANDROID has been completed successfully.");
      GetLogger()->WriteConsole(LogType::Memo,
                                "Output files location: %s",
                                std::filesystem::absolute(publishDirStr).string().c_str());

      PlatformHelpers::OpenExplorer(publishDirStr);
    }

    if (compileResult != 0)
    {
      returnLoggingError("Compiling Fail", true);
      TK_ERR("Compiling failed.");
      return 1;
    }

    std::filesystem::current_path(workDir, ec); // set work directory back

    if (m_deployAfterBuild)
    {
      AndroidRunOnPhone();
    }

    if (returnLoggingError())
    {
      return 1;
    }
    return 0;
  }

  int Packer::WebPublish()
  {
    TK_LOG("Building for Web");
    Path workDir = std::filesystem::current_path();

    std::error_code ec;
    auto returnLoggingError = [&ec, &workDir](const String& path, bool setPathBack = false, int line = 0) -> bool
    {
      bool ret     = false;
      bool setback = setPathBack;
      if (ec)
      {
        TK_ERR("line %i; %s %s", line, ec.message().c_str(), path.c_str());
        setback = true;
        ret     = true;
      }

      if (setback)
      {
        std::filesystem::current_path(workDir, ec);
        if (ec)
        {
          TK_ERR("line %i; %s %s", line, ec.message().c_str(), path.c_str());
          TK_ERR("%s", "******** PLEASE RESTART THE EDITOR ********");
        }

        ret = true;
      }

      return ret;
    };

    // Run toolkit compile script
    Path newWorkDir(ConcatPaths({"..", "BuildScripts"}));
    std::filesystem::current_path(newWorkDir, ec);
    if (returnLoggingError(newWorkDir.string(), false, __LINE__))
    {
      return 1;
    }

    int toolKitCompileResult = RunPipe("WebBuildRelease.bat"); // PlatformHelpers::SysComExec("WebBuildRelease.bat", false, true, nullptr);
    if (toolKitCompileResult != 0)
    {
      returnLoggingError("bat failed", true, __LINE__);
      TK_ERR("ToolKit could not be compiled!");
      return 1;
    }
    newWorkDir                               = Path(ConcatPaths({ResourcePath(), "..", "Web"}));

    const String pluginWebBuildScriptsFolder = ConcatPaths({ResourcePath(), "..", "Web", "WebBuildRelease.bat"});

    // Run scripts
    std::filesystem::current_path(newWorkDir, ec);
    if (returnLoggingError(newWorkDir.string(), false, __LINE__))
    {
      return 1;
    }

    int pluginCompileResult = RunPipe(pluginWebBuildScriptsFolder);
    
    if (pluginCompileResult != 0)
    {
      returnLoggingError(pluginWebBuildScriptsFolder, true, __LINE__);
      TK_ERR("Web build has failed!");
      return 1;
    }
    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError(workDir.string(), false, __LINE__))
    {
      return 1;
    }

    // Move files to a directory
    String projectName           = activeProjectName;
    String publishDirectory      = ConcatPaths({ResourcePath(), "..", "Publish", "Web"});
    String firstPart             = ConcatPaths({ResourcePath(), "..", "Codes", "Bin", projectName}) + ".";
    String files[] = {firstPart + "data", firstPart + "html", firstPart + "js", firstPart + "wasm"};
    
    std::filesystem::create_directories(publishDirectory, ec);
    if (returnLoggingError(publishDirectory, false, __LINE__))
    {
      return 1;
    }

    for (int i = 0; i < ArraySize(files); i++)
    {
      std::filesystem::copy(files[i].c_str(), publishDirectory, std::filesystem::copy_options::overwrite_existing, ec);
      if (returnLoggingError(files[i], false, __LINE__))
      {
        return 1;
      }
    }

    // Create run script
    std::ofstream runBatchFile(ConcatPaths({publishDirectory, "Run.bat"}).c_str());
    runBatchFile << "emrun ./" + projectName + ".html";
    runBatchFile.close();

    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError(workDir.string(), false, __LINE__))
    {
      return 1;
    }

    // Output user about where are the output files
    GetLogger()->WriteConsole(LogType::Success, "Building for web has been completed successfully.");
    GetLogger()->WriteConsole(LogType::Memo,
                              "Output files location: %s",
                              std::filesystem::absolute(publishDirectory).string().c_str());

    PlatformHelpers::OpenExplorer(publishDirectory);
    return 0;
  }

  static bool finished   = false;
  static bool canPublish = false;

  Packer packer {};

  std::mutex messageMutex;

  static DWORD CreatePipe(void*)
  {
    SOCKET ConnectSocket    = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    String sendbuf = "a Hello from Packer";
    int iResult;

    // Initialize Winsock
    iResult = sock_init();
    if (iResult != 0)
    {
      sock_perror("socket init failed with error");
      return 1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult           = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
      sock_perror("getaddrinfo failed with error");
      sock_cleanup();
      return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
      // Create a SOCKET for connecting to server
      ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
      if (ConnectSocket == INVALID_SOCKET)
      {
        sock_perror("socket failed with error");
        sock_cleanup();
        return 1;
      }

      // Connect to server.
      iResult = connect(ConnectSocket, ptr->ai_addr, (int) ptr->ai_addrlen);
      if (iResult == SOCKET_ERROR)
      {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        continue;
      }
      break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
      perror("Unable to connect to server!\n");
      sock_cleanup();
      return 1;
    }

    uint64_t mask = 0;
    recv(ConnectSocket, (char*) &mask, 8, 0);
    // Get platform arguments that toolkit sended
    packer.m_deployAfterBuild = (mask >> 0) & 0xff;
    packer.m_isDebugBuild     = (mask >> 8) & 0xff;
    packer.m_minSdk           = (mask >> 16) & 0xff;
    packer.m_maxSdk           = (mask >> 24) & 0xff;
    packer.m_platform         = (PublishPlatform) ((mask >> 32) & 0xff);
    packer.m_oriantation      = (Oriantation) ((mask >> 40) & 0xff);
    canPublish                   = true;

    TK_LOG("Packing Resources...");

    // Receive until the peer closes the connection
    while (!finished || !messages.empty())
    {
      while (messages.empty())
      {
        std::this_thread::yield();
        if (finished) 
        {
          goto end;
        }
      }

      messageMutex.lock();
      sendbuf = messages.front();
      pop_front(messages);
      messageMutex.unlock();

      iResult = send(ConnectSocket, sendbuf.c_str(), (int)sendbuf.size(), 0);
      if (iResult == SOCKET_ERROR)
      {
        perror("client message send failed with error");
        closesocket(ConnectSocket);
        sock_cleanup();
        return 1;
      }
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(1ms);
    }
  end:
  {
  }
    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);

    if (iResult == SOCKET_ERROR)
    {
      sock_perror("client shutdown failed with error");
      closesocket(ConnectSocket);
      sock_cleanup();
      return 1;
    }
    // cleanup
    closesocket(ConnectSocket);
    sock_cleanup();
    return 0;
  }

  int ToolKitMain(int argc, char* argv[])
  {
    // Initialize ToolKit to serialize resources
    Main* g_proxy = new Main();
    Main::SetProxy(g_proxy);

    String publishArguments = GetFileManager()->ReadAllText("PublishArguments.txt");
    StringArray arguments;
    Split(publishArguments, "\n", arguments);

    activeProjectName      = arguments[0];
    workspacePath          = arguments[1];
    packer.m_appName    = arguments[2];

    const auto whitePredFn = [](char c) { return std::isspace(c); };
    erase_if(activeProjectName, whitePredFn);
    erase_if(workspacePath, whitePredFn);
    erase_if(packer.m_appName, whitePredFn);

    // Set resource root to project's Resources folder
    g_proxy->m_resourceRoot = ConcatPaths({workspacePath, activeProjectName, "Resources"});

    g_proxy->SetConfigPath(ConcatPaths({"..", "Config"}));
    g_proxy->PreInit();

    GetLogger()->SetWriteConsoleFn(
        [](LogType lt, String ms) -> void
        {
          ms.insert(ms.begin(), 'a' + (char) lt);
          messageMutex.lock();
          messages.push_back(ms);
          messageMutex.unlock();
        });

    // Init SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_Window* g_window    = SDL_CreateWindow("temp",
                                            SDL_WINDOWPOS_UNDEFINED,
                                            SDL_WINDOWPOS_UNDEFINED,
                                            32,
                                            32,
                                            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    SDL_GLContext g_context = SDL_GL_CreateContext(g_window);

    g_proxy->m_renderSys->InitGl(SDL_GL_GetProcAddress, nullptr);

    g_proxy->Init();

    std::thread hThread(CreatePipe, nullptr);

    // wait untill packer to connect toolkit server
    while (!canPublish)
    {
      std::this_thread::yield();
    }

    int result = packer.Publish();
    finished = true;
    SDL_GL_DeleteContext(g_context);
    SDL_DestroyWindow(g_window);

    // wait untill all messages writen to the toolkit console
    while (messages.size())
    {
      std::this_thread::yield();
    }
    hThread.detach();
    getchar();
    return 0;
  }
} // namespace ToolKit

int main(int argc, char* argv[]) { return ToolKit::ToolKitMain(argc, argv); }