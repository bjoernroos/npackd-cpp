#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

/**
 * @brief a pool of std::thread
 */
class ThreadPool
{
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex mutex;
    std::condition_variable cond;
    bool done = false;
public:
    /**
     * @param n number of threads
     */
    ThreadPool(const int n);

    ~ThreadPool();

    /**
     * @brief adds a task
     * @param task a task
     */
    void addTask(std::function<void()> task);
private:
    void process();
};

#endif // THREADPOOL_H
