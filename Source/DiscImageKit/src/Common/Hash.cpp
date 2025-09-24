#include "Common/Hash.h"

#include <algorithm>
#include <cstddef>

#if DIK_HAVE_ZLIB
#include <zlib.h>
#else
#include <array>
#endif

namespace Common
{
#if DIK_HAVE_ZLIB
u32 HashAdler32(const u8* data, size_t len)
{
  return adler32(0u, data, static_cast<uInt>(len));
}

u32 StartCRC32()
{
  return crc32_z(0u, nullptr, 0);
}

u32 UpdateCRC32(u32 crc, const u8* data, size_t len)
{
  return crc32_z(crc, data, len);
}

u32 ComputeCRC32(const u8* data, size_t len)
{
  return crc32_z(0u, data, len);
}

u32 ComputeCRC32(std::string_view data)
{
  return crc32_z(0u, reinterpret_cast<const u8*>(data.data()), data.size());
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
    {
      c = (c & 1u) ? (0xEDB88320u ^ (c >> 1u)) : (c >> 1u);
    }
    table[i] = c;
  }
  return table;
}

constexpr std::array<u32, 256> CRC_TABLE = MakeCrcTable();

u32 Adler32Impl(const u8* data, size_t len)
{
  u32 a = 1;
  u32 b = 0;

  while (len > 0)
  {
    const size_t chunk = std::min<size_t>(len, 5550);
    len -= chunk;

    for (size_t i = 0; i < chunk; ++i)
    {
      a += data[i];
      b += a;
    }

    a %= ADLER_BASE;
    b %= ADLER_BASE;
    data += chunk;
  }

  return (b << 16) | a;
}

u32 Crc32Impl(u32 crc, const u8* data, size_t len)
{
  crc ^= 0xFFFFFFFFu;
  for (size_t i = 0; i < len; ++i)
  {
    crc = CRC_TABLE[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8u);
  }
  return crc ^ 0xFFFFFFFFu;
}
}  // namespace

u32 HashAdler32(const u8* data, size_t len)
{
  if (!data || len == 0)
    return 1;

  return Adler32Impl(data, len);
}

u32 StartCRC32()
{
  return 0u;
}

u32 UpdateCRC32(u32 crc, const u8* data, size_t len)
{
  if (!data || len == 0)
    return crc;

  return Crc32Impl(crc, data, len);
}

u32 ComputeCRC32(const u8* data, size_t len)
{
  if (!data || len == 0)
    return 0u;

  return Crc32Impl(0u, data, len);
}

u32 ComputeCRC32(std::string_view data)
{
  if (data.empty())
    return 0u;

  return Crc32Impl(0u, reinterpret_cast<const u8*>(data.data()), data.size());
}
#endif

}  // namespace Common
