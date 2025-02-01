// pool
#include <iostream>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>
#include <condition_variable>
#include <atomic>

std::condition_variable mutex_condition;

std::condition_variable cv_mina;
std::mutex mx;
std::atomic<int> n_running(0);
std::atomic<bool> sink_reached(false);

class ThreadPool
{
public:
    void Start();
    void QueueJob(const std::function<void()> &job);
    void Stop();
    bool busy();
    void clearQueue();

private:
    void ThreadLoop();
    // Tells threads to stop looking for jobs
    bool should_terminate = false;
    // Prevents data races to the job queue
    std::mutex queue_mutex;
    // Allows threads to wait on new jobs or termination std::condition_variable mutex_condition;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;
};

void ThreadPool::Start()
{
    // Max # of threads the system supports
    const uint32_t num_threads =
        std::thread::hardware_concurrency();
    // threads.resize(num_threads);
    std::cout << "Starting " << num_threads << " threads" << std::endl;
    for (uint32_t i = 0; i < num_threads; i++)
    {
        // threads.at(i) = std::thread(&ThreadLoop);
        threads.emplace_back(&ThreadPool::ThreadLoop, this);
    }
    std::cout << "Started " << threads.size() << " threads" << std::endl;
    return;
}

void ThreadPool::ThreadLoop()
{
    while (true)
    {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // Wait until there are jobs, termination is requested, or sink is reached
            mutex_condition.wait(lock, [this]()
                                 { return (!jobs.empty() || should_terminate) && !sink_reached.load(); });
            // return !jobs.empty() || should_terminate || !sink_reached.load();});
            //          FALSE    ||       FALSE       ||  TRUE    return true -> wake up
            if (should_terminate)
            {
                return;
            }

            if (!jobs.empty())
            {
                job = jobs.front();
                jobs.pop();
            }
        }

        n_running.fetch_add(1);
        job();
        n_running.fetch_sub(1);
        cv_mina.notify_one();
    }
}

void ThreadPool::QueueJob(const std::function<void()> &job)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        jobs.push(job);
    }
    mutex_condition.notify_one();
}

bool ThreadPool::busy()
{
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        poolbusy = !jobs.empty();
    }
    return poolbusy;
}

void ThreadPool::Stop()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        should_terminate = true;
    }
    mutex_condition.notify_all();
    for (std::thread &active_thread : threads)
    {
        active_thread.join();
    }
    threads.clear();
}

void ThreadPool::clearQueue()
{
    // Clear the jobs queue
    {
        std::lock_guard<std::mutex> lock(mx);
        jobs = {};
    }
}