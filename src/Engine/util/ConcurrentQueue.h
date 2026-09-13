#ifndef MINECRAFT_RECREATION_RECREATION_CONCURRENTQUEUE_H
#define MINECRAFT_RECREATION_RECREATION_CONCURRENTQUEUE_H
#include <mutex>
#include <queue>

namespace engine::util
{
template<typename T>
class ConcurrentQueue {
    std::queue<T> m_Queue;
    mutable std::mutex m_Mutex;

public:
    void push(T value) {
        std::lock_guard lock(m_Mutex);
        m_Queue.push(std::move(value));
    }

    bool tryPop(T& value) {
        std::lock_guard lock(m_Mutex);
        if (m_Queue.empty()) {
            return false;
        }
        value = std::move(m_Queue.front());
        m_Queue.pop();
        return true;
    }
};
} // engine::util

#endif //MINECRAFT_RECREATION_RECREATION_CONCURRENTQUEUE_H
