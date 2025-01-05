// pool
#include <iostream>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>
#include <chrono>
#include <condition_variable>
#include <atomic>
// using namespace std;

std::condition_variable mutex_condition;

std::condition_variable cv_slaves, cv_mina, cv_main;
std::mutex mx, mx_print;
int iterations, max_iterations = 3, max_threads = 5;
std::atomic<int> num_threads;
bool threads_ready = false, done = false;

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
        active_thread.join();
    }
    threads.clear();
}



/**
 * Function that orchestrates the execution of master and slave threads within a thread pool.
 * The master thread initializes the task and waits for all slave threads to reach a certain
 * condition before proceeding with its own execution. Slave threads wait for a signal from
 * the master thread to proceed with their tasks. Once all threads complete their tasks, the
 * process can either iterate or terminate based on a specified condition.
 *
 * @param thread_pool Pointer to the ThreadPool object managing the threads.
 */

void thread_function(ThreadPool *thread_pool) {
    if (num_threads.load() == 0) {
        while (true) {
            mx_print.lock();
            std::cout << "Iteration " << iterations << ": " << "I am master thread " << num_threads.load() << " doing my tasks here and I'll sleep one sec" << std::endl; 
            mx_print.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(1));

            num_threads.fetch_add(1);
            thread_pool->QueueJob([thread_pool] { thread_function(thread_pool); });                
            

            // Wait until all threads have reached the condition
            {
                std::unique_lock<std::mutex> lock(mx);
                cv_mina.wait(lock, [] { return num_threads.load() == max_threads; });
            }

            // Execute some instructions
            mx_print.lock();
            std::cout << "Iteration " << iterations << ": " << "I am master thread " << num_threads.load() << " and I've been woken up since all threads have reached the condition" << std::endl; 
            mx_print.unlock();
            
            // Termination condition, master exists without waking up other threads
            if (iterations == max_iterations) {
                break;
            }

            iterations++;
            num_threads.store(0);

            // Second phase: Unlock all threads
            {
                std::lock_guard<std::mutex> lock(mx);
                threads_ready = true;
            }
            cv_slaves.notify_all();      
        }
        mx_print.lock();
        std::cout << "Iteration " << iterations << ": " << "Master thread exiting" << std::endl;
        mx_print.unlock();
        std::lock_guard<std::mutex> lock(mx);
        done = true;
        cv_main.notify_one();
    } else {
        mx_print.lock();
        std::cout << "Iteration " << iterations << ": " << "I am slave thread " << num_threads.load() << " doing my tasks here and I'll sleep one sec" << std::endl;
        mx_print.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // First phase: Wait until all threads reach this point
        {
            std::unique_lock<std::mutex> lock(mx);
            if (num_threads.load() == max_threads) {
                // Notify TA that all threads have reached the condition
                cv_mina.notify_one();
            } else {
                num_threads.fetch_add(1);
                thread_pool->QueueJob([thread_pool] { thread_function(thread_pool); });
            }
            mx_print.lock();
            std::cout << "Iteration " << iterations << ": " << "I am slave thread " << num_threads.load() << " waiting to be woken up" << std::endl;
            mx_print.unlock();
            // Wait until TA processes and signals to proceed
            cv_slaves.wait(lock, [] { return threads_ready; });
        }
        mx_print.lock();
        std::cout << "I am slave thread " << num_threads.load() << " and I was woken up, I'll exit" << std::endl;
        mx_print.unlock();
    }

}

/**
 * @brief Main function of the program.
 * 
 * This function creates a thread pool of size equal to the number of hardware threads
 * available and starts it. Then it submits a job to the thread pool and waits until
 * the master thread finishes. Finally, it stops the thread pool and exits.
 * 
 * @return 0 on success.
 */
int main(){
    // pool 
        // 
    // size_t num_threads = std::thread::hardware_concurrency();
    ThreadPool thread_pool;
    std::cout << thread_pool.busy() << std::endl;
    std::cout << "starting" << std::endl;
    num_threads.store(0);
    thread_pool.Start();
    thread_pool.QueueJob([&thread_pool] { thread_function(&thread_pool); });


    // if (thread_pool.busy()){
    //     std::cout << thread_pool.busy() << std::endl;
    //     thread_pool.Stop();        
    // }
    
    // Wait until master thread finishes
    {
        std::unique_lock<std::mutex> lock(mx);
        cv_main.wait(lock, [] { return done; });    // buonasera hello hey HEY hEy HeY Hey HEy hEY heY hello echo hey
    }
    std::cout << "main ready to end" << std::endl;//MI senti? si, tu mi senti? adesso si
    thread_pool.Stop();        
    std::cout << "Main exiting" << std::endl;


    return 0;
}