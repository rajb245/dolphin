#pragma once

#include <variant>

namespace dik
{
template <typename ResultCode, typename T>
class Result
{
public:
  Result(ResultCode code) : m_variant(code) {}
  Result(const T& value) : m_variant(value) {}
  Result(T&& value) : m_variant(std::move(value)) {}

  explicit operator bool() const { return succeeded(); }
  bool succeeded() const { return std::holds_alternative<T>(m_variant); }

  ResultCode error() const { return std::get<ResultCode>(m_variant); }

  const T& operator*() const { return std::get<T>(m_variant); }
  const T* operator->() const { return &std::get<T>(m_variant); }

  T& operator*() { return std::get<T>(m_variant); }
  T* operator->() { return &std::get<T>(m_variant); }

private:
  std::variant<ResultCode, T> m_variant;
};
}

