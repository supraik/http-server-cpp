# C++ HTTP Server

A lightweight, high-performance HTTP/1.1 server written in modern C++17.  
Features include URL routing, header parsing, gzip compression, persistent connections, explicit `Connection: close` handling, and concurrency via a thread pool.

🔗 **Repo:** https://github.com/supratik/http-server-cpp

---

## 🚀 Features

- **HTTP/1.1 Compliance**  
  - Parses request lines, headers, and path parameters  
  - Supports GET (easily extended to POST, PUT, etc.)

- **Gzip Compression**  
  - Automatically detects `Accept-Encoding: gzip`  
  - Compresses responses on-the-fly with zlib’s gzip mode  
  - Sets `Content-Encoding: gzip` and correct `Content-Length`

- **Persistent Connections**  
  - Reuses the same TCP socket for multiple requests by default  
  - Honors explicit `Connection: close` to terminate cleanly

- **Thread-Pool Concurrency**  
  - Fixed-size pool of worker threads  
  - Each worker handles one client socket in a loop  
  - Scales to hundreds of simultaneous connections

- **Simple Routing & Handlers**  
  - Map URL patterns (e.g. `/echo/{str}`, `/user-agent`) to C++ lambdas  
  - Easy to register new endpoints

---

