#include "Common/CommonTypes.h"
#include "UICommon/UICommon.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

#include <fmt/format.h>

namespace UICommon
{
std::string FormatSize(u64 bytes, int decimals)
{
  static constexpr std::array<const char*, 7> units = {"B", "KiB", "MiB", "GiB",
                                                       "TiB", "PiB", "EiB"};

  const u64 safe_bytes = std::max<u64>(bytes, 1);
  const int unit = std::min<int>(units.size() - 1,
                                 static_cast<int>(std::log2(static_cast<long double>(safe_bytes)) / 10.0L));

  const long double unit_size = std::pow(2.0L, unit * 10);
  const long double value = static_cast<long double>(bytes) / unit_size;

  return fmt::format("{:.{}Lf} {}", value, decimals, units[unit]);
}
}  // namespace UICommon
