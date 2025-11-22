#ifndef WRITE_QUEUE_H
#define WRITE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

struct WriteTask
{
    std::string filename;
    std::vector<unsigned char> data;
};

class WriteQueue
{
  private:
    std::queue<WriteTask> tasks;
    std::mutex mutex;
    std::condition_variable cv;
    bool done = false;

  public:
    void push(std::string filename, std::vector<unsigned char> data);
    bool pop(WriteTask &task);
    void finish();
};

#endif // WRITE_QUEUE_H
