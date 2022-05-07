#include "thread_pool.h"

#include <iostream>
#include <thread>
#include <functional>
#include <stdexcept>
#include <mutex>

server::concurrent::ThreadPool::ThreadPool() : max_threads(std::thread::hardware_concurrency()) {
    shutdown = false;
}

server::concurrent::ThreadPool::ThreadPool(int max_threads) : max_threads(max_threads) {    
}

server::concurrent::ThreadPool::~ThreadPool() {
    shutdown = true;
    for (auto it = threads.begin(); it != threads.end(); it++) {
        it->join();
    }
}

bool server::concurrent::ThreadPool::is_shut_down() const {
    return shutdown;
}

void server::concurrent::ThreadPool::execute_next_job() {
    std::function<void()> job;
    {
        std::unique_lock<std::mutex> jobs_lock(jobs_mutex);
        jobs_available.wait(jobs_lock, [&] { return !jobs.empty();});
        job = jobs.front();
        jobs.pop();
    }
    job();
}

void server::concurrent::ThreadPool::execute(std::function<void()> job) {
    {
        std::lock_guard<std::mutex> guard(jobs_mutex);
        jobs.push(job);
        if (threads.size() < max_threads) {
            threads.push_back(std::thread(thread_execute, std::ref(*this)));
        }
    }
    jobs_available.notify_one();
}

void server::concurrent::thread_execute(ThreadPool& thread_pool) {
    while (!thread_pool.is_shut_down()) {
        thread_pool.execute_next_job();
    }
}
