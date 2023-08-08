#include "Logger.h"

namespace logging {

Logger::Logger(WorkThreadQueue& workQueue,
               std::string name) :
  m_name(std::move(name)),
  m_logPrefix("[" + m_name + "] "),
  m_queue(workQueue)
{
}

void Logger::log(std::string&& message)
{
  LogStatement statement;

  statement.m_statement = m_logPrefix + std::move(message);

  m_queue.putWork(std::move(statement));
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

std::unique_ptr<Logger> Log::makeLogger(const std::string& name)
{
  return std::make_unique<Logger>(m_queue, name);
}

void Log::threadFunction()
{
  while (m_running)
  {
    if (m_queue.hasWork())
    {
      m_output << m_queue.getNextWork().m_statement + '\n';
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
  }
}

} // namespace logging
