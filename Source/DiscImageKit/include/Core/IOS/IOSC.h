#pragma once

#include <array>
#include <cstdint>

namespace IOS
{
namespace HLE
{
class IOSC
{
public:
  enum class ConsoleType
  {
    Retail,
    RVT,
  };

  static constexpr std::array<std::uint32_t, 2> COMMON_KEY_HANDLES = {4, 11};
};
}  // namespace HLE
}  // namespace IOS
