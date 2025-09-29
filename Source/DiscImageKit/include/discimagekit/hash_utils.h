#pragma once

#include <cstddef>
#include <span>
#include <string_view>

#include "discimagekit/types.h"

namespace dik::hash_utils
{
u32 adler32(const u8* data, size_t length);
u32 adler32(std::span<const u8> data);

u32 crc32_start();
u32 crc32_update(u32 crc, const u8* data, size_t length);
u32 crc32_update(u32 crc, std::span<const u8> data);
u32 crc32_compute(const u8* data, size_t length);
u32 crc32_compute(std::span<const u8> data);
u32 crc32_compute(std::string_view data);
}

