#include "write_queue.h"

void WriteQueue::push(std::string filename, std::vector<unsigned char> data)
{
    std::lock_guard<std::mutex> lock(mutex);
    tasks.push({std::move(filename), std::move(data)});
    cv.notify_one();
}

bool WriteQueue::pop(WriteTask &task)
{
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this]
            { return !tasks.empty() || done; });
    if (tasks.empty() && done)
        return false;
    task = std::move(tasks.front());
    tasks.pop();
    return true;
}

void WriteQueue::finish()
{
    std::lock_guard<std::mutex> lock(mutex);
    done = true;
    cv.notify_all();
}
