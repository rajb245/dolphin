#pragma once

#include <optional>
#include <utility>

namespace dik
{
template <typename Callable>
class ScopeGuard
{
public:
  explicit ScopeGuard(Callable&& fn) : m_finalizer(std::forward<Callable>(fn)) {}
  ScopeGuard(ScopeGuard&& other) noexcept : m_finalizer(std::move(other.m_finalizer))
  {
    other.m_finalizer.reset();
  }

  ~ScopeGuard() { exit(); }

  void dismiss() { m_finalizer.reset(); }

  void exit()
  {
    if (m_finalizer)
    {
      (*m_finalizer)();
      m_finalizer.reset();
    }
  }

  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
  std::optional<Callable> m_finalizer;
};
}

