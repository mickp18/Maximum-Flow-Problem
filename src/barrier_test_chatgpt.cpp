#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

std::mutex mtx;
std::condition_variable cv_main, cv_threads;
int count = 0;
bool main_ready = false;
bool threads_ready = false;

void thread_worker(int id, int N) {
    // First phase: Wait until all threads reach this point
    {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        if (count == N) {
            // Notify TA that all threads have reached the condition
            cv_main.notify_one();
        }

        // Wait until TA processes and signals to proceed
        cv_threads.wait(lock, [] { return threads_ready; });
    }

    // Continue with other tasks after TA signals
    std::cout << "Thread " << id << " proceeding after TA.\n";
}

void thread_main(int N) {
    // Wait until all threads have reached the condition
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv_main.wait(lock, [N] { return count == N; });
    }

    // Execute some instructions
    std::cout << "TA executing its instructions.\n";

    // Second phase: Unlock all threads
    {
        std::lock_guard<std::mutex> lock(mtx);
        threads_ready = true;
    }
    cv_threads.notify_all();

    std::cout << "TA finished execution, allowing all threads to continue.\n";
}

int main() {
    const int N = 5; // Number of threads
    std::vector<std::thread> threads;

    // Create worker threads
    for (int i = 1; i <= N; ++i) {
        threads.emplace_back(thread_worker, i, N);
    }

    // Create and execute main thread
    std::thread main_thread(thread_main, N);

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }
    main_thread.join();

    return 0;
}
