// pool
#include <iostream>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>
#include <chrono>
#include <condition_variable>
// using namespace std;

std::condition_variable mutex_condition;

class ThreadPool {
public:
    void Start();
    void QueueJob(const std::function<void()> &job);
    void Stop();
    bool busy();

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
            mutex_condition.wait(lock, [this]
                                 { return !jobs.empty() || should_terminate; });
            if (should_terminate)
            {
                return;
            }
            job = jobs.front();
            jobs.pop();
        }
        job();
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
    std::cout << "Stopping threads" << std::endl;
        active_thread.join();
    }
    threads.clear();
}

int main(){
    ThreadPool thread_pool;
    // pool 
        // 
    // size_t num_threads = std::thread::hardware_concurrency();

    std::cout << thread_pool.busy() << std::endl;

    thread_pool.QueueJob([] { 
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Hello from thread " << std::this_thread::get_id() << std::endl;
    });
    
    thread_pool.QueueJob([] { 
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Hello from thread " << std::this_thread::get_id() << std::endl;
    });

    thread_pool.Start();

    // if (thread_pool.busy()){
    //     std::cout << thread_pool.busy() << std::endl;
    //     thread_pool.Stop();        
    // }
    
    while (thread_pool.busy()){
        std::cout << thread_pool.busy() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "before" << std::endl;
    thread_pool.Stop();        
    std::cout << "something" << std::endl;


    return 0;
}