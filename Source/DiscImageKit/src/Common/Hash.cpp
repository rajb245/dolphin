#include "Common/Hash.h"

#include <zlib.h>

namespace Common
{
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

}  // namespace Common
