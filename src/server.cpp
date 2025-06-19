#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "thread_pool.h"
#include "http_server.h"

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char **argv)
{
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::cout << "Logs from your program will appear here!\n";

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    std::cerr << "WSAStartup failed\n";
    return 1;
  }

  SOCKET server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_fd == INVALID_SOCKET)
  {
    std::cerr << "Failed to create server socket\n";
    WSACleanup();
    return 1;
  }

  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0)
  {
    std::cerr << "setsockopt failed\n";
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
  {
    std::cerr << "Failed to bind to port 4221\n";
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) == SOCKET_ERROR)
  {
    std::cerr << "listen failed\n";
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  ThreadPool pool(std::thread::hardware_concurrency());

  std::cout << "Waiting for clients...\n";
  while (true)
  {
    SOCKET client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd == INVALID_SOCKET)
    {
      std::cerr << "Failed to accept client connection\n";
      break;
    }
    pool.enqueue((int)client_fd); // Cast to int if your ThreadPool expects int
  }

  closesocket(server_fd);
  WSACleanup();
  return 0;
}