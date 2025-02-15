// pool
#include <iostream>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>
#include <condition_variable>
#include <atomic>
#include "ThreadMonitor.hpp"

class ThreadPool
{
public:
    void Start();
    void QueueJob(const std::function<void()> &job);
    void Stop();
    bool busy();
    void clearQueue();
    // bool isProcessing() const { return is_processing.load(); }
    int getActiveThreads() const { return active_tasks.load(); }
    // bool jobEnqueued() const { return job_enqueued.load(); }
    // void resetJobEnqueued();
    void waitForCompletion();
    void notify();
    // ThreadMonitor &getMonitor() { return monitor; }

private:
    void ThreadLoop();
    // Tells threads to stop looking for jobs
    std::atomic<bool> should_terminate{false};

    // Prevents data races to the job queue
    std::mutex queue_mutex;
    std::condition_variable mutex_condition;

    // Allows threads to wait on new jobs or termination std::condition_variable mutex_condition;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;

    std::condition_variable cv_completion;
    std::mutex completion_mutex;

    std::atomic<int> active_tasks{0};

    ThreadMonitor monitor;
};

void ThreadPool::Start()
{
    // Max # of threads the system supports
    const int num_threads = std::thread::hardware_concurrency();
    cout << "THREADS " << num_threads << endl;
    // std::thread::hardware_concurrency();
    // threads.resize(num_threads);
    // Logger() << "Starting " << num_threads << " threads";
    for (uint32_t i = 1; i < num_threads + 1; i++)
    {
        threads.emplace_back(&ThreadPool::ThreadLoop, this);
    }
    // Logger() << "Started " << threads.size() << " threads";
    return;
}

void ThreadPool::ThreadLoop()
{
    while (true)
    {
        std::function<void()> job;
        // std::atomic<bool> has_job{false};
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            // Logger() << "locked queue mutex";

            mutex_condition.wait(lock, [this]()
                {
            // Logger() << "thread IS WAITING seeing " << !jobs.empty() << should_terminate.load(); 
            return (!jobs.empty() || should_terminate.load()); });
            // Logger() << "thread IS NOT WAITING seeing " << !jobs.empty() << should_terminate.load();

            if (should_terminate.load() && jobs.empty()) // could check also if jobs.empty()
            {
                return;
            }

            // getMonitor().updateState("getting job");
            job = std::move(jobs.front());
            jobs.pop();
            // has_job.store(true);

            // Logger() << "unlcokoed queue mutex";
        }

        job();
        size_t remaining = --active_tasks; // This line decrements the active_tasks atomic variable and assigns the result to remaining. It is equivalent to remaining = active_tasks - 1;, but is atomic, meaning it is thread-safe.
        if (remaining == 0)
        {
            lock_guard<mutex> lock(queue_mutex);
            if (jobs.empty())
            {
                lock_guard<std::mutex> completion_lock(completion_mutex);
                // cv_completion.notify_one();
                cv_completion.notify_all();
            }
        }
    }
}

void ThreadPool::QueueJob(const std::function<void()> &job)
{

    std::lock_guard<std::mutex> lock(queue_mutex);

    // cout << "i locked the queue mutex" << endl;
    if (should_terminate.load())
        return; // Prevent job addition after termination
    jobs.push(job);
    active_tasks.fetch_add(1);
    // Logger() << "added job";

    mutex_condition.notify_one();
}

bool ThreadPool::busy()
{
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        // Logger() << "locked queue mutex";
        poolbusy = !jobs.empty();
        // Logger() << "unlocked queue mutex";
    }
    return poolbusy;
}

void ThreadPool::Stop()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        // Logger() << "locked queue mutex";
        should_terminate.store(true);
        // Logger() << "unlocked queue mutex";
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
        std::unique_lock<std::mutex> lock(queue_mutex);
        // Logger() << "locked queue mutex";
        // getMonitor().updateState("clearing queue");
        std::queue<std::function<void()>> empty;
        std::swap(jobs, empty); // Atomic swap instead of direct assignment
        // Logger() << "unlocked queue mutex";
    }
}

// called by the main thread, it waits for threads to finish all jobs in the queue
void ThreadPool::waitForCompletion()
{
    // std::unique_lock<std::mutex> lock(completion_mutex);
    // {
    //     // getMonitor().updateState("waiting for completion");
    //     cv_completion.wait(lock, [this]()
    //                        { return jobs.empty() && active_tasks.load() == 0; });
    // }
    unique_lock<mutex> lock_queue(queue_mutex);
    if (jobs.empty() && active_tasks.load() == 0){
        return;
    }
    lock_queue.unlock();

    unique_lock<mutex> lock_completion(completion_mutex);
    cv_completion.wait(lock_completion);


    // getMonitor().updateState("done waiting for completion");
}

void ThreadPool::notify()
{
    mutex_condition.notify_all();
}