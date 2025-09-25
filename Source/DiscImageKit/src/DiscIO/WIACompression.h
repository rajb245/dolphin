// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "Common/CommonTypes.h"
#include "Common/Crypto/SHA1.h"
#include "DiscIO/LaggedFibonacciGenerator.h"

#ifndef DIK_HAVE_BZIP2
#define DIK_HAVE_BZIP2 1
#endif

#ifndef DIK_HAVE_LZMA
#define DIK_HAVE_LZMA 1
#endif

#ifndef DIK_HAVE_ZSTD
#define DIK_HAVE_ZSTD 1
#endif

namespace DiscIO
{
struct DecompressionBuffer
{
  std::vector<u8> data;
  size_t bytes_written = 0;
};

struct PurgeSegment
{
  u32 offset;
  u32 size;
};
static_assert(sizeof(PurgeSegment) == 0x08, "Wrong size for WIA purge segment");

class Decompressor
{
public:
  virtual ~Decompressor();

  virtual bool Decompress(const DecompressionBuffer& in, DecompressionBuffer* out,
                          size_t* in_bytes_read) = 0;
  virtual bool Done() const { return m_done; }

protected:
  bool m_done = false;
};

class NoneDecompressor final : public Decompressor
{
public:
  bool Decompress(const DecompressionBuffer& in, DecompressionBuffer* out,
                  size_t* in_bytes_read) override;
};

// This class assumes that more bytes won't be added to in once in.bytes_written == in.data.size()
// and that *in_bytes_read initially will be equal to the size of the exception lists
class PurgeDecompressor final : public Decompressor
{
public:
  PurgeDecompressor(u64 decompressed_size);
  bool Decompress(const DecompressionBuffer& in, DecompressionBuffer* out,
                  size_t* in_bytes_read) override;

private:
  const u64 m_decompressed_size;

  PurgeSegment m_segment = {};
  size_t m_bytes_read = 0;
  size_t m_segment_bytes_written = 0;
  size_t m_out_bytes_written = 0;
  bool m_started = false;

  std::unique_ptr<Common::SHA1::Context> m_sha1_context;
};

#if DIK_HAVE_BZIP2
class Bzip2Decompressor final : public Decompressor
{
public:
  Bzip2Decompressor();
  ~Bzip2Decompressor() override;

  bool Decompress(const DecompressionBuffer& in, DecompressionBuffer* out,
                  size_t* in_bytes_read) override;

private:
  bool m_started = false;
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};
#endif

#if DIK_HAVE_LZMA
class LZMADecompressor final : public Decompressor
{
public:
  LZMADecompressor(bool lzma2, const u8* filter_options, size_t filter_options_size);
  ~LZMADecompressor() override;

  bool Decompress(const DecompressionBuffer& in, DecompressionBuffer* out,
                  size_t* in_bytes_read) override;

private:
  bool m_started = false;
  bool m_error_occurred = false;
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};
#endif

#if DIK_HAVE_ZSTD
class ZstdDecompressor final : public Decompressor
{
public:
  ZstdDecompressor();
  ~ZstdDecompressor() override;

  bool Decompress(const DecompressionBuffer& in, DecompressionBuffer* out,
                  size_t* in_bytes_read) override;

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};
#endif

class RVZPackDecompressor final : public Decompressor
{
public:
  RVZPackDecompressor(std::unique_ptr<Decompressor> decompressor, DecompressionBuffer decompressed,
                      u64 data_offset, u32 rvz_packed_size);

  bool Decompress(const DecompressionBuffer& in, DecompressionBuffer* out,
                  size_t* in_bytes_read) override;

  bool Done() const override;

private:
  bool IncrementBytesRead(size_t x);
  std::optional<bool> ReadToDecompressed(const DecompressionBuffer& in, size_t* in_bytes_read,
                                         size_t decompressed_bytes_read, size_t bytes_to_read);

  std::unique_ptr<Decompressor> m_decompressor;
  DecompressionBuffer m_decompressed;
  size_t m_decompressed_bytes_read = 0;
  size_t m_bytes_read;
  u64 m_data_offset;
  u32 m_rvz_packed_size;

  u32 m_size = 0;
  bool m_junk = false;
  LaggedFibonacciGenerator m_lfg;
};

class Compressor
{
public:
  virtual ~Compressor();

  // First call Start, then AddDataOnlyForPurgeHashing/Compress any number of times,
  // then End, then GetData/GetSize any number of times.

  virtual bool Start(std::optional<u64> size) = 0;
  virtual bool AddPrecedingDataOnlyForPurgeHashing(const u8* data, size_t size) { return true; }
  virtual bool Compress(const u8* data, size_t size) = 0;
  virtual bool End() = 0;

  virtual const u8* GetData() const = 0;
  virtual size_t GetSize() const = 0;
};

class PurgeCompressor final : public Compressor
{
public:
  PurgeCompressor();
  ~PurgeCompressor() override;

  bool Start(std::optional<u64> size) override;
  bool AddPrecedingDataOnlyForPurgeHashing(const u8* data, size_t size) override;
  bool Compress(const u8* data, size_t size) override;
  bool End() override;

  const u8* GetData() const override;
  size_t GetSize() const override;

private:
  std::vector<u8> m_buffer;
  size_t m_bytes_written = 0;
  std::unique_ptr<Common::SHA1::Context> m_sha1_context;
};

#if DIK_HAVE_BZIP2
class Bzip2Compressor final : public Compressor
{
public:
  Bzip2Compressor(int compression_level);
  ~Bzip2Compressor() override;

  bool Start(std::optional<u64> size) override;
  bool Compress(const u8* data, size_t size) override;
  bool End() override;

  const u8* GetData() const override;
  size_t GetSize() const override;

private:
  void ExpandBuffer(size_t bytes_to_add);

  std::vector<u8> m_buffer;
  int m_compression_level;
  size_t m_bytes_written = 0;
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};
#endif

#if DIK_HAVE_LZMA
class LZMACompressor final : public Compressor
{
public:
  LZMACompressor(bool lzma2, int compression_level, u8 compressor_data_out[7],
                 u8* compressor_data_size_out);
  ~LZMACompressor() override;

  bool Start(std::optional<u64> size) override;
  bool Compress(const u8* data, size_t size) override;
  bool End() override;

  const u8* GetData() const override;
  size_t GetSize() const override;

private:
  void ExpandBuffer(size_t bytes_to_add);

  std::vector<u8> m_buffer;
  bool m_initialization_failed = false;
  size_t m_bytes_written = 0;
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};
#endif

#if DIK_HAVE_ZSTD
class ZstdCompressor final : public Compressor
{
public:
  ZstdCompressor(int compression_level);
  ~ZstdCompressor() override;

  bool Start(std::optional<u64> size) override;
  bool Compress(const u8* data, size_t size) override;
  bool End() override;

  const u8* GetData() const override;
  size_t GetSize() const override;

private:
  void ExpandBuffer(size_t bytes_to_add);

  std::vector<u8> m_buffer;
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};
#endif

}  // namespace DiscIO
