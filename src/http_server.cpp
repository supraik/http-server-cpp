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
    std::getline(stream, line);
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

int read_request(SOCKET client_fd) {
    const int buffer_size = 4096;
    char buffer[buffer_size];
    std::string request;

    int bytes_received;
    while ((bytes_received = recv(client_fd, buffer, buffer_size - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        request.append(buffer, bytes_received);
        if (request.find("\r\n\r\n") != std::string::npos) break;
    }

    if (request.empty()) return -1;

    std::istringstream request_stream(request);
    std::string request_line;
    std::getline(request_stream, request_line);

    char method[16], path[256], protocol[16];
    sscanf(request_line.c_str(), "%s %s %s", method, path, protocol);

    auto headers = parse_headers(request);
    std::cout<<"Options==>"<<" "<<method<<std::endl;
   /* if (strcmp(method, "OPTIONS") == 0) {
        std::string resp = "HTTP/1.1 204 No Content\r\n";
        resp += "Access-Control-Allow-Origin: *\r\n";
        resp += "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n";
        resp += "Access-Control-Allow-Headers: Content-Type\r\n\r\n";
        send(client_fd, resp.c_str(), resp.size(), 0);
        return 0;
    }/*/

    if (strcmp(method, "POST") == 0 && strncmp(path, "/files/", 7) == 0) {
        std::string filename = path + 7;
        std::filesystem::path full_path = std::filesystem::path(files_directory) / filename;

        size_t header_end = request.find("\r\n\r\n");
        size_t body_start = header_end + 4;

        auto content_length_it = headers.find("Content-Length");
        if (content_length_it == headers.end()) {
           std::string resp = "HTTP/1.1 201 Created\r\n";
resp += "Access-Control-Allow-Origin: *\r\n";
resp += "Content-Length: 0\r\n\r\n";
            send(client_fd, resp.c_str(), resp.size(), 0);
            return -1;
        }

        size_t content_length = std::stoul(content_length_it->second);
        std::string body = request.substr(body_start);

        while (body.size() < content_length) {
            int more = recv(client_fd, buffer, buffer_size, 0);
            if (more <= 0) break;
            body.append(buffer, more);
        }

        std::ofstream outfile(full_path, std::ios::binary);
        outfile.write(body.data(), content_length);
        outfile.close();

       std::string resp = "HTTP/1.1 201 Created\r\n";
resp += "Access-Control-Allow-Origin: *\r\n";
resp += "Content-Length: 0\r\n\r\n";
send(client_fd, resp.c_str(), resp.size(), 0);

        send(client_fd, resp.c_str(), resp.size(), 0);
        return 0;
    }

    if (strcmp(method, "GET") == 0 && strncmp(path, "/files/", 7) == 0) {
        std::string filename = path + 7;
        std::filesystem::path full_path = std::filesystem::path(files_directory) / filename;
        std::ifstream file(full_path, std::ios::binary | std::ios::ate);
        if (!file) {
            std::string resp = "HTTP/1.1 404 Not Found\r\nAccess-Control-Allow-Origin: *\r\n\r\n";
            send(client_fd, resp.c_str(), resp.size(), 0);
            return 0;
        }
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
        return 0;
    }

    if (strncmp(path, "/echo/", 6) == 0) {
        std::string msg = path + 6;
        std::string body = msg;

        bool gzip = false;
        auto ae = headers.find("Accept-Encoding");
        if (ae != headers.end() && ae->second.find("gzip") != std::string::npos)
            gzip = true;

        std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nAccess-Control-Allow-Origin: *\r\n";
        if (gzip) {
            body = gzip_compress(body);
            resp += "Content-Encoding: gzip\r\n";
        }
        resp += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
        send(client_fd, resp.c_str(), resp.size(), 0);
        send(client_fd, body.data(), body.size(), 0);
        return 0;
    }

    if (strcmp(path, "/user-agent") == 0) {
        std::string search = "User-Agent: ";
        std::string ua;
        size_t pos = request.find(search);
        if (pos != std::string::npos) {
            size_t start = pos + search.size();
            size_t end = request.find("\r\n", start);
            ua = request.substr(start, end - start);
        }
        std::string headers = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nAccess-Control-Allow-Origin: *\r\n";
        headers += "Content-Length: " + std::to_string(ua.size()) + "\r\n\r\n";
        std::string resp = headers + ua;
        send(client_fd, resp.c_str(), resp.size(), 0);
        return 0;
    }

    std::string resp = "HTTP/1.1 404 Not Found\r\nAccess-Control-Allow-Origin: *\r\n\r\n";
    send(client_fd, resp.c_str(), resp.size(), 0);
    return 0;
}
