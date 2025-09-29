#pragma once

#include "discimagekit/types.h"

namespace dik::color
{
void decode_5a3(u32* dst, const u16* src, int width, int height);
void decode_ci8(u32* dst, const u8* src, const u16* palette, int width, int height);
}

