#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <winsock2.h>

class ThreadPool
{
public:
    ThreadPool(size_t num_threads);
    void enqueue(SOCKET client_fd);
    ~ThreadPool();

private:
    std::vector<std::thread> workers;
    std::queue<SOCKET> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};