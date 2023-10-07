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

#include "PublishManager.h"

#include "App.h"
#include "FileManager.h"

#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <thread>

#include "DebugNew.h"

#ifdef _WIN32
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <shellapi.h>
  #include <strsafe.h>

  // ToolKit collisions
  #undef WriteConsole

  #define _WINSOCK_DEPRECATED_NO_WARNINGS
  #include <WinSock2.h>
  #include <iphlpapi.h>
  #include <ws2tcpip.h>

  #define DEFAULT_BUFLEN 512
  #define DEFAULT_PORT   "27015"
  #pragma comment(lib, "ws2_32.lib")
  #pragma comment(lib, "Mswsock.lib")
  #pragma comment(lib, "AdvApi32.lib")

  // linux compatibility things: https://gist.github.com/willeccles/3ba0741143b573b74b1c0a7aea2bdb40
  typedef SOCKET socket_t;
  typedef int socklen_t;
  /* POSIX ssize_t is not a thing on Windows */
  typedef signed long long int ssize_t;
  // Must be called on Windows before the program ends. No-op on unix.
  #define sock_cleanup() WSACleanup()

#else
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <sys/socket.h>
  #include <sys/time.h>
  #include <sys/types.h>
  #include <unistd.h>
  typedef int socket_t;
  // winsock has INVALID_SOCKET which is returned by socket(),
  // this is the POSIX replacement
  #define INVALID_SOCKET -1
  #define sock_cleanup()
  #define close_socket(x) close(x)
#endif

inline void sock_perror(const char* msg)
{
#ifdef _WIN32
  TK_ERR("%s: %ld\n", msg, WSAGetLastError());
#else
  TK_ERR(msg);
#endif
}

// Returns 0 on success.
inline int sock_init()
{
#ifdef _WIN32
  WSADATA wsaData;
  return WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
  return 0;
#endif
}
namespace ToolKit
{
  namespace Editor
  {
    uint64 PublishManager::BuildArgs = 0;
    bool PublishManager::IsBuilding  = false;

    // this function creates an loop that recieves packer's messages and log them in ToolKit console.
    int PublishManager::CreatePackerMessageListener()
    {
      // execute the functions that we need after we exit from this function, using C++ RAII
      struct ExecOnExit
      {
        std::function<void()> fn;

        ExecOnExit(std::function<void()> f) : fn(std::move(f)) {}

        ~ExecOnExit() { fn(); }
      };

      // when we exit this thread we must set IsBuilding to false using C++ RAII
      ExecOnExit setBuildingFalse([]() -> void { IsBuilding = false; });

      // Initialize Winsock
      int iResult = sock_init();
      if (iResult != 0)
      {
        sock_perror("Packer message listener socket failed with error");
        return 1;
      }

      ExecOnExit sockCleanup([]() -> void { sock_cleanup(); });

      addrinfo hints;
      memset(&hints, 0, sizeof(hints));
      hints.ai_family   = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
      hints.ai_flags    = AI_PASSIVE;

      addrinfo* result  = NULL;
      // Resolve the server address and port
      iResult           = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
      if (iResult != 0)
      {
        sock_perror("getaddrinfo failed with error");
        return 1;
      }

      ExecOnExit addrCleanup([result]() -> void { freeaddrinfo(result); });

      // Create a SOCKET for the server to listen for client connections.
      SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
      if (ListenSocket == INVALID_SOCKET)
      {
        sock_perror("socket failed with error");
        return 1;
      }

      ExecOnExit listenSocketCleanup([ListenSocket]() -> void { closesocket(ListenSocket); });

      // Setup the TCP listening socket
      if (bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen) == SOCKET_ERROR)
      {
        sock_perror("bind failed with error");
        return 1;
      }

      if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
      {
        sock_perror("listen failed with error");
        return 1;
      }

      // Accept a client socket
      SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
      if (ClientSocket == INVALID_SOCKET)
      {
        sock_perror("accept failed with error");
        return 1;
      }

      ExecOnExit clientSocketCleanup([ClientSocket]() -> void { closesocket(ClientSocket); });

      if (send(ClientSocket, (char*) &BuildArgs, 8, 0) == SOCKET_ERROR)
      {
        sock_perror("sending build args failed");
        return 1;
      }

      char recvbuf[DEFAULT_BUFLEN] {};
      int recvbuflen = DEFAULT_BUFLEN;
      // Receive until the peer shuts down the connection
      do
      {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
          // the message that comes start with latter a-d which indicates
          // a: error, b: memo, c: success, d: warning
          if (recvbuf[0] == 'a' + (char) LogType::Error)
          {
            TK_ERR("PACKER %s", recvbuf + 1);
          }
          else if (recvbuf[0] == 'a' + (char) LogType::Memo)
          {
            TK_LOG("PACKER %s", recvbuf + 1);
          }
          else if (recvbuf[0] == 'a' + (char) LogType::Success)
          {
            GetLogger()->WriteConsole(LogType::Success, "PACKER %s", recvbuf + 1);
          }
          else if (recvbuf[0] == 'a' + (char) LogType::Warning)
          {
            TK_WRN("PACKER %s", recvbuf + 1);
          }

          memset(recvbuf, 0, sizeof(recvbuf));
        }
        else if (iResult == 0)
        {
          TK_LOG("Packer message listenner closing\n");
        }
        else
        {
          sock_perror("recv failed with error");
          return 1;
        }

      } while (iResult > 0);

      // shutdown the connection since we're done
      if (shutdown(ClientSocket, SD_SEND) == SOCKET_ERROR)
      {
        sock_perror("shutdown failed with error");
        return 1;
      }

      return 0;
    }

    PublishManager::PublishManager() {}
    
    PublishManager::~PublishManager() 
    {
      if (m_thread.joinable())
      {
        m_thread.join();
      }
    }

    void PublishManager::Publish(PublishPlatform platform)
    {
      if (IsBuilding)
      {
        TK_WRN("Toolkit already building an project");
        return;
      }
      if (m_thread.joinable())
      {
        m_thread.join();
      }
      // Pack all of the integer data into an uint64 to send it to the packer.
      // the values are less than 8bit(1byte) that's why we can pack all of the arguments in one integer 
      uint64 mask              = ((uint64) m_deployAfterBuild << 0ull) | ((uint64) m_isDebugBuild << 8ull);
      mask                    |= ((uint64) m_minSdk << 16ull) | ((uint64) m_maxSdk << 24ull);
      mask                    |= ((uint64) platform << 32ull) | ((uint64) m_oriantation << 40ull);
      BuildArgs                = mask;

      String publishArguments  = g_app->m_workspace.GetActiveProject().name + '\n';
      publishArguments        += g_app->m_workspace.GetActiveWorkspace() + '\n';
      publishArguments        += m_appName.empty() ? g_app->m_workspace.GetActiveProject().name : m_appName;
      GetFileManager()->WriteAllText("PublishArguments.txt", publishArguments);

      m_thread          = std::thread(CreatePackerMessageListener);

      String packerPath = "Utils\\Packer.exe";
      NormalizePath(packerPath);
      // close zip file before running packer, because packer will use this file as well,
      // this will cause errors otherwise
      GetFileManager()->CloseZipFile();

      packerPath = std::filesystem::absolute(ConcatPaths({"..", packerPath})).string();

      IsBuilding = true;
      g_app->ExecSysCommand(packerPath, true, false, nullptr);
    }

  } // namespace Editor
} // namespace ToolKit
