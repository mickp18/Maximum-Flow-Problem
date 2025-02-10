#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <atomic>
#include <iostream>

class ThreadPool
{
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable queue_condition;

    std::mutex completion_mutex;
    std::condition_variable completion_condition;

    // Single counter for all tasks (both queued and executing)
    std::atomic<size_t> total_tasks{0};

    bool stop_flag{false};

public:
    ThreadPool(size_t num_threads)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            workers.emplace_back([this]
                                 { thread_loop(); });
        }
    }

    void queue_job(std::function<void()> job)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.push(job);
            total_tasks++; // Increment counter for new task
        }
        queue_condition.notify_one();
    }

    void wait_for_completion()
    {
        while (true)
        {
            // First acquire queue_mutex
            std::unique_lock<std::mutex> queue_lock(queue_mutex);

            if (tasks.empty() && total_tasks == 0)
            {
                return;
            }

            // Release queue_mutex before waiting
            queue_lock.unlock();

            // Wait with just completion_mutex
            std::unique_lock<std::mutex> completion_lock(completion_mutex);
            completion_condition.wait(completion_lock);
        }
    }

    void thread_loop()
    {
        while (true)
        {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                queue_condition.wait(lock, [this]
                                     { return !tasks.empty() || stop_flag; });

                if (stop_flag && tasks.empty())
                {
                    return;
                }

                job = std::move(tasks.front());
                tasks.pop();
            }

            job();

            size_t remaining = --total_tasks;
            if (remaining == 0)
            {
                // First acquire queue_mutex
                std::lock_guard<std::mutex> queue_lock(queue_mutex);
                if (tasks.empty())
                {
                    // Then acquire completion_mutex
                    std::lock_guard<std::mutex> completion_lock(completion_mutex);
                    completion_condition.notify_all();
                }
            }
        }
    }
    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop_flag = true;
        }
        queue_condition.notify_all();

        for (auto &worker : workers)
        {
            worker.join();
        }
    }
};

void recursive_task(int depth, ThreadPool &pool)
{
    std::cout << std::this_thread::get_id() << " " << depth << std::endl;
    if (depth > 0)
    {
        pool.queue_job([&pool, depth]()
                       { recursive_task(depth - 1, pool); });
        pool.queue_job([&pool, depth]()
                       { recursive_task(depth - 1, pool); });
    }
    return;
};

int main()
{
    ThreadPool pool(4);


    for (int i = 0; i < 4; i++){
        pool.queue_job([&]()
                       { recursive_task(3, pool); });
    
        pool.wait_for_completion(); // Will wait for ALL generated tasks
        std::cout << "DONE" << std::endl;

    }
    // Start with one task that will generate more
    pool.stop();
    return 0;
}