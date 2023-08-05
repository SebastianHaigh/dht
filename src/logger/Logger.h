#ifndef LOGGER_H_
#define LOGGER_H_

#include <atomic>
#include <functional>
#include <iostream>
#include <ostream>
#include <thread>

namespace logging {

class WorkThreadQueue
{
  public:
    WorkThreadQueue() = default;

    bool putWork(std::function<std::string()> workItem)
    {
      size_t putIndex = m_tail.load();
      size_t nextIndex = (putIndex + 1) % 100;

      while (nextIndex == m_head.load())
      {
        return false;
      }

      m_workItems[putIndex] = std::move(workItem);
      m_tail.store(nextIndex);
      return true;
    }

    std::string getNextWork()
    {
      size_t readIndex = m_head.load();
      size_t nextIndex = (readIndex + 1) % 100;

      while (readIndex == m_tail.load())
      {
        return {};
      }

      m_head.store(nextIndex);

      return std::move(m_workItems[readIndex]());
    }

    bool hasWork()
    {
      return (m_head.load(std::memory_order_relaxed) != m_tail.load(std::memory_order_relaxed));
    }

  private:
    std::array<std::function<std::string()>, 100> m_workItems;
    size_t m_ringSize = 100;
    std::atomic<size_t> m_head{0};
    std::atomic<size_t> m_tail{0};
};

class Logger
{
  public:
    explicit Logger(WorkThreadQueue& workQueue,
                    std::string name);

    void log(const std::string& message);

  private:
    const std::string m_name;
    WorkThreadQueue& m_queue;
};

class Log
{
  public:
    Log();
    ~Log();
    Logger makeLogger(const std::string& name);

  private:
    void threadFunction();
    std::ostream& m_output = std::cout;
    std::thread m_thread;
    WorkThreadQueue m_queue;
    bool m_running;
};

} // namespace logging

#endif
