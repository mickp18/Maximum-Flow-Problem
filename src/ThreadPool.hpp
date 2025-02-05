// pool
#include <iostream>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>
#include <condition_variable>
#include <atomic>
#include "ThreadLogger.hpp"
#include "ThreadMonitor.hpp"

class ThreadPool
{
public:
    void Start();
    void QueueJob(const std::function<void()> &job);
    void Stop();
    bool busy();
    void clearQueue();
    bool isProcessing() const { return is_processing.load(); }
    int getActiveThreads() const { return active_threads.load(); }
    bool jobEnqueued() const { return job_enqueued.load(); }
    void resetJobEnqueued();
    void waitForCompletion(atomic<int> *pending_jobs);
    void notify();
    ThreadMonitor &getMonitor() { return monitor; }

private:
    void ThreadLoop();
    // Tells threads to stop looking for jobs
    std::atomic<bool> should_terminate{false};
    // Prevents data races to the job queue
    std::mutex queue_mutex;
    // Allows threads to wait on new jobs or termination std::condition_variable mutex_condition;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;

    std::condition_variable cv_mina;
    std::condition_variable mutex_condition;
    std::mutex completion_mutex;
    std::atomic<bool> is_processing{false};
    std::atomic<int> active_threads{0};
    std::atomic<bool> job_enqueued{false};
    ThreadMonitor monitor;
};

void ThreadPool::Start()
{
    // Max # of threads the system supports
    const uint32_t num_threads = 2;
    // std::thread::hardware_concurrency();
    // threads.resize(num_threads);
    Logger() << "Starting " << num_threads << " threads";
    for (uint32_t i = 0; i < num_threads; i++)
    {
        // threads.at(i) = std::thread(&ThreadLoop);
        threads.emplace_back(&ThreadPool::ThreadLoop, this);
    }
    Logger() << "Started " << threads.size() << " threads";
    return;
}

void ThreadPool::ThreadLoop()
{
    while (true)
    {
        std::function<void()> job;
        std::atomic<bool> has_job{false};
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // Wait until there are jobs, termination is requested, or sink is reached

            mutex_condition.wait(lock, [this]()
                                 {Logger() << "thread IS WAITING seeing " << !jobs.empty() << should_terminate.load(); return (!jobs.empty() || should_terminate.load()); });
            Logger() << "thread IS NOT WAITING seeing " << !jobs.empty() << should_terminate.load();

            if (should_terminate.load())
            {
                return;
            }

            if (!jobs.empty())
            {
                getMonitor().updateState("getting job");
                job = std::move(jobs.front());
                jobs.pop();
                has_job.store(true);
            }
        }

        if (has_job.load())
        {
            is_processing.store(true);
            active_threads.fetch_add(1);
            if (job)
                getMonitor().updateState("processing job");
                job();
            active_threads.fetch_sub(1);
            is_processing.store(active_threads.load() > 0);
            {
                scoped_lock lock(queue_mutex, completion_mutex);
                Logger() << "MINA WAKE UP FROM LOOP AFTER JOB";
                Logger() << "jobs empty when wake up: " << jobs.empty();

                cv_mina.notify_one();
            }
        }
    }
}

void ThreadPool::QueueJob(const std::function<void()> &job)
{
    {
        Logger() << "adding job";
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (should_terminate.load())
            return; // Prevent job addition after termination
        jobs.push(job);
        Logger() << "added job";
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
        should_terminate.store(true);
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
        getMonitor().updateState("clearing queue");
        std::queue<std::function<void()>> empty;
        std::swap(jobs, empty); // Atomic swap instead of direct assignment
    }
}

void ThreadPool::resetJobEnqueued()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    job_enqueued.store(false);
}

void ThreadPool::waitForCompletion(atomic<int> *pending_jobs)
{
    {
        std::unique_lock<std::mutex> lock(completion_mutex);
        while (!cv_mina.wait_for(lock, std::chrono::seconds(5), [this, pending_jobs]()

                                 {
                    Logger() << "mina is  checking " << !this->busy() << !this->isProcessing() << this->getActiveThreads() << pending_jobs->load();
                    bool wake_up = (!this->busy() &&
                                    !this->isProcessing() &&
                                    this->getActiveThreads() == 0 &&
                                    pending_jobs->load() == 0);
                    Logger() << "mina woke up since has found " << !this->busy() << !this->isProcessing() << this->getActiveThreads() << pending_jobs->load();
                    return wake_up; }))
        {
            // If we've waited more than 5 seconds, dump thread states
            std::cout << "\n=== Thread States after 5s wait ===" << std::endl;
            monitor.dumpState();

            // Log additional info
            std::cout << "Pending jobs: " << pending_jobs->load() << std::endl;
            std::cout << "Queue size: " << jobs.size() << std::endl;
            std::cout << "Active threads: " << active_threads.load() << std::endl;
        }
    }
}

void ThreadPool::notify()
{
    mutex_condition.notify_all();
}