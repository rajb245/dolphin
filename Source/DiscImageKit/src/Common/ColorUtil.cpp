#include "Common/ColorUtil.h"

#include "Common/Swap.h"

namespace Common
{
namespace
{
constexpr int kLut5To8[] = {0x00, 0x08, 0x10, 0x18, 0x20, 0x29, 0x31, 0x39, 0x41, 0x4A, 0x52,
                            0x5A, 0x62, 0x6A, 0x73, 0x7B, 0x83, 0x8B, 0x94, 0x9C, 0xA4, 0xAC,
                            0xB4, 0xBD, 0xC5, 0xCD, 0xD5, 0xDE, 0xE6, 0xEE, 0xF6, 0xFF};

constexpr int kLut4To8[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                            0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

constexpr int kLut3To8[] = {0x00, 0x24, 0x48, 0x6D, 0x91, 0xB6, 0xDA, 0xFF};

u32 Decode5A3(u16 val)
{
  constexpr u32 kBackgroundColor = 0x00000000;

  int r;
  int g;
  int b;
  int a;

  if ((val & 0x8000) != 0)
  {
    r = kLut5To8[(val >> 10) & 0x1F];
    g = kLut5To8[(val >> 5) & 0x1F];
    b = kLut5To8[val & 0x1F];
    a = 0xFF;
  }
  else
  {
    a = kLut3To8[(val >> 12) & 0x7];
    r = (kLut4To8[(val >> 8) & 0xF] * a + (kBackgroundColor & 0xFF) * (255 - a)) / 255;
    g = (kLut4To8[(val >> 4) & 0xF] * a + ((kBackgroundColor >> 8) & 0xFF) * (255 - a)) / 255;
    b = (kLut4To8[val & 0xF] * a + ((kBackgroundColor >> 16) & 0xFF) * (255 - a)) / 255;
    a = 0xFF;
  }

  return static_cast<u32>(a << 24 | r << 16 | g << 8 | b);
}
}  // namespace

void Decode5A3Image(u32* dst, const u16* src, int width, int height)
{
  for (int y = 0; y < height; y += 4)
  {
    for (int x = 0; x < width; x += 4)
    {
      for (int iy = 0; iy < 4; ++iy, src += 4)
      {
        for (int ix = 0; ix < 4; ++ix)
        {
          const u32 rgba = Decode5A3(Common::swap16(src[ix]));
          dst[(y + iy) * width + (x + ix)] = rgba;
        }
      }
    }
  }
}

void DecodeCI8Image(u32* dst, const u8* src, const u16* pal, int width, int height)
{
  for (int y = 0; y < height; y += 4)
  {
    for (int x = 0; x < width; x += 8)
    {
      for (int iy = 0; iy < 4; ++iy, src += 8)
      {
        u32* const row = dst + (y + iy) * width + x;
        for (int ix = 0; ix < 8; ++ix)
        {
          row[ix] = Decode5A3(Common::swap16(pal[src[ix]]));
        }
      }
    }
  }
}

}  // namespace Common
