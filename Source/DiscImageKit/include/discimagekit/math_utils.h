#pragma once

#include <limits>
#include <type_traits>

namespace dik::math
{
inline constexpr bool is_power_of_two(unsigned long long value)
{
  return value != 0 && (value & (value - 1)) == 0;
}

template <typename T, typename U>
T saturating_cast(U value)
{
  static_assert(std::is_arithmetic_v<T> && std::is_arithmetic_v<U>);
  if constexpr (std::is_floating_point_v<U> && std::is_integral_v<T>)
  {
    if (value >= static_cast<U>(std::numeric_limits<T>::max()))
      return std::numeric_limits<T>::max();
    if (value <= static_cast<U>(std::numeric_limits<T>::min()))
      return std::numeric_limits<T>::min();
    return static_cast<T>(value);
  }
  else if constexpr (std::is_integral_v<U> && std::is_integral_v<T>)
  {
    if (value > static_cast<U>(std::numeric_limits<T>::max()))
      return std::numeric_limits<T>::max();
    if (value < static_cast<U>(std::numeric_limits<T>::min()))
      return std::numeric_limits<T>::min();
    return static_cast<T>(value);
  }
  else
  {
    return static_cast<T>(value);
  }
}
}

