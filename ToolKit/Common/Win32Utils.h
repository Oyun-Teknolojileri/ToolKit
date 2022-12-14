#pragma once

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <atlstr.h>
  #include <strsafe.h>

  #include <chrono>
  #include <thread>

  // ToolKit collisions
  #undef WriteConsole

//////////////////////////////////////////////////////////////////////////////
//
// *** Routines to convert between Unicode UTF-8 and Unicode UTF-16 ***
//
// By Giovanni Dicanio <giovanni.dicanio AT gmail.com>
//
// Last update: 2010, January 2nd
//
//
// These routines use ::MultiByteToWideChar and ::WideCharToMultiByte
// Win32 API functions to convert between Unicode UTF-8 and UTF-16.
//
// UTF-16 strings are stored in instances of CStringW.
// UTF-8 strings are stored in instances of CStringA.
//
// On error, the conversion routines use AtlThrow to signal the
// error condition.
//
// If input string pointers are NULL, empty strings are returned.
//
//
// Prefixes used in these routines:
// --------------------------------
//
//  - cch  : count of characters (CHAR's or WCHAR's)
//  - cb   : count of bytes
//  - psz  : pointer to a NUL-terminated string (CHAR* or WCHAR*)
//  - str  : instance of CString(A/W) class
//
//
//
// Useful Web References:
// ----------------------
//
// WideCharToMultiByte Function
// http://msdn.microsoft.com/en-us/library/dd374130.aspx
//
// MultiByteToWideChar Function
// http://msdn.microsoft.com/en-us/library/dd319072.aspx
//
// AtlThrow
// http://msdn.microsoft.com/en-us/library/z325eyx0.aspx
//
//
// Developed on VC9 (Visual Studio 2008 SP1)
//
//
//////////////////////////////////////////////////////////////////////////////

namespace ToolKit
{
  namespace Win32Helpers
  {
    namespace UTF8Util
    {

      //----------------------------------------------------------------------------
      // FUNCTION: ConvertUTF8ToUTF16
      // DESC: Converts Unicode UTF-8 text to Unicode UTF-16 (Windows default).
      //----------------------------------------------------------------------------
      CStringW ConvertUTF8ToUTF16(__in const CHAR* pszTextUTF8)
      {
        //
        // Special case of NULL or empty input string
        //
        if ((pszTextUTF8 == NULL) || (*pszTextUTF8 == '\0'))
        {
          // Return empty string
          return L"";
        }

        //
        // Consider CHAR's count corresponding to total input string length,
        // including end-of-string (\0) character
        //
        const size_t cchUTF8Max = INT_MAX - 1;
        size_t cchUTF8;
        HRESULT hr = ::StringCchLengthA(pszTextUTF8, cchUTF8Max, &cchUTF8);
        if (FAILED(hr))
        {
          AtlThrow(hr);
        }

        // Consider also terminating \0
        ++cchUTF8;

        // Convert to 'int' for use with MultiByteToWideChar API
        int cbUTF8 = static_cast<int>(cchUTF8);

        //
        // Get size of destination UTF-16 buffer, in WCHAR's
        //
        int cchUTF16 = ::MultiByteToWideChar(
            CP_UTF8,              // convert from UTF-8
            MB_ERR_INVALID_CHARS, // error on invalid chars
            pszTextUTF8,          // source UTF-8 string
            cbUTF8,               // total length of source UTF-8 string,
                    // in CHAR's (= bytes), including end-of-string \0
            NULL, // unused - no conversion done in this step
            0     // request size of destination buffer, in WCHAR's
        );
        ATLASSERT(cchUTF16 != 0);
        if (cchUTF16 == 0)
        {
          AtlThrowLastWin32();
        }

        //
        // Allocate destination buffer to store UTF-16 string
        //
        CStringW strUTF16;
        WCHAR* pszUTF16 = strUTF16.GetBuffer(cchUTF16);

        //
        // Do the conversion from UTF-8 to UTF-16
        //
        int result = ::MultiByteToWideChar(
            CP_UTF8,              // convert from UTF-8
            MB_ERR_INVALID_CHARS, // error on invalid chars
            pszTextUTF8,          // source UTF-8 string
            cbUTF8,               // total length of source UTF-8 string,
                    // in CHAR's (= bytes), including end-of-string \0
            pszUTF16, // destination buffer
            cchUTF16  // size of destination buffer, in WCHAR's
        );
        ATLASSERT(result != 0);
        if (result == 0)
        {
          AtlThrowLastWin32();
        }

        // Release internal CString buffer
        strUTF16.ReleaseBuffer();

        // Return resulting UTF16 string
        return strUTF16;
      }

      //----------------------------------------------------------------------------
      // FUNCTION: ConvertUTF16ToUTF8
      // DESC: Converts Unicode UTF-16 (Windows default) text to Unicode UTF-8.
      //----------------------------------------------------------------------------
      CStringA ConvertUTF16ToUTF8(__in const WCHAR* pszTextUTF16)
      {
        //
        // Special case of NULL or empty input string
        //
        if ((pszTextUTF16 == NULL) || (*pszTextUTF16 == L'\0'))
        {
          // Return empty string
          return "";
        }

        //
        // Consider WCHAR's count corresponding to total input string length,
        // including end-of-string (L'\0') character.
        //
        const size_t cchUTF16Max = INT_MAX - 1;
        size_t cchUTF16;
        HRESULT hr = ::StringCchLengthW(pszTextUTF16, cchUTF16Max, &cchUTF16);
        if (FAILED(hr))
        {
          AtlThrow(hr);
        }

        // Consider also terminating \0
        ++cchUTF16;

        //
        // WC_ERR_INVALID_CHARS flag is set to fail if invalid input character
        // is encountered.
        // This flag is supported on Windows Vista and later.
        // Don't use it on Windows XP and previous.
        //
  #if (WINVER >= 0x0600)
        DWORD dwConversionFlags = WC_ERR_INVALID_CHARS;
  #else
        DWORD dwConversionFlags = 0;
  #endif

        //
        // Get size of destination UTF-8 buffer, in CHAR's (= bytes)
        //
        int cbUTF8 = ::WideCharToMultiByte(
            CP_UTF8,                    // convert to UTF-8
            dwConversionFlags,          // specify conversion behavior
            pszTextUTF16,               // source UTF-16 string
            static_cast<int>(cchUTF16), // total source string length, in
                                        // WCHAR's, including end-of-string \0
            NULL, // unused - no conversion required in this step
            0,    // request buffer size
            NULL,
            NULL // unused
        );
        ATLASSERT(cbUTF8 != 0);
        if (cbUTF8 == 0)
        {
          AtlThrowLastWin32();
        }

        //
        // Allocate destination buffer for UTF-8 string
        //
        CStringA strUTF8;
        int cchUTF8   = cbUTF8; // sizeof(CHAR) = 1 byte
        CHAR* pszUTF8 = strUTF8.GetBuffer(cchUTF8);

        //
        // Do the conversion from UTF-16 to UTF-8
        //
        int result = ::WideCharToMultiByte(
            CP_UTF8,                    // convert to UTF-8
            dwConversionFlags,          // specify conversion behavior
            pszTextUTF16,               // source UTF-16 string
            static_cast<int>(cchUTF16), // total source string length, in
                                        // WCHAR's, including end-of-string \0
            pszUTF8,                    // destination buffer
            cbUTF8,                     // destination buffer size, in bytes
            NULL,
            NULL // unused
        );
        ATLASSERT(result != 0);
        if (result == 0)
        {
          AtlThrowLastWin32();
        }

        // Release internal CString buffer
        strUTF8.ReleaseBuffer();

        // Return resulting UTF-8 string
        return strUTF8;
      }

    } // namespace UTF8Util

    // Win32 console command execution callback.
    static auto g_SysComExecFn = [](StringView cmd,
                                    bool async,
                                    bool showConsole,
                                    std::function<void(int)> callback) -> int {
      // https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes
      STARTUPINFOW si;
      PROCESS_INFORMATION pi;

      ZeroMemory(&si, sizeof(si));
      si.cb          = sizeof(si);
      si.dwFlags     = STARTF_USESHOWWINDOW;
      si.wShowWindow = showConsole ? SW_SHOW : SW_HIDE;

      ZeroMemory(&pi, sizeof(pi));

      CStringW wCmd =
          ToolKit::Win32Helpers::UTF8Util::ConvertUTF8ToUTF16("cmd /C ") +
          ToolKit::Win32Helpers::UTF8Util::ConvertUTF8ToUTF16(cmd.data());

      // Start the child process.
      if (!CreateProcessW(NULL,             // No module name (use command line)
                          wCmd.GetBuffer(), // Command line
                          NULL,             // Process handle not inheritable
                          NULL,             // Thread handle not inheritable
                          FALSE,            // Set handle inheritance to FALSE
                          0,                // No creation flags
                          NULL,             // Use parent's environment block
                          NULL,             // Use parent's starting directory
                          &si,              // Pointer to STARTUPINFO structure
                          &pi) // Pointer to PROCESS_INFORMATION structure
      )
      {
        DWORD errCode = GetLastError();
        GetLogger()->WriteConsole(
            LogType::Error, "CreateProcess failed (%d).\n", errCode);
        return (int) errCode;
      }

      auto finalizeFn = [pi, callback](DWORD stat) -> void {
        // Close process and thread handles.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (callback != nullptr)
        {
          callback((int) stat);
        }
      };

      if (!async)
      {
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD stat = 0;
        GetExitCodeProcess(pi.hProcess, &stat);
        finalizeFn(stat);
      }
      else
      {
        // We suppose to wait to call callback.
        if (callback != nullptr)
        {
          std::thread t([pi, callback, finalizeFn]() -> void {
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
          finalizeFn(0);
        }
      }

      return 0;
    };

  } // namespace Win32Helpers
} // namespace ToolKit

#endif
