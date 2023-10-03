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
  #include <thread>

  // ToolKit collisions
  #undef WriteConsole

  #include "Windows.h"

  #define _WINSOCK_DEPRECATED_NO_WARNINGS
  #include <WinSock2.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <ws2tcpip.h>

  #define DEFAULT_BUFLEN 512
  #define DEFAULT_PORT   "27015"

  #pragma comment(lib, "ws2_32.lib")
#endif

#include "App.h"
#include "FileManager.h"
#include "PublishManager.h"

#include "DebugNew.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

namespace ToolKit
{
  namespace Editor
  {
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

    DWORD WINAPI CreatePipe(LPVOID lpParam)
    {
      WSADATA wsaData;
      int iResult;

      SOCKET ListenSocket     = INVALID_SOCKET;
      SOCKET ClientSocket     = INVALID_SOCKET;

      struct addrinfo* result = NULL;
      struct addrinfo hints;

      int iSendResult;
      char recvbuf[DEFAULT_BUFLEN] {};
      int recvbuflen = DEFAULT_BUFLEN;

      // Initialize Winsock
      iResult        = WSAStartup(MAKEWORD(2, 2), &wsaData);
      if (iResult != 0)
      {
        TK_ERR("WSAStartup failed with error: %d\n", iResult);
        return 1;
      }

      ZeroMemory(&hints, sizeof(hints));
      hints.ai_family   = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
      hints.ai_flags    = AI_PASSIVE;

      // Resolve the server address and port
      iResult           = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
      if (iResult != 0)
      {
        TK_ERR("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
      }

      // Create a SOCKET for the server to listen for client connections.
      ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
      if (ListenSocket == INVALID_SOCKET)
      {
        TK_ERR("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
      }

      // Setup the TCP listening socket
      iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
      if (iResult == SOCKET_ERROR)
      {
        TK_ERR("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
      }

      freeaddrinfo(result);

      iResult = listen(ListenSocket, SOMAXCONN);
      if (iResult == SOCKET_ERROR)
      {
        TK_ERR("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
      }

      // Accept a client socket
      ClientSocket = accept(ListenSocket, NULL, NULL);
      if (ClientSocket == INVALID_SOCKET)
      {
        TK_ERR("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
      }

      // No longer need server socket
      closesocket(ListenSocket);

      // Receive until the peer shuts down the connection
      do
      {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
          if (recvbuf[0] == 'a' + (char) LogType::Error) TK_ERR("PACKER %s", recvbuf+1);
          else if (recvbuf[0] == 'a' + (char)LogType::Memo) TK_LOG("PACKER %s", recvbuf+1);
          else if (recvbuf[0] == 'a' + (char)LogType::Success) GetLogger()->WriteConsole(LogType::Success, "PACKER %s", recvbuf+1);
          else if (recvbuf[0] == 'a' + (char)LogType::Warning) TK_WRN("PACKER %s", recvbuf+1);
          
          memset(recvbuf, 0, sizeof(recvbuf));
        }
        else if (iResult == 0)
          TK_LOG("Connection closing...\n");
        else
        {
          TK_ERR("recv failed with error: %d\n", WSAGetLastError());
          closesocket(ClientSocket);
          WSACleanup();
          return 1;
        }

      } while (iResult > 0);

      // shutdown the connection since we're done
      iResult = shutdown(ClientSocket, SD_SEND);
      if (iResult == SOCKET_ERROR)
      {
        TK_ERR("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
      }

      // cleanup
      closesocket(ClientSocket);
      WSACleanup();
      return 0;
    }

    std::thread pipeThread;

    void PublishManager::Publish(PublishPlatform platform)
    {
      HANDLE hThread = CreateThread(nullptr, 0, CreatePipe, nullptr, 0, nullptr);
      g_app->PackResources();
      String packerPath = "Utils\\Packer\\Packer.exe";
      NormalizePath(packerPath);
      packerPath = std::filesystem::absolute(ConcatPaths({ "..", packerPath})).string();

      g_app->ExecSysCommand(packerPath, true, false, nullptr);
      return;
    }

    void WindowsPublisher::Publish() const
    {
    }

    void AndroidPublisher::RunOnPhone() const
    {
    }

    void AndroidPublisher::EditAndroidManifest() const
    {
    }

    void AndroidPublisher::Publish() const
    {

    }

    void WebPublisher::Publish() const
    {

    }

  } // namespace Editor
} // namespace ToolKit
