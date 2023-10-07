#pragma once

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