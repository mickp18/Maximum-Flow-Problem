#include <iostream>
#include <mutex>
#include <thread>
#include <sstream>
#include <chrono>
#include <vector>
#include <map>
#include "ThreadLogger.hpp"

std::mutex Logger::cout_mutex;

// Thread state tracker
class ThreadMonitor
{
private:
    std::mutex state_mutex;
    std::map<std::thread::id, std::string> thread_states;

public:
    void updateState(const std::string &state)
    {
/*         std::lock_guard<std::mutex> lock(state_mutex);
        thread_states[std::this_thread::get_id()] = state; */
    }

    void dumpState()
    {
/*         std::lock_guard<std::mutex> lock(state_mutex);
        Logger() << "=== Thread States ===";
        for (const auto &[thread_id, state] : thread_states)
        {
            Logger() << "Thread " << thread_id << ": " << state;
        } */
    }
};
