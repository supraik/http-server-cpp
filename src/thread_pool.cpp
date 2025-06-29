#include "thread_pool.h"
#include "http_server.h"

ThreadPool::ThreadPool(size_t num_threads) : stop(false)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers.emplace_back([this]
                             {
            while (true) {
                SOCKET client_fd;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty())
                        return;
                    client_fd = this->tasks.front();
                    this->tasks.pop();
                }
                // Persistent connection: handle multiple requests per socket
                while (true) {
                    int result = read_request(client_fd);
                    if (result < 0) break; 
                    
                    char peek_buf;
                    int peeked = recv(client_fd, &peek_buf, 1, MSG_PEEK);
                    if (peeked <= 0) break;
                }
                closesocket(client_fd);
            } });
    }
}

void ThreadPool::enqueue(SOCKET client_fd)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push(client_fd);
    }
    condition.notify_one();
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers)
        worker.join();
}