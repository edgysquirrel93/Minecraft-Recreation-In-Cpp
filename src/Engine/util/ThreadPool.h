#ifndef MINECRAFT_RECREATION_RECREATION_THREADPOOL_H
#define MINECRAFT_RECREATION_RECREATION_THREADPOOL_H
#include <condition_variable>
#include <functional>
#include <queue>
#include <vector>

namespace engine::util
{
class ThreadPool {
    std::vector<std::thread> m_Workers;
    std::queue<std::function<void()>> m_Tasks;
    std::mutex m_QueueMutex;
    std::condition_variable m_Condition;
    bool m_Stop{false};

public:
    explicit ThreadPool(const size_t threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < threads; ++i) {
            m_Workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(this->m_QueueMutex);
                        this->m_Condition.wait(lock, [this] {
                            return this->m_Stop || !this->m_Tasks.empty();
                        });

                        if (this->m_Stop && this->m_Tasks.empty()) return;

                        task = std::move(this->m_Tasks.front());
                        this->m_Tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::lock_guard lock(m_QueueMutex);
            if (m_Stop) return;
            m_Tasks.emplace(std::forward<F>(f));
        }
        m_Condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::lock_guard lock(m_QueueMutex);
            m_Stop = true;
        }
        m_Condition.notify_all();

        for (std::thread &worker : m_Workers) {
            if (worker.joinable()) worker.join();
        }
    }
};
} // engine::util

#endif //MINECRAFT_RECREATION_RECREATION_THREADPOOL_H
