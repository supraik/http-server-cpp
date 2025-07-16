# C++ HTTP Server - Complete Setup & Usage Guide

## Prerequisites

- **Windows** (MSYS2/MinGW-w64 recommended)
- **MSYS2 UCRT64 terminal** ([Download MSYS2](https://www.msys2.org/))
- **GCC** (via MSYS2)
- **zlib** (`mingw-w64-ucrt-x86_64-zlib`)
- **VS Code** (optional, for editing/building)
- **Git** (optional, for cloning)

---

## 1. Install MSYS2 and Required Packages

Open the **MSYS2 UCRT64** terminal and run:

```sh
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-zlib
```

---

## 2. Clone or Download the Repository

```sh
git clone https://github.com/supratik/http-server-cpp.git
cd http-server-cpp
```
Or download and extract the ZIP, then `cd` into the folder.

---

## 3. Build the Server

### **A. Using MSYS2 Terminal**

```sh
cd src
g++ -std=c++20 -g server.cpp thread_pool.cpp http_server.cpp -o server.exe -lz -lws2_32
```

### **B. Using VS Code**

- Open the project folder in VS Code.
- Press <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>B</kbd> and select **"Build server.cpp"**.
- The executable will be generated at `src/server.exe`.

---

## 4. Run the Server

### **Default (serve current directory):**
```sh
./server.exe
```

### **Serve a specific directory:**
```sh
./server.exe --directory C:\Users\supra\OneDrive\Documents\Portfolio\http-server-cpp\src
```

### **Set environment variable (MSYS2 syntax):**
```sh
export FILES_DIRECTORY=/c/Users/supra/OneDrive/Documents/Portfolio/http-server-cpp/src
./server.exe
```

---

## 5. API Endpoints

- **Upload a file:**  
  `POST /files/<filename>`  
  Example (using `curl`):
  ```sh
  curl -X POST --data-binary @localfile.txt http://localhost:4221/files/localfile.txt
  ```

- **Download a file:**  
  `GET /files/<filename>`  
  Example:
  ```sh
  curl http://localhost:4221/files/localfile.txt -o downloaded.txt
  ```

- **Echo a message:**  
  `GET /echo/<message>`  
  Example:
  ```sh
  curl http://localhost:4221/echo/hello
  ```

- **Get User-Agent:**  
  `GET /user-agent`  
  Example:
  ```sh
  curl http://localhost:4221/user-agent
  ```

- **CORS Preflight (OPTIONS):**  
  `OPTIONS /files/<filename>`  
  Example:
  ```sh
  curl -X OPTIONS http://localhost:4221/files/localfile.txt
  ```

---

## 6. Frontend Usage

- Open `src/index.html` or `frontend/index.html` in your browser.
- Use the dashboard UI to upload/download files.

---

## 7. Troubleshooting

- **Port in use:** Make sure port `4221` is free.
- **Environment variable not working:** Use `export` in MSYS2, not `set`.
- **Missing DLLs:** Run from MSYS2 UCRT64 terminal.
- **Gzip errors:** Ensure `zlib` is installed and linked (`-lz`).

---

## 8. Example Workflow

```sh
# Build
cd src
g++ -std=c++20 -g server.cpp thread_pool.cpp http_server.cpp -o server.exe -lz -lws2_32

# Run (serving current directory)
./server.exe

# Upload a file
curl -X POST --data-binary @localfile.txt http://localhost:4221/files/localfile.txt

# Download a file
curl http://localhost:4221/files/localfile.txt -o downloaded.txt
```

---

**Happy coding!**
