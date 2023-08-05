#include "Logger.h"

namespace logging {

Logger::Logger(WorkThreadQueue& workQueue,
               std::string name) :
  m_name(std::move(name)),
  m_queue(workQueue)
{
}

void Logger::log(const std::string& message)
{
  std::function<std::string()> logTask = [name = m_name, message] () -> std::string
  {
    return "[" + name + "] " +  message;
  };

  m_queue.putWork(logTask);
}

Log::Log() :
  m_running(true)
{
  m_thread = std::thread{&Log::threadFunction, this};
}

Log::~Log()
{
  m_running = false;
  m_thread.join();
}

Logger Log::makeLogger(const std::string& name)
{
  return Logger{ m_queue, name };
}

void Log::threadFunction()
{
  while (m_running)
  {
    if (m_queue.hasWork())
    {
      m_output << m_queue.getNextWork() + '\n';
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
  }
}

} // namespace logging
