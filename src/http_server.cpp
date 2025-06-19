#include "http_server.h"
#include <iostream>
#include <string>
#include <winsock2.h>

int read_request(SOCKET client_fd)
{
    const int buffer_size = 4096;
    char buffer[buffer_size];

    int bytes_received = recv(client_fd, buffer, buffer_size - 1, 0);
    if (bytes_received < 0)
    {
        std::cerr << "recv failed\n";
        return -1;
    }

    buffer[bytes_received] = '\0';

    std::cout << "Received request:\n"
              << buffer << "\n";

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
        const char *echo_prefix = "/echo/";
        size_t echo_prefix_len = strlen(echo_prefix);

        if (strncmp(path, echo_prefix, echo_prefix_len) == 0)
        {
            const char *echo_text = path + echo_prefix_len;
            std::string body = echo_text;
            std::string headers = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n";
            std::string resp = headers + body;
            send(client_fd, resp.c_str(), resp.size(), 0);
            std::cout << "Echoed: " << body << "\n";
        }
        else if (strcmp(path, "/user-agent") == 0)
        {
            std::string req(buffer);
            std::string user_agent_value;
            std::string search = "User-Agent: ";
            size_t pos = req.find(search);
            if (pos != std::string::npos)
            {
                size_t start = pos + search.length();
                size_t end = req.find("\r\n", start);
                user_agent_value = req.substr(start, end - start);
            }
            std::string headers = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(user_agent_value.size()) + "\r\n\r\n";
            std::string resp = headers + user_agent_value;
            send(client_fd, resp.c_str(), resp.size(), 0);
            std::cout << "User-Agent echoed: " << user_agent_value << "\n";
        }
        else if (strcmp(path, "/index.html") == 0)
        {
            std::string resp = "HTTP/1.1 200 OK\r\n\r\n";
            send(client_fd, resp.c_str(), resp.size(), 0);
            std::cout << "Request is valid.\n";
        }
        else if (strcmp(path, "/") == 0)
        {
            std::string resp = "HTTP/1.1 200 OK\r\n\r\n";
            send(client_fd, resp.c_str(), resp.size(), 0);
            std::cout << "Root path served as index.html\n";
        }
        else
        {
            std::string resp = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(client_fd, resp.c_str(), resp.size(), 0);
        }
        return 0;
    }
}