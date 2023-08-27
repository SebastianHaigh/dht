#include "ThreadPool.h"

namespace odd {

ThreadJoiner::ThreadJoiner(std::vector<std::thread>& threads) : m_threads(threads)
{
}

ThreadJoiner::~ThreadJoiner()
{
  int toJoin = m_threads.size();
  int joined{ 0 };

  for (auto& thread : m_threads)
  {
    if (thread.joinable())
    {
      joined++;
      thread.join();
    }
  }
}

ThreadPool::ThreadPool() : m_done(false), m_joiner(m_threads)
{
  auto threadCount = std::thread::hardware_concurrency();

  try
  {
    for (int i = 0; i < threadCount; i++)
    {
      m_threads.push_back(std::thread{ &ThreadPool::workerThread, this });
    }
  }
  catch (...)
  {
    m_done = true;
    throw;
  }
}

ThreadPool::~ThreadPool()
{
  m_done = true;
}

void ThreadPool::workerThread()
{
  while (!m_done)
  {
    FunctionWrapper task;

    if (m_workQueue.tryPop(task))
    {
      task();
      continue;
    }

    std::this_thread::yield();
  }
}

} // namespace odd

