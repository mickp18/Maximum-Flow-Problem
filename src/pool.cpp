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
int iterations, max_iterations = 3, max_threads = 3;
std::atomic<int> n_running(0);
bool threads_ready = false, done = false;
std::atomic<bool> sink_reached(false);
std::atomic<int> id(0);
bool sink = false;
std::mutex mutex;

class ThreadPool {
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

void ThreadPool::clearQueue() {
    // Clear the jobs queue
    {
        std::lock_guard<std::mutex> lock(mx);
        jobs = {};
    }
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
    mx_print.lock();
    std::cout << "Iteration " << iterations << ": " << "I am slave thread " << id.load() << " doing my tasks here and I'll sleep one sec" << std::endl;
    mx_print.unlock();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    if (id.load()==6){
        sink_reached.store(true);
        std::cout << "Sink reached" << std::endl;
        std::cout << "MINA WAKE UP" << std::endl;
        cv_mina.notify_one();
    }
    else {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int tmp = id.load();
        if (!sink_reached.load()){
            // add 2 jobs for 2 neighbors
            id.fetch_add(1);
            thread_pool->QueueJob([thread_pool] { thread_function(thread_pool); });  
            id.fetch_add(1);
            thread_pool->QueueJob([thread_pool] { thread_function(thread_pool); });  
        }
        mx_print.lock();
        std::cout << "task with id " << tmp << " completed" << std::endl;
        mx_print.unlock();
    }
    return;
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
   
    ThreadPool thread_pool;
    // std::cout << thread_pool.busy() << std::endl;
    std::cout << "starting" << std::endl;
    thread_pool.Start();

    while (true)
    {
        mx_print.lock();
        std::cout << "Iteration " << iterations << ": " << "I am master thread: " << id.load() << " doing my tasks here and I'll sleep one sec" << std::endl;
        mx_print.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // add 2 jobs for 2 neighbors
        id.fetch_add(1);
        thread_pool.QueueJob([&thread_pool] { thread_function(&thread_pool); });
        id.fetch_add(1);
        thread_pool.QueueJob([&thread_pool] { thread_function(&thread_pool); });

        // Wait until job found sink  and Wait until all threads completed running task
        {
            std::unique_lock<std::mutex> lock(mx);
            cv_mina.wait(lock, []
                         { return sink_reached.load() && n_running.load() == 0; });
        }
        // {
        //     std::unique_lock<std::mutex> lock(mx);
        //     cv_mina.wait(lock, []
        //                  { return n_running.load() == 0; });
        // }

        // Execute some instructions
        mx_print.lock();
        std::cout << "sink was reached and everyone stopped working" << std::endl;  
        mx_print.unlock();

        sink_reached.store(false);
        // Termination condition
        if (iterations == max_iterations){
            break;
        }


        mx_print.lock();
        std::cout << "resetting queue..." << std::endl;
        mx_print.unlock();

        thread_pool.clearQueue();

        mx_print.lock();
        std::cout << "resettting labels..." << std::endl;
        mx_print.unlock();
        
        iterations++;
        id.store(0);
        mutex_condition.notify_all();
    }

 
    std::cout << "main ready to end" << std::endl;//MI senti? si, tu mi senti? adesso si
    thread_pool.Stop();        
    std::cout << "Main exiting" << std::endl;


    return 0;
}

// solve 
    // create pool
    // start pool
    // neighbours edges of source
    // while true
        // enqueue jobs for all neighbours
        // waits for a job to reach sink
        // check if flow was augmented
            // if not, break
            // if yes
                // waits remaining tasks to finish
                // destroy queue
                // reset labels
                // recreate queue
                // loop until no more augmenting paths are found
    // stop pool
                

// task function (starnode, endnode)
    // tries to put a label on end node
    // if not possible, return
    // else, 
        // update label  on endnode/startnode
        // if label on sink
            // done = true;
            // augment flow
            // wake up main
            // return
        // else 
            // find neighbours 
            // for each neighbour
                // if (!done) enqueue job for neighbour to do task function (starnode, endnode)
            // return

// avoid threads taking from queue while resetting labels

/*
main                                t1                      t2                                         
waiting on done with cv...          puts label on sink      sleeping..
                                    sink found              
                                    augment
                                    wake main
wakes up                            return                  sleeping...
                                                            if (!done) puts laebl
                                                            return
wait on n_running with another cv
wakes up
destroy queue
reset all labels
recreate queue
loop until no more augmentig paths are found

*/