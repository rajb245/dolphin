#pragma once

#include <atomic>

namespace dik
{
class Flag
{
public:
  explicit Flag(bool initial = false) : m_value(initial) {}

  void set(bool value = true) { m_value.store(value, std::memory_order_release); }
  void clear() { set(false); }
  bool is_set() const { return m_value.load(std::memory_order_acquire); }

  bool test_and_set(bool value = true)
  {
    bool expected = !value;
    return m_value.compare_exchange_strong(expected, value, std::memory_order_acq_rel);
  }

  bool test_and_clear() { return test_and_set(false); }

private:
  std::atomic_bool m_value;
};
}

