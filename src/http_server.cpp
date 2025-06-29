#include "http_server.h"
#include <iostream>
#include <string>
#include <winsock2.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <zlib.h>
extern std::string files_directory;

std::string gzip_compress(const std::string& data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        throw(std::runtime_error("deflateInit2 failed"));

    zs.next_in = (Bytef*)data.data();
    zs.avail_in = data.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END)
        throw(std::runtime_error("deflate failed"));

    return outstring;
}

std::unordered_map<std::string, std::string> parse_headers(const std::string& request) {
    std::unordered_map<std::string, std::string> headers;
    std::istringstream stream(request);
    std::string line;
    std::getline(stream, line); // skip request line
    while (std::getline(stream, line) && line != "\r") {
        size_t pos = line.find(":");
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            key.erase(std::remove(key.begin(), key.end(), '\r'), key.end());
            value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
            value.erase(0, value.find_first_not_of(" "));
            headers[key] = value;
        }
    }
    return headers;
}
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

    std::cout << "Received request:\n" << buffer << "\n";

    char method[16], path[256], protocol[16];
    sscanf(buffer, "%s %s %s", method, path, protocol);

    std::cout << "Method: " << method << "\n";
    std::cout << "Path: " << path << "\n";
    std::cout << "Protocol: " << protocol << "\n";

    const char *files_prefix = "/files/";
    size_t files_prefix_len = strlen(files_prefix);

    if (strcmp(method, "POST") == 0 && strncmp(path, files_prefix, files_prefix_len) == 0)
    {
        std::string filename = path + files_prefix_len;
        std::filesystem::path full_path = std::filesystem::path(files_directory) / filename;

        std::string req(buffer, bytes_received);
        size_t cl_pos = req.find("Content-Length:");
        size_t content_length = 0;
        if (cl_pos != std::string::npos)
        {
            size_t start = cl_pos + strlen("Content-Length:");
            while (start < req.size() && (req[start] == ' ' || req[start] == '\t')) start++;
            size_t end = req.find("\r\n", start);
            content_length = std::stoul(req.substr(start, end - start));
        }
        size_t header_end = req.find("\r\n\r\n");
        if (header_end == std::string::npos)
        {
            std::cerr << "Invalid request format\n";
            return -1;
        }

        std::string body = req.substr(header_end + 4);
        while (body.size() < content_length)
        {
            int more = recv(client_fd, buffer, buffer_size, 0);
            if (more <= 0) break;
            body.append(buffer, more);
        }

        std::ofstream outfile(full_path, std::ios::binary);
        outfile.write(body.data(), content_length);
        outfile.close();

        std::string resp = "HTTP/1.1 201 Created\r\n";
        resp += "Access-Control-Allow-Origin: *\r\n\r\n";
        send(client_fd, resp.c_str(), resp.size(), 0);
        return 0;
    }
    else if (strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0)
    {
        std::cerr << "Unsupported method: " << method << "\n";
        return -1;
    }
    else
    {
        const char *echo_prefix = "/echo/";
        size_t echo_prefix_len = strlen(echo_prefix);
        if (strncmp(path, files_prefix, files_prefix_len) == 0)
        {
            std::string filename = path + files_prefix_len;
            std::filesystem::path full_path = std::filesystem::path(files_directory) / filename;
            std::cout << "Trying to open file: " << full_path << std::endl;
            std::ifstream file(full_path, std::ios::binary | std::ios::ate);
            if (file)
            {
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                std::vector<char> buffer(size);
                file.read(buffer.data(), size);

                std::string headers = "HTTP/1.1 200 OK\r\n";
                headers += "Content-Type: application/octet-stream\r\n";
                headers += "Access-Control-Allow-Origin: *\r\n";
                headers += "Content-Length: " + std::to_string(size) + "\r\n\r\n";

                send(client_fd, headers.c_str(), headers.size(), 0);
                send(client_fd, buffer.data(), buffer.size(), 0);
            }
            else
            {
                std::string resp = "HTTP/1.1 404 Not Found\r\n";
                resp += "Access-Control-Allow-Origin: *\r\n\r\n";
                send(client_fd, resp.c_str(), resp.size(), 0);
            }
            return 0;
        }
        else if (strncmp(path, echo_prefix, echo_prefix_len) == 0)
        {
            const char *echo_text = path + echo_prefix_len;
            std::string body = echo_text;

            std::string req(buffer, bytes_received);
            auto headers = parse_headers(req);
            bool add_gzip_header = false;
            auto it = headers.find("Accept-Encoding");
            if (it != headers.end()) {
                std::istringstream iss(it->second);
                std::string encoding;
                while (std::getline(iss, encoding, ',')) {
                    encoding.erase(0, encoding.find_first_not_of(" \t"));
                    encoding.erase(encoding.find_last_not_of(" \t") + 1);
                    if (encoding == "gzip") {
                        add_gzip_header = true;
                        break;
                    }
                }
            }

            std::string resp = "HTTP/1.1 200 OK\r\n";
            resp += "Content-Type: text/plain\r\n";
            resp += "Access-Control-Allow-Origin: *\r\n";
            std::string response_body = body;
            if (add_gzip_header) {
                response_body = gzip_compress(body);
                resp += "Content-Encoding: gzip\r\n";
            }
            resp += "Content-Length: " + std::to_string(response_body.size()) + "\r\n\r\n";

            send(client_fd, resp.c_str(), resp.size(), 0);
            send(client_fd, response_body.data(), response_body.size(), 0);
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
            std::string headers = "HTTP/1.1 200 OK\r\n";
            headers += "Content-Type: text/plain\r\n";
            headers += "Access-Control-Allow-Origin: *\r\n";
            headers += "Content-Length: " + std::to_string(user_agent_value.size()) + "\r\n\r\n";
            std::string resp = headers + user_agent_value;
            send(client_fd, resp.c_str(), resp.size(), 0);
            std::cout << "User-Agent echoed: " << user_agent_value << "\n";
        }
        else if (strcmp(path, "/index.html") == 0 || strcmp(path, "/") == 0)
        {
            std::string resp = "HTTP/1.1 200 OK\r\n";
            resp += "Access-Control-Allow-Origin: *\r\n\r\n";
            send(client_fd, resp.c_str(), resp.size(), 0);
            std::cout << "Root/index path served.\n";
        }
        else
        {
            std::string resp = "HTTP/1.1 404 Not Found\r\n";
            resp += "Access-Control-Allow-Origin: *\r\n\r\n";
            send(client_fd, resp.c_str(), resp.size(), 0);
        }
        return 0;
    }
    return 0;
}
