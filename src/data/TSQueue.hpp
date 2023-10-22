#ifndef __TSQueue__
#define __TSQueue__
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>

// Thread-safe queue
template <typename T>
class TSQueue {
private:
    std::queue<T>           m_queue;
    std::mutex              m_mutex;
    std::condition_variable m_cond;

public:
    void push(T item)
    {

        // Acquire lock
        std::unique_lock<std::mutex> lock(m_mutex);

        // Add item
        m_queue.push(item);

        // Notify one thread that
        // is waiting
        m_cond.notify_one();
    }

    int size()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        int                          size = m_queue.size();
        m_cond.notify_one();
        return size;
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // wait until queue is not empty
        m_cond.wait(lock,
            [this]() { return !m_queue.empty(); });

        T item = m_queue.front();
        m_queue.pop();

        return item;
    }
};
#endif