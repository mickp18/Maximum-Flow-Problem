#include <iostream>
#include <mutex>
#include <thread>
#include <sstream>
#include <chrono>
#include <vector>
#include <map>

// Thread-safe logger
class Logger
{
private:
    static std::mutex cout_mutex;
    std::stringstream buffer;

public:
    template <typename T>
    Logger &operator<<(const T &data)
    {
        //buffer << "threadID " << std::this_thread::get_id() << ": " << data << '\n';
        buffer << data;
        return *this;
    }

    // Destructor ensures atomic writing of the complete line
    ~Logger()
    {
        buffer << '\n';
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << buffer.str();
    }
};

