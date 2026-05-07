#ifndef __TSQueue__
#define __TSQueue__

#include <condition_variable>
#include <mutex>
#include <queue>

// Added SIZE_LIMIT as a template parameter
template <typename T, std::size_t SIZE_LIMIT = 100> 
class TSQueue {
private:
    std::queue<T>           m_queue;
    std::mutex              m_mutex;
    std::condition_variable m_cond;

public:
    void push(T item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Check if the queue is full before adding
        if (m_queue.size() >= SIZE_LIMIT) {
            return; // Ignore the entry
        }

        m_queue.push(std::move(item));
        
        // Notify a waiting pop() that data is available
        m_cond.notify_one();
    }

    std::size_t size()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Wait until queue is not empty
        m_cond.wait(lock, [this]() { return !m_queue.empty(); });

        T item = std::move(m_queue.front());
        m_queue.pop();

        return item;
    }
};

#endif