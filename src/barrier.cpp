#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>

std::atomic<long> num_threads(0);

class Barrier {
public:
    Barrier(): count(0),worker_done(false){}

    
    void arrive_and_wait(bool is_worker) {
        long current_num_threads = num_threads.load();
        std::unique_lock<std::mutex> lock(mtx);

        if (is_worker) {
            // Worker waits for all other threads to arrive at the barrier
            cv.wait(lock, [this]() { return count == num_threads - 1; });

            // Worker performs its work
            std::cout << "Worker is performing its task..." << std::endl;
            worker_done = true;

            // Notify all other threads
            cv.notify_all();
        } else {
            // Increment the count
            ++count;

            // If the number of blocked threads equals total threads - 1, notify the worker
            if (count == num_threads - 1) {
                cv.notify_one(); // Wake up the worker thread
            }

            // Wait for the worker to finish its work
            cv.wait(lock, [this]() { return worker_done; });

            // Decrement count for reuse of the barrier
            --count;

            // Notify other threads in case they are waiting
            if (count == 0) {
                worker_done = false; // Reset for the next cycle
            }
        }
    }

private:
    size_t count;
    bool worker_done;
    std::mutex mtx;
    std::condition_variable cv;
};

void thread_function(Barrier &barrier, bool is_worker) {
    while(true){
        barrier.arrive_and_wait(is_worker);
        if (!is_worker) {
            std::cout << "Other thread is proceeding after worker's task." << std::endl;
        }
        else {
            std::cout << "I'm slave thread performing other tasks...." << endl;
            // thread::this_thread::sleep_for(1);
            cout << "Finished my task" << endl;
        }
    }
}

int main() {
    // const size_t num_threads = 5; // Total threads (1 worker + 4 others)
    Barrier barrier();

    std::vector<std::thread> threads;

    // Create the worker thread
    threads.emplace_back(thread_function, std::ref(barrier), true);

    // Create the other threads
    for (size_t i = 0; i < num_threads - 1; ++i) {
        threads.emplace_back(thread_function, std::ref(barrier), false);
    }

    // Join all threads
    for (auto &t : threads) {
        t.join();
    }

    return 0;
}
