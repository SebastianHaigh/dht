#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <future>
#include <memory>
#include <queue>
#include <vector>

namespace odd {

template<typename T>
class ThreadSafeQueue
{
  public:
    ThreadSafeQueue()
    {
    }

    void push(T value)
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_data.push(std::move(value));
      m_conditionVariable.notify_one();
    }

    void waitAndPop(T& value)
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_conditionVariable.wait(lock, [this]{ return not m_data.empty(); });
    }

    std::shared_ptr<T> waitAndPop()
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_conditionVariable.wait(lock, [this]{ return not m_data.empty(); });
      std::shared_ptr<T> result{std::make_shared<T>(std::move(m_data.front()))};
      m_data.pop();
      return result;
    }

    bool tryPop(T& value)
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      if (m_data.empty()) return false;

      value = std::move(m_data.front());
      m_data.pop();
      return true;
    }

    std::shared_ptr<T> tryPop()
    {
      std::lock_guard<std::mutex> lock(m_mutex);

      if (m_data.empty()) return std::shared_ptr<T>();

      std::shared_ptr<T> result{ std::make_shared<T>(std::move(m_data.front())) };

      m_data.pop();
      return result;
    }

    bool empty() const
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_data.empty();
    }

  private:
    mutable std::mutex m_mutex;
    std::queue<T> m_data;
    std::condition_variable m_conditionVariable;
};

class ThreadJoiner
{
  public:
    explicit ThreadJoiner(std::vector<std::thread>& threads);

    ~ThreadJoiner();

  private:
      std::vector<std::thread>& m_threads;
};

class FunctionWrapper
{
  public:
    FunctionWrapper() = default;

    template<typename F>
    FunctionWrapper(F&& f) : m_impl(std::make_unique<impl_type<F>>(std::move(f))) {}

    void operator()() { m_impl->call(); }

    FunctionWrapper(FunctionWrapper&& other) : m_impl(std::move(other.m_impl))
    {
    }

    FunctionWrapper& operator=(FunctionWrapper&& other)
    {
      m_impl = std::move(other.m_impl);
      return *this;
    }

    FunctionWrapper(const FunctionWrapper&) = delete;
    FunctionWrapper(FunctionWrapper&) = delete;
    FunctionWrapper& operator=(const FunctionWrapper&) = delete;
    
  private:
    struct impl_base
    {
      virtual void call() = 0;
      virtual ~impl_base() {};
    };

    std::unique_ptr<impl_base> m_impl;
    
    template<typename F>
    struct impl_type : impl_base
    {
      F m_f;
      impl_type(F&& f) : m_f(std::move(f)) {}
      void call() { m_f(); }
    };
};

class ThreadPool
{
  public:
    ThreadPool();
    ~ThreadPool();

    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> post(FunctionType fn)
    {
      typedef typename std::result_of<FunctionType()>::type ResultType;

      std::packaged_task<ResultType()> task(std::move(fn));
      std::future<ResultType> result(task.get_future());

      m_workQueue.push(std::move(task));

      return result;
    }

    template<typename FunctionType>
    void postWithoutResult(FunctionType fn)
    {
      m_workQueue.push(std::move(fn));
    }

  private:
    std::atomic_bool m_done;
    ThreadSafeQueue<FunctionWrapper> m_workQueue;
    std::vector<std::thread> m_threads;
    ThreadJoiner m_joiner;

    void workerThread();

};

} // namespace odd

#endif // THREAD_POOL_H_
