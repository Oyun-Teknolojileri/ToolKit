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

#ifdef _WIN32
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <shellapi.h>
  #include <strsafe.h>

  // ToolKit collisions
  #undef WriteConsole

  #include <chrono>
  #include <thread>

namespace ToolKit
{
  namespace PlatformHelpers
  {
    namespace UTF8Util
    {
      // Function to convert UTF-8 to UTF-16
      std::wstring ConvertUTF8ToUTF16(const std::string& utf8String)
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
    } // namespace UTF8Util

    // Win32 console command execution callback.
    int SysComExec(StringView cmd, bool async, bool showConsole, std::function<void(int)> callback)
    {
      // https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes
      STARTUPINFOW si;
      PROCESS_INFORMATION pi;

      ZeroMemory(&si, sizeof(si));
      si.cb          = sizeof(si);
      si.dwFlags     = STARTF_USESHOWWINDOW;
      si.wShowWindow = showConsole ? SW_SHOWNORMAL : SW_HIDE;

      ZeroMemory(&pi, sizeof(pi));

      std::wstring wCmd = UTF8Util::ConvertUTF8ToUTF16("cmd /C ") + UTF8Util::ConvertUTF8ToUTF16(cmd.data());

      // Start the child process.
      if (!CreateProcessW(NULL,        // No module name (use command line)
                          wCmd.data(), // Command line
                          NULL,        // Process handle not inheritable
                          NULL,        // Thread handle not inheritable
                          FALSE,       // Set handle inheritance to FALSE
                          0,           // No creation flags
                          NULL,        // Use parent's environment block
                          NULL,        // Use parent's starting directory
                          &si,         // Pointer to STARTUPINFO structure
                          &pi)         // Pointer to PROCESS_INFORMATION structure
      )
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

      if (!async)
      {
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD stat = 0;
        GetExitCodeProcess(pi.hProcess, &stat);
        return finalizeFn(stat);
      }
      else
      {
        // We suppose to wait to call callback.
        if (callback != nullptr)
        {
          std::thread t(
              [pi, callback, finalizeFn]() -> void
              {
                DWORD stat = 0;
                bool exit  = false;
                while (!exit)
                {
                  GetExitCodeProcess(pi.hProcess, &stat);
                  if (stat != STILL_ACTIVE)
                  {
                    exit = true;
                  }

                  std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                finalizeFn(stat);
              });

          t.detach();
        }
        else
        {
          return finalizeFn(0);
        }
      }

      return 0;
    };

    void OutputLog(int logType, const char* szFormat, ...)
    {
      static const char* logNames[] = {"[Memo]", "[Error]", "[Warning]", "[Command]"};

      static char szBuff[1024]      = {0};
      va_list arg;
      va_start(arg, szFormat);
      _vsnprintf(szBuff, sizeof(szBuff), szFormat, arg);
      va_end(arg);

      static char szOutputBuff[1024] = {0};

      // concat log type name and log string
      _snprintf(szOutputBuff, sizeof(szOutputBuff), "%s %s\n", logNames[logType], szBuff);
      std::wstring wOutput = UTF8Util::ConvertUTF8ToUTF16(szOutputBuff);

      OutputDebugStringW(wOutput.data());
    }

    void OpenExplorer(const StringView utf8Path)
    {
      std::wstring utf16Path = UTF8Util::ConvertUTF8ToUTF16(utf8Path.data());
      HINSTANCE result =
          ShellExecuteW(GetActiveWindow(), L"open", L"explorer.exe", utf16Path.data(), NULL, SW_SHOWNORMAL);

      // Check the result of ShellExecute
      if ((intptr_t) result <= 32)
      {
        // ShellExecute failed
        TK_ERR("Failed to open the folder: %s", utf8Path);
      }
    }

    void HideConsoleWindow() 
    {
      HWND handle = GetConsoleWindow();
      ShowWindow(handle, SW_HIDE);
    }

    String GetCreationTime(const String& fullPath)
    {
      std::wstring wFile = UTF8Util::ConvertUTF8ToUTF16(fullPath.c_str());

      WIN32_FILE_ATTRIBUTE_DATA attrData;
      GetFileAttributesExW(wFile.data(), GetFileExInfoStandard, &attrData);

      String time = std::to_string(attrData.ftLastWriteTime.dwHighDateTime) +
                    std::to_string(attrData.ftLastWriteTime.dwLowDateTime);

      return time;
    }

    void* TKLoadModule(StringView fullPath)
    {
      std::wstring wFile = UTF8Util::ConvertUTF8ToUTF16(fullPath.data());
      HMODULE module     = LoadLibraryW(wFile.data());

      return (void*) module;
    }

    void TKFreeModule(void* module) { FreeLibrary((HMODULE) module); }

    void* TKGetFunction(void* module, StringView func) { return (void*) GetProcAddress((HMODULE) module, func.data()); }

  } // namespace PlatformHelpers
} // namespace ToolKit

#endif
