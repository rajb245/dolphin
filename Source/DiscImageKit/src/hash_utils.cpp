#include "discimagekit/hash_utils.h"

#include <algorithm>

#if DIK_HAVE_ZLIB
#include <zlib.h>
#else
#include <array>
#endif

namespace dik::hash_utils
{
#if DIK_HAVE_ZLIB
u32 adler32(const u8* data, size_t length)
{
  return ::adler32(0u, data, static_cast<uInt>(length));
}

u32 adler32(std::span<const u8> data)
{
  return adler32(data.data(), data.size());
}

u32 crc32_start()
{
  return ::crc32_z(0u, nullptr, 0u);
}

u32 crc32_update(u32 crc, const u8* data, size_t length)
{
  return ::crc32_z(crc, data, length);
}

u32 crc32_update(u32 crc, std::span<const u8> data)
{
  return crc32_update(crc, data.data(), data.size());
}

u32 crc32_compute(const u8* data, size_t length)
{
  return ::crc32_z(0u, data, length);
}

u32 crc32_compute(std::span<const u8> data)
{
  return crc32_compute(data.data(), data.size());
}

u32 crc32_compute(std::string_view data)
{
  return crc32_compute(reinterpret_cast<const u8*>(data.data()), data.size());
}

#else
namespace
{
constexpr u32 ADLER_BASE = 65521u;

constexpr std::array<u32, 256> MakeCrcTable()
{
  std::array<u32, 256> table{};
  for (size_t i = 0; i < table.size(); ++i)
  {
    u32 c = static_cast<u32>(i);
    for (int bit = 0; bit < 8; ++bit)
      c = (c & 1u) ? (0xEDB88320u ^ (c >> 1u)) : (c >> 1u);
    table[i] = c;
  }
  return table;
}

constexpr std::array<u32, 256> CRC_TABLE = MakeCrcTable();

template <typename Ptr>
u32 Adler32Impl(Ptr data, size_t length)
{
  u32 a = 1;
  u32 b = 0;

  while (length > 0)
  {
    const size_t chunk = std::min<size_t>(length, 5550);
    length -= chunk;

    for (size_t i = 0; i < chunk; ++i)
    {
      a += static_cast<u8>(data[i]);
      b += a;
    }

    a %= ADLER_BASE;
    b %= ADLER_BASE;
    data += chunk;
  }

  return (b << 16) | a;
}

u32 Crc32Impl(u32 crc, const u8* data, size_t length)
{
  crc ^= 0xFFFFFFFFu;
  for (size_t i = 0; i < length; ++i)
    crc = CRC_TABLE[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8u);
  return crc ^ 0xFFFFFFFFu;
}
}  // namespace

u32 adler32(const u8* data, size_t length)
{
  if (!data || length == 0)
    return 1;
  return Adler32Impl(data, length);
}

u32 adler32(std::span<const u8> data)
{
  return adler32(data.data(), data.size());
}

u32 crc32_start()
{
  return 0u;
}

u32 crc32_update(u32 crc, const u8* data, size_t length)
{
  if (!data || length == 0)
    return crc;
  return Crc32Impl(crc, data, length);
}

u32 crc32_update(u32 crc, std::span<const u8> data)
{
  return crc32_update(crc, data.data(), data.size());
}

u32 crc32_compute(const u8* data, size_t length)
{
  if (!data || length == 0)
    return 0u;
  return Crc32Impl(0u, data, length);
}

u32 crc32_compute(std::span<const u8> data)
{
  return crc32_compute(data.data(), data.size());
}

u32 crc32_compute(std::string_view data)
{
  return crc32_compute(reinterpret_cast<const u8*>(data.data()), data.size());
}

#endif

}  // namespace dik::hash_utils

