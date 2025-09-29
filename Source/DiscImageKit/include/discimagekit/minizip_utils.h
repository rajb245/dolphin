#pragma once

#include <algorithm>

#include <mz.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

#include "discimagekit/scope_guard.h"
#include "discimagekit/types.h"

namespace dik::zip
{

inline bool read_file(void* zip_reader, u8* destination, u64 len)
{
  constexpr u64 MAX_BUFFER_SIZE = 65535;

  if (mz_zip_reader_entry_open(zip_reader) != MZ_OK)
    return false;

  dik::ScopeGuard guard{[&] { mz_zip_reader_entry_close(zip_reader); }};

  u64 bytes_to_go = len;
  while (bytes_to_go > 0)
  {
    const u32 read_len = static_cast<u32>(std::min(bytes_to_go, MAX_BUFFER_SIZE));
    const int rv = mz_zip_reader_entry_read(zip_reader, destination, read_len);
    if (rv < 0)
      return false;

    const u32 bytes_read = static_cast<u32>(rv);
    bytes_to_go -= bytes_read;
    destination += bytes_read;
  }

  return bytes_to_go == 0;
}

template <typename ContiguousContainer>
bool read_file(void* zip_reader, ContiguousContainer* destination)
{
  return read_file(zip_reader, reinterpret_cast<u8*>(destination->data()), destination->size());
}

}  // namespace dik::zip

