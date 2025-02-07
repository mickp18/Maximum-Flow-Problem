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
    std::condition_variable mutex_condition;

    // Allows threads to wait on new jobs or termination std::condition_variable mutex_condition;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;

    std::condition_variable cv_completion;
    std::mutex completion_mutex;

    std::atomic<bool> is_processing{false};

    std::atomic<int> active_threads{0};
    std::atomic<int> job_enqueued{0};
    ThreadMonitor monitor;
};

void ThreadPool::Start()
{
    // Max # of threads the system supports
    const int num_threads = 1;
    cout << "THREADS " << num_threads << endl;
    // std::thread::hardware_concurrency();
    // threads.resize(num_threads);
    Logger() << "Starting " << num_threads << " threads";
    for (uint32_t i = 1; i < num_threads+1; i++)
    {
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
            Logger() << "locked queue mutex";

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
            Logger() << "unlcokoed queue mutex";
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
                Logger() << "locked queue mutex";
                Logger() << "MINA WAKE UP FROM LOOP AFTER JOB";
                Logger() << "jobs empty when wake up: " << jobs.empty();

                Logger() << "locked queue mutex";
                cv_completion.notify_one();
            }
        }
    }
}

void ThreadPool::QueueJob(const std::function<void()> &job)
{
    {
        Logger() << "adding job";
        cout << "check if queue mutex is locked" << endl;
        std::unique_lock<std::mutex> lock(queue_mutex);
            
        cout << "i locked the queue mutex" << endl;
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
        Logger() << "locked queue mutex";
        poolbusy = !jobs.empty();
        Logger() << "unlocked queue mutex";
    }
    return poolbusy;
}

void ThreadPool::Stop()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        Logger() << "locked queue mutex";
        should_terminate.store(true);
        Logger() << "unlocked queue mutex";
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
        Logger() << "locked queue mutex";
        getMonitor().updateState("clearing queue");
        std::queue<std::function<void()>> empty;
        std::swap(jobs, empty); // Atomic swap instead of direct assignment
        Logger() << "unlocked queue mutex";
    }
}

void ThreadPool::resetJobEnqueued()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    job_enqueued.store(false);
}

void ThreadPool::waitForCompletion(atomic<int> *pending_jobs)
{
    std::unique_lock<std::mutex> lock(completion_mutex);
    {
        getMonitor().updateState("waiting for completion");
        cv_completion.wait(lock, [this, pending_jobs]() { return   !busy() && !pending_jobs->load() && active_threads.load() == 0; });

    }
    getMonitor().updateState("done waiting for completion");
}

void ThreadPool::notify()
{
    mutex_condition.notify_all();
}