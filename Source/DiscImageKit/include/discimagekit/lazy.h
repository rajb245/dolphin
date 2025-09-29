#pragma once

#include <functional>
#include <utility>
#include <variant>

namespace dik
{
template <typename T>
class Lazy
{
public:
  Lazy() : m_value(T{}) {}
  explicit Lazy(std::variant<T, std::function<T()>> value) : m_value(std::move(value)) {}

  Lazy& operator=(std::variant<T, std::function<T()>> value)
  {
    m_value = std::move(value);
    return *this;
  }

  const T& operator*() const { return *compute(); }
  const T* operator->() const { return compute(); }
  T& operator*() { return *compute(); }
  T* operator->() { return compute(); }

private:
  T* compute() const
  {
    if (!std::holds_alternative<T>(m_value))
      m_value = std::get<std::function<T()>>(m_value)();
    return &std::get<T>(m_value);
  }

  mutable std::variant<T, std::function<T()>> m_value;
};
}

