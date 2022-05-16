#ifndef COROS_CONCURRENT_THREAD_POOL_H
#define COROS_CONCURRENT_THREAD_POOL_H

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace coros {
    namespace concurrent {
        class ThreadPool {
            private:
                const int max_threads;
                std::atomic_bool is_shutdown;
                std::vector<std::thread> threads;
                std::queue<std::function<void()>> jobs;
                std::mutex jobs_mutex;
                std::condition_variable jobs_condition;
                void run_jobs();
            public:
                ThreadPool();
                ThreadPool(int max_threads);
                void shutdown();
                void run(std::function<void()> job);
        };
    }
}

#endif
