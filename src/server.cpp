#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib

int read_request(SOCKET client_fd)
{
  const int buffer_size = 4096; // 4KB buffer
  char buffer[buffer_size];

  int bytes_received = recv(client_fd, buffer, buffer_size - 1, 0);
  if (bytes_received < 0)
  {
    std::cerr << "recv failed\n";
    return -1;
  }

  buffer[bytes_received] = '\0'; // Null-terminate for safety

  std::cout << "Received request:\n"
            << buffer << "\n";

  // Extract the first line
  char method[16], path[256], protocol[16];
  sscanf(buffer, "%s %s %s", method, path, protocol);

  std::cout << "Method: " << method << "\n";
  std::cout << "Path: " << path << "\n";
  std::cout << "Protocol: " << protocol << "\n";
  if (strcmp(method, "GET") != 0)
  {
    std::cerr << "Unsupported method: " << method << "\n";
    return -1;
  }
  else
  {
    if (strcmp(path, "/index.html") == 0)
    {
      std::string resp = "HTTP/1.1 200 OK\r\n\r\n";
      send(client_fd, resp.c_str(), resp.size(), 0);
      std::cout << "Request is valid.\n";
    }
    else
    {
      std::string resp = "HTTP/1.1 404 Not Found\r\n\r\n";
      send(client_fd, resp.c_str(), resp.size(), 0);
    }
    return 0;
  }
}

int main(int argc, char **argv)
{
  // Initialize Winsock
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    std::cerr << "WSAStartup failed\n";
    return 1;
  }

  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::cout << "Logs from your program will appear here!\n";

  SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
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

  sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  SOCKET client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_addr_len);

  if (client_fd == INVALID_SOCKET)
  {
    std::cerr << "Failed to accept client connection\n";
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  std::cout << "Client connected\n";

  read_request(client_fd);

  closesocket(client_fd);
  closesocket(server_fd);
  WSACleanup();

  return 0;
}