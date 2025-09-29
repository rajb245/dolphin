#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include <fmt/format.h>

#include "discimagekit/types.h"

namespace dik::byte_utils
{
namespace detail
{
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
constexpr bool kIsLittleEndian = false;
#else
constexpr bool kIsLittleEndian = true;
#endif

template <typename T>
constexpr T byteswap(T value)
{
  static_assert(std::is_integral_v<T>, "byteswap requires an integral type");
  if constexpr (sizeof(T) == 1)
  {
    return value;
  }
  else if constexpr (sizeof(T) == 2)
  {
#if defined(__has_builtin)
#if __has_builtin(__builtin_bswap16)
    return static_cast<T>(__builtin_bswap16(static_cast<uint16_t>(value)));
#endif
#endif
    return static_cast<T>(((value & 0x00FF) << 8) | ((value & 0xFF00) >> 8));
  }
  else if constexpr (sizeof(T) == 4)
  {
#if defined(__has_builtin)
#if __has_builtin(__builtin_bswap32)
    return static_cast<T>(__builtin_bswap32(static_cast<uint32_t>(value)));
#endif
#endif
    return static_cast<T>(((value & 0x000000FFu) << 24) | ((value & 0x0000FF00u) << 8) |
                          ((value & 0x00FF0000u) >> 8) | ((value & 0xFF000000u) >> 24));
  }
  else if constexpr (sizeof(T) == 8)
  {
#if defined(__has_builtin)
#if __has_builtin(__builtin_bswap64)
    return static_cast<T>(__builtin_bswap64(static_cast<uint64_t>(value)));
#endif
#endif
    return static_cast<T>(((value & 0x00000000000000FFull) << 56) |
                          ((value & 0x000000000000FF00ull) << 40) |
                          ((value & 0x0000000000FF0000ull) << 24) |
                          ((value & 0x00000000FF000000ull) << 8) |
                          ((value & 0x000000FF00000000ull) >> 8) |
                          ((value & 0x0000FF0000000000ull) >> 24) |
                          ((value & 0x00FF000000000000ull) >> 40) |
                          ((value & 0xFF00000000000000ull) >> 56));
  }
  else
  {
    static_assert(sizeof(T) <= 8, "byteswap only supports types up to 64 bits");
    return value;
  }
}

}  // namespace detail

inline constexpr u16 swap16(u16 value)
{
  return detail::byteswap(value);
}

inline constexpr u32 swap32(u32 value)
{
  return detail::byteswap(value);
}

inline constexpr u64 swap64(u64 value)
{
  return detail::byteswap(value);
}

inline u16 swap16(const u8* ptr)
{
  u16 value;
  std::memcpy(&value, ptr, sizeof(value));
  return swap16(value);
}

inline u32 swap32(const u8* ptr)
{
  u32 value;
  std::memcpy(&value, ptr, sizeof(value));
  return swap32(value);
}

inline u64 swap64(const u8* ptr)
{
  u64 value;
  std::memcpy(&value, ptr, sizeof(value));
  return swap64(value);
}

template <typename T>
inline constexpr T to_big_endian(T value)
{
  static_assert(std::is_integral_v<T>, "to_big_endian requires an integral type");
  if constexpr (detail::kIsLittleEndian)
    return detail::byteswap(value);
  else
    return value;
}

template <typename T>
inline constexpr T from_big_endian(T value)
{
  static_assert(std::is_integral_v<T>, "from_big_endian requires an integral type");
  if constexpr (detail::kIsLittleEndian)
    return detail::byteswap(value);
  else
    return value;
}

template <typename T>
inline T read_be(const void* ptr)
{
  static_assert(std::is_integral_v<T>, "read_be requires an integral type");
  T value;
  std::memcpy(&value, ptr, sizeof(T));
  return from_big_endian(value);
}

template <typename T>
inline void write_be(T value, void* ptr)
{
  static_assert(std::is_integral_v<T>, "write_be requires an integral type");
  const T be = to_big_endian(value);
  std::memcpy(ptr, &be, sizeof(T));
}

template <typename T>
struct big_endian_value
{
  static_assert(std::is_integral_v<T>, "big_endian_value must wrap an integral type");

  big_endian_value() = default;
  explicit big_endian_value(T value) { *this = value; }

  operator T() const { return from_big_endian(raw); }

  big_endian_value& operator=(T value)
  {
    raw = to_big_endian(value);
    return *this;
  }

private:
  T raw{};
};

template <typename T, typename U>
inline constexpr T align_down(T value, U alignment)
{
  static_assert(std::is_integral_v<T>, "align_down value must be integral");
  const T align = static_cast<T>(alignment);
  if (align <= 1)
    return value;
  return static_cast<T>(value - (value % align));
}

template <typename T, typename U>
inline constexpr T align_up(T value, U alignment)
{
  static_assert(std::is_integral_v<T>, "align_up value must be integral");
  const T align = static_cast<T>(alignment);
  if (align <= 1)
    return value;
  const T adjusted = static_cast<T>(value + (align - 1));
  return align_down(adjusted, align);
}

}  // namespace dik::byte_utils

namespace fmt
{
template <typename T>
struct formatter<dik::byte_utils::big_endian_value<T>>
{
  formatter<T> inner;
  constexpr auto parse(format_parse_context& ctx) { return inner.parse(ctx); }
  template <typename FormatContext>
  auto format(const dik::byte_utils::big_endian_value<T>& value, FormatContext& ctx) const
  {
    return inner.format(static_cast<T>(value), ctx);
  }
};
}  // namespace fmt
