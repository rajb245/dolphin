#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "discimagekit/flag.h"

namespace dik
{
class Event
{
public:
  void Set()
  {
    if (m_flag.test_and_set())
    {
      {
        std::lock_guard<std::mutex> lk(m_mutex);
      }
      m_condition.notify_one();
    }
  }

  void Wait()
  {
    if (m_flag.test_and_clear())
      return;

    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock, [&] { return m_flag.test_and_clear(); });
  }

  template <class Rep, class Period>
  bool WaitFor(const std::chrono::duration<Rep, Period>& duration)
  {
    if (m_flag.test_and_clear())
      return true;

    std::unique_lock<std::mutex> lock(m_mutex);
    return m_condition.wait_for(lock, duration, [&] { return m_flag.test_and_clear(); });
  }

  void Reset() { m_flag.clear(); }

private:
  Flag m_flag;
  std::condition_variable m_condition;
  std::mutex m_mutex;
};
}

