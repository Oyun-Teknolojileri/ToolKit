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

#ifdef _WIN32
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <shellapi.h>
  #include <strsafe.h>

  #include <chrono>
  #include <mutex>
  #include <thread>

  // ToolKit collisions
  #undef WriteConsole

  #define _WINSOCK_DEPRECATED_NO_WARNINGS
  #include <WinSock2.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <ws2tcpip.h>

  #define DEFAULT_BUFLEN 512
  #define DEFAULT_PORT   "27015"
  #pragma comment(lib, "ws2_32.lib")

#endif

#include "FileManager.h"

#include <Animation.h>
#include <Texture.h>
#include <ToolKit.h>
#include <Util.h>
#include <assert.h>

#include "DebugNew.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

namespace ToolKit
{
  static String activeProjectName = "OsmanTest";
  static String workspacePath     = "C:\\Users\\Administrator\\source\\repos\\Go2";

  std::vector<String> messages;

  // Function to convert UTF-8 to UTF-16
  static std::wstring ConvertUTF8ToUTF16(const std::string& utf8String)
  {
    // Calculate the length of the UTF-16 string
    int utf16Length = MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, NULL, 0);

    if (utf16Length == 0)
    {
      throw std::runtime_error("Error calculating UTF-16 string length");
    }
    // Allocate memory for the UTF-16 string

    wchar_t* utf16String = new wchar_t[utf16Length];
    // Convert UTF-8 to UTF-16
    if (MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, utf16String, utf16Length) == 0)
    {
      delete[] utf16String;
      throw std::runtime_error("Error converting UTF-8 to UTF-16");
    }
    // Create a wstring from the UTF-16 string
    std::wstring result(utf16String);
    // Clean up
    delete[] utf16String;
    return result;
  }

  // Win32 console command execution callback.
  static int SysComExec(
      StringView cmd,
      bool async,
      bool showConsole,
      std::function<void(int)> callback = [](int x) {})
  {
    // https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb             = sizeof(si);
    si.dwFlags        = STARTF_USESHOWWINDOW;
    si.wShowWindow    = showConsole ? SW_SHOWNORMAL : SW_HIDE;

    std::wstring wCmd = ConvertUTF8ToUTF16("cmd /C ") + ConvertUTF8ToUTF16(cmd.data());

    // Start the child process.
    if (!CreateProcessW(NULL, wCmd.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
      DWORD errCode = GetLastError();
      GetLogger()->WriteConsole(LogType::Error, "CreateProcess failed (%d).\n", errCode);
      return (int) errCode;
    }

    SetForegroundWindow((HWND) pi.hProcess);

    auto finalizeFn = [pi, callback](DWORD stat) -> int
    {
      // Close process and thread handles.
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      if (callback != nullptr)
      {
        callback((int) stat);
      }
      return stat;
    };

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD stat = 0;
    GetExitCodeProcess(pi.hProcess, &stat);
    return finalizeFn(stat);
  }

  static void OpenExplorer(const StringView utf8Path)
  {
    std::wstring utf16Path = ConvertUTF8ToUTF16(utf8Path.data());
    HINSTANCE result       = ShellExecuteW(nullptr, L"open", L"explorer.exe", utf16Path.data(), NULL, SW_SHOWNORMAL);

    // Check the result of ShellExecute
    if ((intptr_t) result <= 32)
    {
      // ShellExecute failed
      TK_ERR("Failed to open the folder: %s", utf8Path);
    }
  }

  enum class PublishPlatform
  {
    Web,
    Windows,
    Linux,
    Android
  };

  class Publisher
  {
   public:
    virtual void Publish() const = 0;
  };

  class WebPublisher : Publisher
  {
   public:
    void Publish() const override;
  };

  class AndroidPublisher : Publisher
  {
   public:
    void Publish() const override;
    void PrepareIcon() const;
    void EditAndroidManifest() const;
    void RunOnPhone() const;

   public:
    TexturePtr m_icon = nullptr;
    String m_appName {};
    int m_minSdk            = 27;
    int m_maxSdk            = 32;
    bool m_deployAfterBuild = false;
    bool m_isDebugBuild     = false;

    enum Oriantation
    {
      Undefined,
      Landscape,
      Portrait
    };

    Oriantation m_oriantation;
  };

  class WindowsPublisher : Publisher
  {
   public:
    void Publish() const override;
  };

  class PublishManager
  {
   public:
    PublishManager();
    ~PublishManager();

    void Publish(PublishPlatform platform);

    WebPublisher* m_webPublisher         = nullptr;
    AndroidPublisher* m_androidPublisher = nullptr;
    WindowsPublisher* m_windowsPublisher = nullptr;
  };

  PublishManager::PublishManager()
  {
    m_webPublisher     = new WebPublisher();
    m_windowsPublisher = new WindowsPublisher();
    m_androidPublisher = new AndroidPublisher();
  }

  PublishManager::~PublishManager()
  {
    SafeDel(m_webPublisher);
    SafeDel(m_windowsPublisher);
    SafeDel(m_androidPublisher);
  }

  void PackResources()
  {
    String projectName = activeProjectName;
    if (projectName.empty())
    {
      GetLogger()->WriteConsole(LogType::Error, "No project is loaded!");
      return;
    }

    String sceneResourcesPath = ConcatPaths({workspacePath, projectName, "Resources", "Scenes"});

    GetFileManager()->PackResources(sceneResourcesPath);
  }

  void PublishManager::Publish(PublishPlatform platform)
  {
    PackResources();

    if (platform == PublishPlatform::Web)
    {
      m_webPublisher->Publish();
    }

    if (platform == PublishPlatform::Android)
    {
      m_androidPublisher->Publish();
    }

    if (platform == PublishPlatform::Windows)
    {
      m_windowsPublisher->Publish();
    }
  }

  void WindowsPublisher::Publish() const
  {
    Path workDir = std::filesystem::current_path();

    std::error_code ec;
    auto returnLoggingError = [&ec, &workDir](bool setPathBack = false) -> bool
    {
      bool ret     = false;
      bool setback = setPathBack;
      if (ec)
      {
        TK_ERR("%s", ec.message().c_str());
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

    // Run toolkit compile script
    Path newWorkDir(ConcatPaths({"..", "BuildScripts"}));
    std::filesystem::current_path(newWorkDir);
    int toolKitCompileResult = SysComExec("WinBuildRelease.bat", false, true);
    if (toolKitCompileResult != 0)
    {
      returnLoggingError(true);
      TK_ERR("ToolKit could not be compiled");
      return;
    }

    // Run plugin compile script
    newWorkDir = Path(ConcatPaths({ResourcePath(), "..", "Windows"}));
    std::filesystem::current_path(newWorkDir, ec);
    if (returnLoggingError())
    {
      return;
    }

    int pluginCompileResult = SysComExec("WinBuildRelease.bat", false, true);
    if (pluginCompileResult != 0)
    {
      returnLoggingError(true);
      TK_ERR("Windows build has failed!");
      return;
    }
    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError())
    {
      return;
    }

    // Move files to publish directory
    const String projectName      = activeProjectName;
    const String publishDirectory = ConcatPaths({ResourcePath(), "..", "Publish", "Windows"});
    const String publishBinDir    = ConcatPaths({publishDirectory, "Bin"});
    const String publishConfigDir = ConcatPaths({publishDirectory, "Config"});

    const String exeFile =
        ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}) + GetPathSeparatorAsStr() + projectName + ".exe";

    const String pakFile                = ConcatPaths({ResourcePath(), "..", "MinResources.pak"});
    const String sdlDllPath             = ConcatPaths({workDir.string(), "SDL2.dll"});
    const String configDirectory        = ConcatPaths({ResourcePath(), "..", "Config"});
    const String engineSettingsPath     = ConcatPaths({ConfigPath(), "Engine.settings"});
    const String destEngineSettingsPath = ConcatPaths({publishConfigDir, "Engine.settings"});

    // Create directories
    if (!std::filesystem::exists(publishDirectory))
    {
      std::filesystem::create_directories(publishDirectory, ec);
      if (returnLoggingError())
      {
        return;
      }
    }
    if (!std::filesystem::exists(publishBinDir))
    {
      std::filesystem::create_directories(publishBinDir, ec);
      if (returnLoggingError())
      {
        return;
      }
    }
    if (!std::filesystem::exists(publishConfigDir))
    {
      std::filesystem::create_directories(publishConfigDir, ec);
      if (returnLoggingError())
      {
        return;
      }
    }

    // Copy exe file
    std::filesystem::copy(exeFile.c_str(), publishBinDir, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError())
    {
      return;
    }

    // Copy SDL2.dll from ToolKit bin folder to publish bin folder
    std::filesystem::copy(sdlDllPath.c_str(), publishBinDir, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError())
    {
      return;
    }

    // Copy pak
    std::filesystem::copy(pakFile.c_str(), publishDirectory, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError())
    {
      return;
    }

    // Copy engine settings to config folder
    std::filesystem::copy(engineSettingsPath.c_str(),
                          destEngineSettingsPath.c_str(),
                          std::filesystem::copy_options::overwrite_existing,
                          ec);
    if (returnLoggingError())
    {
      return;
    }

    // Tell user about where the location of output files is
    GetLogger()->WriteConsole(LogType::Success, "Building for WINDOWS has been completed successfully.");
    GetLogger()->WriteConsole(LogType::Memo,
                              "Output files location: %s",
                              std::filesystem::absolute(publishDirectory).string().c_str());

    OpenExplorer(publishDirectory);
  }

  void AndroidPublisher::PrepareIcon() const
  {
    String assetsPath = "Android/app/src/main/res";
    NormalizePath(assetsPath);

    String projectName = activeProjectName;
    String resLocation = ConcatPaths({workspacePath, projectName, assetsPath});
    if (m_icon == nullptr)
    {
      return;
    }
    TK_LOG("Preparing Icons");
    int refWidth, refHeight, refComp;
    stbi_uc* refImage = stbi_load(m_icon->GetFile().c_str(), &refWidth, &refHeight, &refComp, 0);

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
          stbi_uc* img = stbi_load(path.c_str(), &width, &height, &comp, 0);
          assert(img && "cannot load android icon");
          int res;
          res = stbir_resize_uint8(refImage, refWidth, refHeight, 0, img, width, height, 0, comp);
          assert(res && "cannot resize android icon");
          // write resized image
          res = stbi_write_png(path.c_str(), width, height, comp, img, 0);
          assert(res && "cannot write to android icon");
          stbi_image_free(img);
        }
      }
    }
    stbi_image_free(refImage);
  }

  void AndroidPublisher::RunOnPhone() const
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

    String apkPath = "Android\\app\\build\\outputs\\apk";
    NormalizePath(apkPath);
    apkPath            = ConcatPaths({apkPath, m_isDebugBuild ? "debug" : "release"});
    apkPath            = ConcatPaths({apkPath, m_isDebugBuild ? "app-debug.apk" : "app-release-unsigned.apk"});

    String projectName = activeProjectName;
    String apkLocation = ConcatPaths({workspacePath, projectName, apkPath});
    String packageName = "com.otyazilim.toolkit/com.otyazilim.toolkit.MainActivity"; // adb uses / forward slash

    int execResult;
    execResult = SysComExec("adb install " + apkLocation, false, true, nullptr);
    if (checkIfFailedFn(execResult, "adb install " + apkLocation))
    {
      return;
    }

    execResult = SysComExec("adb shell am start -n " + packageName, true, true, nullptr);
    if (checkIfFailedFn(execResult, "adb shell am start -n " + packageName))
    {
      return;
    }
    std::filesystem::current_path(oldPath);
  }

  void AndroidPublisher::EditAndroidManifest() const
  {
    TK_LOG("Editing Android Manifest");
    String projectName     = activeProjectName;
    String applicationName = m_appName.empty() ? projectName : m_appName;
    String mainPath        = "Android/app/src/main";
    NormalizePath(mainPath);
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
                              "screenOrientation=\"" + oriantationNames[m_oriantation] + "\"");

    String mainLocation = ConcatPaths({workspacePath, projectName, mainPath});
    String manifestLoc  = ConcatPaths({mainLocation, "AndroidManifest.xml"});

    GetFileManager()->WriteAllText(manifestLoc, androidManifest);
  }

  void AndroidPublisher::Publish() const
  {
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
      return;
    }

    String assetsPath = "Android/app/src/main/assets";
    NormalizePath(assetsPath);

    String projectLocation                         = ConcatPaths({workspacePath, projectName});
    String sceneResourcesPath                      = ConcatPaths({projectLocation, "MinResources.pak"});
    String androidResourcesPath                    = ConcatPaths({projectLocation, assetsPath, "MinResources.pak"});

    const std::filesystem::copy_options copyOption = std::filesystem::copy_options::overwrite_existing;

    std::filesystem::copy(sceneResourcesPath, androidResourcesPath, copyOption, ec);
    if (returnLoggingError())
    {
      return;
    }

    EditAndroidManifest();

    std::filesystem::current_path(ConcatPaths({projectLocation, "Android"}), ec);
    if (returnLoggingError())
    {
      return;
    }

    PrepareIcon();

    const auto afterBuildFn = [&](int res) -> void
    {
      if (res == 1)
      {
        GetLogger()->WriteConsole(LogType::Error, "Android build failed.");
        return;
      }
      String buildLocation = ConcatPaths({projectLocation, "Android/app/build/outputs/apk"});
      NormalizePath(buildLocation);
      buildLocation                = ConcatPaths({buildLocation, m_isDebugBuild ? "debug" : "release"});
      const String publishDirStr   = ConcatPaths({ResourcePath(), "..", "Publish", "Android"});
      const String apkName         = m_isDebugBuild ? "app-debug.apk" : "app-release-unsigned.apk";
      const String apkPathStr      = ConcatPaths({buildLocation, apkName});

      projectName                  = !m_appName.empty() ? m_appName : projectName;
      projectName                 += m_isDebugBuild ? "_debug.apk" : "_release.apk";
      const String publishApkPath  = ConcatPaths({publishDirStr, projectName});

      // Create directories
      if (!std::filesystem::exists(publishDirStr))
      {
        bool res = std::filesystem::create_directories(publishDirStr, ec);
        if (returnLoggingError())
        {
          return;
        }
      }

      std::filesystem::copy(apkPathStr.c_str(), publishApkPath, std::filesystem::copy_options::overwrite_existing, ec);
      if (returnLoggingError())
      {
        return;
      }

      // Tell user about where the location of output files is
      GetLogger()->WriteConsole(LogType::Success, "Building for ANDROID has been completed successfully.");
      GetLogger()->WriteConsole(LogType::Memo,
                                "Output files location: %s",
                                std::filesystem::absolute(publishDirStr).string().c_str());

      OpenExplorer(publishDirStr);
    };

    TK_LOG("building android apk...");

    // use "gradlew bundle" command to build .aab project or use "gradlew assemble" to release build
    String command    = m_isDebugBuild ? "gradlew assembleDebug" : "gradlew assemble";
    int compileResult = compileResult = SysComExec(command, false, true, afterBuildFn);

    if (compileResult != 0)
    {
      returnLoggingError("", true);
      TK_ERR("Compiling failed.");
      return;
    }

    std::filesystem::current_path(workDir, ec); // set work directory back

    if (m_deployAfterBuild)
    {
      RunOnPhone();
    }

    if (returnLoggingError())
    {
      return;
    }
  }

  void WebPublisher::Publish() const
  {
    Path workDir = std::filesystem::current_path();

    std::error_code ec;
    auto returnLoggingError = [&ec, &workDir](bool setPathBack = false) -> bool
    {
      bool ret     = false;
      bool setback = setPathBack;
      if (ec)
      {
        TK_ERR("%s", ec.message().c_str());
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

    // Run toolkit compile script
    Path newWorkDir(ConcatPaths({"..", "BuildScripts"}));
    std::filesystem::current_path(newWorkDir, ec);
    if (returnLoggingError())
    {
      return;
    }

    int toolKitCompileResult = SysComExec("WebBuildRelease.bat", false, true);
    if (toolKitCompileResult != 0)
    {
      returnLoggingError(true);
      TK_ERR("ToolKit could not be compiled!");
      return;
    }
    newWorkDir                               = Path(ConcatPaths({ResourcePath(), "..", "Web"}));

    const String pluginWebBuildScriptsFolder = ConcatPaths({ResourcePath(), "..", "Web", "WebBuildRelease.bat"});

    // Run scripts
    std::filesystem::current_path(newWorkDir, ec);
    if (returnLoggingError())
    {
      return;
    }

    int pluginCompileResult = SysComExec(pluginWebBuildScriptsFolder.c_str(), false, true);
    if (pluginCompileResult != 0)
    {
      returnLoggingError(true);
      TK_ERR("Web build has failed!");
      return;
    }
    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError())
    {
      return;
    }

    // Move files to a directory
    String projectName           = activeProjectName;
    String publishDirectoryStr   = ConcatPaths({ResourcePath(), "..", "Publish", "Web"});
    const char* publishDirectory = publishDirectoryStr.c_str();
    String firstPart =
        ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}) + GetPathSeparatorAsStr() + projectName + ".";
    String files[] = {firstPart + "data", firstPart + "html", firstPart + "js", firstPart + "wasm"};

    std::filesystem::create_directories(publishDirectory, ec);
    if (returnLoggingError())
    {
      return;
    }

    for (int i = 0; i < ArraySize(files); i++)
    {
      std::filesystem::copy(files[i].c_str(), publishDirectory, std::filesystem::copy_options::overwrite_existing, ec);
      if (returnLoggingError())
      {
        return;
      }
    }

    // Create run script
    std::ofstream runBatchFile(ConcatPaths({publishDirectory, "Run.bat"}).c_str());
    runBatchFile << "emrun ./" + projectName + ".html";
    runBatchFile.close();

    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError())
    {
      return;
    }

    // Output user about where are the output files
    GetLogger()->WriteConsole(LogType::Success, "Building for web has been completed successfully.");
    GetLogger()->WriteConsole(LogType::Memo,
                              "Output files location: %s",
                              std::filesystem::absolute(publishDirectory).string().c_str());

    OpenExplorer(publishDirectory);
  }

  static bool finished = false;
  std::mutex messageMutex;

  static DWORD CreatePipe(void*)
  {
    WSADATA wsaData;
    SOCKET ConnectSocket    = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    String sendbuf = "a Hello from Packer";
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult        = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
      printf("WSAStartup failed with error: %d\n", iResult);
      return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult           = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
      printf("getaddrinfo failed with error: %d\n", iResult);
      WSACleanup();
      return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
      // Create a SOCKET for connecting to server
      ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
      if (ConnectSocket == INVALID_SOCKET)
      {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
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
      printf("Unable to connect to server!\n");
      WSACleanup();
      return 1;
    }
    
    iResult = recv(ConnectSocket, recvbuf, 4, 0);
    int correct = *((int*)recvbuf) == 0xff0ff;
    iResult = send(ConnectSocket, (char*)&correct, 4, 0);

    // Receive until the peer closes the connection
    while (!finished || !messages.empty())
    {
      while (messages.empty())
      {
        std::this_thread::yield();
        if (finished)
          goto end;
      }

      messageMutex.lock();
      sendbuf = messages.front();
      pop_front(messages);
      messageMutex.unlock();

      iResult = send(ConnectSocket, sendbuf.c_str(), sendbuf.size(), 0);
      if (iResult == SOCKET_ERROR)
      {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
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
      printf("shutdown failed with error: %d\n", WSAGetLastError());
      closesocket(ConnectSocket);
      WSACleanup();
      return 1;
    }
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
  }

  int ToolKitMain(int argc, char* argv[])
  {
    // Initialize ToolKit to serialize resources
    Main* g_proxy = new Main();
    Main::SetProxy(g_proxy);
    g_proxy->m_resourceRoot = ConcatPaths({"..", "Resources"});
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
    messages.push_back("a building android project");
    HANDLE hThread = CreateThread(nullptr, 0, CreatePipe, nullptr, 0, nullptr);
    AndroidPublisher publisher {};
    publisher.m_isDebugBuild     = true;
    publisher.m_deployAfterBuild = false;
    publisher.Publish();
    finished = true;

    while (messages.size())
      std::this_thread::yield();
    WaitForSingleObject(hThread, ~0ull);
    return 0;
  }
} // namespace ToolKit

int main(int argc, char* argv[]) { return ToolKit::ToolKitMain(argc, argv); }